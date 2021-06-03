# Yolov3 函数主体部分说明

# 1. darknet.c

## 1.1 主函数

```c++
int main(int argc, char **argv)
{
    //test_resize("data/bad.jpg");
    //test_box();
    //test_convolutional_layer();
    if(argc < 2){
        fprintf(stderr, "usage: %s <function>\n", argv[0]);
        return 0;
    }
    gpu_index = find_int_arg(argc, argv, "-i", 0);
    if(find_arg(argc, argv, "-nogpu")) {
        gpu_index = -1;
    }

#ifndef GPU
    gpu_index = -1;
#else
    if(gpu_index >= 0){
        cuda_set_device(gpu_index);
    }
#endif

    ...

    else if (0 == strcmp(argv[1], "detector")){
        run_detector(argc, argv);
    }

    ...
}
```

在使用 Yolov3 的时候，主要用到的推理函数为`run_detector(argc, argv)`

1.如果程序运行时发送给main函数的命令行参数小于2，显示标准错误

2.可以调用cudaSetDevice函数设置当前活跃的GPU卡号

3.若argv[1] = "detector"，则转向run_detector( )，并将输入参数传递给run_detector( )，run_detector( )根据argv[2]的值，转向 test 或 train 等 detector 函数

## 1.2 输入

`argv[1]` : 程序功能选择，使用Yolov3进行推理则需设置为`detector`

`argv[2]` : 程序运行模式选择，`train`为训练模式，`test`和`demo`为测试模式

`argv[3]` : 训练数据描述信息文件的名称和路径

`argv[4]` : 神经网络结构配置文件的名称和路径

`argv[5]` : 预训练参数文件的名称和路径

`argv[6]` : 待测试的图片或视频的名称和路径

# 2. detector.c

## 2.1 图像检测网络训练函数

`datacfg` : 训练数据描述信息文件的名称和路径

`cfgfile` : 神经网络结构配置文件的名称和路径

`weights` : 预训练参数文件的名称和路径

`ngpus` : 使用GPUS块数，使用一块或者不使用时，ngpus都等于1

`gpus` : GPU卡号集合（比如使用1块gpu，那么里面只含0元素，默认使用0卡号gpu；如果使用4块gpu，那么含有0,1,2,3四个元素．若不使用gpu，那么为空)

## 2.2 train_detector( )

若`argv[2] = train`，则进人此函数

```c++
    char *base = basecfg(cfgfile);
    printf("%s\n", base);
    float avg_loss = -1;
    network **nets = calloc(ngpus, sizeof(network));
```

提取配置文件名称中的主要信息用于输出打印，如：cfg/yolo.cfg中的yolo

构建网络，之前声明了用多少块GPU，就会构建多少个相同的网络

```c++
#ifdef GPU
        cuda_set_device(gpus[i]);
#endif
        nets[i] = load_network(cfgfile, weightfile, clear);
        nets[i]->learning_rate *= ngpus;
```

设置当前活跃GPU卡号（即设置gpu_index=n，同时调用cudaSetDevice函数设置当前活跃的GPU卡号）

```c++
    network *net = nets[0];

    int imgs = net->batch * net->subdivisions * ngpus;
```

imgs是一次加载到内存的图像数量

如果占内存太多或者太少的话，可以相应地把subdivision（位于yolov3.cfg）调大或者调小一点

这样一来，一次加载imgs张图片到内存，while循环每次count，就是处理完这些图片，完成一次迭代

```c++
        if(i%100==0){
#ifdef GPU
            if(ngpus != 1) sync_nets(nets, ngpus, 0);
#endif
            char buff[256];
            sprintf(buff, "%s/%s.backup", backup_directory, base);
            save_weights(net, buff);
        }
        if(i%1000==0 || (i < 1000 && i%100 == 0)){
#ifdef GPU
            if(ngpus != 1) sync_nets(nets, ngpus, 0);
#endif
            char buff[256];
            sprintf(buff, "%s/%s_%d.weights", backup_directory, base, i);
            save_weights(net, buff);
        }
```

这里的`100`和`1000`为默认的两个保存周期，第一个对应文件`yolov3.backup`，会不断将这个文件进行覆盖；第二个对应文件`yolov3_<episode>.weights`，文件依训练周期不同而有所区分，该值因电脑性能而定，过小则会迅速占用大量内存空间，过大则会导致模型保存过少，无法进行有效分析和比较

## 2.3 test_detector( )

```c++
        float *X = sized.data;
        time=what_time_is_it_now();
        network_predict(net, X);
        printf("%s: Predicted in %f seconds.\n", input, what_time_is_it_now()-time);
        int nboxes = 0;
        detection *dets = get_network_boxes(net, im.w, im.h, thresh, hier_thresh, 0, 1, &nboxes);
        //printf("%d\n", nboxes);
        //if (nms) do_nms_obj(boxes, probs, l.w*l.h*l.n, l.classes, nms);
        if (nms) do_nms_sort(dets, nboxes, l.classes, nms);
```

该部分为函数的主要功能实现部分，bbox与score等信息被存储在`dets`类中，运行完`do_nms_sort()`函数之后，bbox按score从大到小顺序排列

## 2.4 获取预测得到的bbox

修改`2.3`中代码部分所在函数的返回值，将`dets`返回，并按如下方式自行择需读取：

`dets[i].prob[j]` : 第i个位置预测得到的第j个label的score（每个位置的所有label的score至多只有一个大于零）

`dets[i].bbox` : 第i个位置预测得到的bbox

`dets[i].bbox.x`，`dets[i].bbox.y`，`dets[i].bbox.w`，`dets[i].bbox.h`分别为预测所得的坐标及形状信息