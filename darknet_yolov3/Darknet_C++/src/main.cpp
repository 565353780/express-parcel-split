#include <iostream>
#include <vector>
#include <ctime>

#ifdef Linux
extern "C"
{
#include "../include/darknet.h"
}
#endif

#ifdef Linux
    class ObjectDetector
    {
    private:
        std::vector<std::pair<char *, std::vector<float>>> detect(image im, float thresh=0.5, float hier_thresh=0.5, float nms=0.45)
        {
            int num = 0;

            int *pnum = &num;

            network_predict_image(net, im);

            detection *dets = get_network_boxes(net, im.w, im.h, thresh, hier_thresh, nullptr, 0, pnum);

            num = pnum[0];

            if (nms)
            {
                do_nms_obj(dets, num, meta.classes, nms);
            }

            std::vector<std::pair<char *, std::vector<float>>> res;

            for(int j = 0; j < num; ++j)
            {
                for(int i = 0; i < meta.classes; ++i)
                {
                    if(dets[j].prob[i] > 0)
                    {
                        box b = dets[j].bbox;

                        std::pair<char *, std::vector<float>> temp_data;

                        temp_data.first = meta.names[i];

                        temp_data.second.emplace_back(dets[j].prob[i]);
                        temp_data.second.emplace_back(b.x - b.w / 2.0);
                        temp_data.second.emplace_back(b.y - b.h / 2.0);
                        temp_data.second.emplace_back(b.w);
                        temp_data.second.emplace_back(b.h);
                        temp_data.second.emplace_back(i);

                        res.emplace_back(temp_data);
                    }
                }
            }

            for(int i = 0; i < res.size() - 1; ++i)
            {
                //std::cout << i << std::endl;
                for(int j = i + 1; j < res.size(); ++j)
                {
                    if(res[i].second[0] < res[j].second[0])
                    {
                        std::pair<char *, std::vector<float>> exchange_data = res[i];

                        res[i] = res[j];
                        res[j] = exchange_data;
                    }
                }
            }

            free_image(im);

            free_detections(dets, num);

            return res;
        }

        std::vector<std::pair<char *, std::vector<float>>> detect(char *img, float thresh=0.5, float hier_thresh=0.5, float nms=0.45)
        {
            image im = load_image_color(img, 0, 0);

            return detect(im, thresh, hier_thresh, nms);
        }

        std::vector<std::pair<char *, std::vector<float>>> detect(float *img, int w, int h, int c, float thresh=0.5, float hier_thresh=0.5, float nms=0.45)
        {
            image im;
            im.w = w;
            im.h = h;
            im.c = c;

            int im_size = w * h * c;

            im.data = new float[im_size];

            memcpy(im.data, img, im_size * sizeof(float));

            return detect(im, thresh, hier_thresh, nms);
        }

        network *net;
        metadata meta;

    public:
        ObjectDetector(const std::string &yolov3_cfg,
                       const std::string &yolov3_weights,
                       const std::string &coco_cfg)
        {
            const char *c_str_yolov3_cfg = yolov3_cfg.c_str();
            const char *c_str_yolov3_weights = yolov3_weights.c_str();
            const char *c_str_coco_cfg = coco_cfg.c_str();

            char *str_yolov3_cfg = new char[strlen(c_str_yolov3_cfg) + 1];
            char *str_yolov3_weights = new char[strlen(c_str_yolov3_weights) + 1];
            char *str_coco_cfg = new char[strlen(c_str_coco_cfg) + 1];

            strcpy(str_yolov3_cfg, c_str_yolov3_cfg);
            strcpy(str_yolov3_weights, c_str_yolov3_weights);
            strcpy(str_coco_cfg, c_str_coco_cfg);

            cuda_set_device(gpu_index);

            net = load_network(str_yolov3_cfg, str_yolov3_weights, 0);

            meta = get_metadata(str_coco_cfg);
        }

        std::vector<std::pair<char *, std::vector<float>>> getDarknetResult(image img, float thresh=0.5, float hier_thresh=0.5, float nms=0.45)
        {
            return detect(img, thresh, hier_thresh, nms);
        }

        std::vector<std::pair<char *, std::vector<float>>> getDarknetResult(char *img, float thresh=0.5, float hier_thresh=0.5, float nms=0.45)
        {
            return detect(img, thresh, hier_thresh, nms);
        }

        std::vector<std::pair<char *, std::vector<float>>> getDarknetResult(float *img, int w, int h, int c, float thresh=0.5, float hier_thresh=0.5, float nms=0.45)
        {
            return detect(img, w, h, c, thresh, hier_thresh, nms);
        }
    };
#endif

int main()
{
#ifdef WIN32
    std::cout << "Please run this code on Linux OS." << std::endl;
#endif

#ifdef Linux
    auto yolov3_cfg = "../models/yolov3.cfg";

    auto yolov3_weights = "../models/yolov3_17100.weights";

    auto coco_cfg = "../models/coco.data";

    ObjectDetector* darknet_detector = new ObjectDetector(yolov3_cfg, yolov3_weights, coco_cfg);

    image img = load_image_color("../imgs/01.jpg", 0, 0);

    // float *image = new float[data.img_height_ * data.img_width_ * data.img_format_];

    // for(int j=0; j < data.img_height_; j++)
    // {
    //     for(int i=0; i < data.img_width_; i++)
    //     {
    //         // pixel(i,j)
    //         int color = data.getRed(i, j);
    //         if(color < 0)
    //         {
    //             color += 255;
    //         }
    //         image[j * data.img_width_ + i] = float(color) / 255.0;
    //     }
    // }
    // for(int j=0; j < data.img_height_; j++)
    // {
    //     for(int i=0; i < data.img_width_; i++)
    //     {
    //         // pixel(i,j)
    //         int color = data.getGreen(i, j);
    //         if(color < 0)
    //         {
    //             color += 255;
    //         }
    //         image[data.img_height_ * data.img_width_ + j * data.img_width_ + i] = float(color) / 255.0;
    //     }
    // }
    // for(int j=0; j < data.img_height_; j++)
    // {
    //     for(int i=0; i < data.img_width_; i++)
    //     {
    //         // pixel(i,j)
    //         int color = data.getBlue(i, j);
    //         if(color < 0)
    //         {
    //             color += 255;
    //         }
    //         image[2 * data.img_height_ * data.img_width_ + j * data.img_width_ + i] = float(color) / 255.0;
    //     }
    // }

    clock_t start = clock();
    std::vector<std::pair<char *, std::vector<float>>> result = darknet_detector->getDarknetResult(img);
    std::cout << int((clock() - start)/1000) << " ms" << std::endl;

    for(int i=0; i < result.size(); ++i)
    {
        std::cout << "label : " << result[i].first << "(" << result[i].second[5] << ")" << " ; score : " << result[i].second[0] << " ; box[tlwh] : (";
        std::cout << result[i].second[1] << ",";
        std::cout << result[i].second[2] << ",";
        std::cout << result[i].second[3] << ",";
        std::cout << result[i].second[4] << ")";

        std::cout << std::endl;
    }
#endif

    return 1;
}
