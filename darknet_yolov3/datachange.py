import os.path as osp
import os
import json as js
import numpy as np
from shutil import copyfile
from PIL import Image

import cv2
import shutil
import xml.etree.ElementTree as ET

import pickle
from os import listdir, getcwd
from os.path import join

# my_labels = ['person', 'rock', 'broom']
my_labels = ["Sprite", "Coca-Cola", "Fanta_Orange", "Fanta_Apple", "Gatorade_Orange", "Gatorade_Huang", "Gatorade_Blue",
             "Qin lemon water", "Beauty Juice Orange", "Honey Citron Tea", "Sour plum soup", "Unified Green Tea",
             "Unified Ice Black Tea", "Oolong Tea", "Lemon U grid", "Jasmine Honey Tea", "Jasmine Tea",
             "Master Green Tea", "Master Kong Ice Black Tea", "Ome Green Tea", "Osmanthus Sour Plum Soup Drink",
             "Unification of fresh oranges"]

sets = [('2012', 'train')]
# classes = ['person', 'rock', 'broom']
classes = my_labels.copy()

dataset = {}


def convert(size, box):
    dw = 1. / (size[0])
    dh = 1. / (size[1])
    x = (box[0] + box[1]) / 2.0 - 1
    y = (box[2] + box[3]) / 2.0 - 1
    w = box[1] - box[0]
    h = box[3] - box[2]
    x = x * dw
    w = w * dw
    y = y * dh
    h = h * dh
    return (x, y, w, h)


def convert_annotation(year, image_id):
    in_file = open(os.getcwd() + '/Annotations_/%s.xml' % (image_id))
    out_file = open(os.getcwd() + '/labels/val2014/%s.txt' % (image_id), 'w')
    tree = ET.parse(in_file)
    root = tree.getroot()
    size = root.find('size')
    w = int(size.find('width').text)
    h = int(size.find('height').text)

    for obj in root.iter('object'):
        difficult = obj.find('difficult').text
        cls = obj.find('name').text
        if cls not in classes or int(difficult) == 1:
            continue
        cls_id = classes.index(cls)
        xmlbox = obj.find('bndbox')
        b = (float(xmlbox.find('xmin').text), float(xmlbox.find('xmax').text), float(xmlbox.find('ymin').text),
             float(xmlbox.find('ymax').text))
        bb = convert((w, h), b)
        out_file.write(str(cls_id) + " " + " ".join([str(a) for a in bb]) + '\n')


def readxml(dataset, xml, count, file_name):
    tree = ET.parse(xml)
    root = tree.getroot()
    for child in root:
        if child.tag == "size":
            for s_ch in child:
                if s_ch.tag == "width":
                    w = s_ch.text
                else:
                    h = s_ch.text
        elif child.tag == "object":
            for s_ch in child:
                if s_ch.tag == "bndbox":
                    for ss_ch in s_ch:
                        if ss_ch.tag == "xmin":
                            xmin = ss_ch.text
                        elif ss_ch.tag == "ymin":
                            ymin = ss_ch.text
                        elif ss_ch.tag == "xmax":
                            xmax = ss_ch.text
                        elif ss_ch.tag == "ymax":
                            ymax = ss_ch.text
                else:
                    ca_name = s_ch.text

    dataset.setdefault("images", []).append({
        'file_name': file_name,
        'id': int(count),
        'width': int(w),
        'height': int(h)
    })
    dataset.setdefault("annotations", []).append({
        'image_id': int(count),
        'bbox': [int(xmin), int(ymin), int(xmax) - int(xmin), int(ymax) - int(ymin)],
        'category_id': 6,
        'area': int(w) * int(h),
        'iscrowd': 0,
        'id': int(count),
        'segmentation': []
    })


class DataChange:

    def __init__(self, rootpath, source_json_path, source_img_path, use_my_labels=False,
                 need_to_change_image_size=False, image_width=300, image_height=300):
        self.rootpath = rootpath
        self.source_json_path = source_json_path
        self.source_img_path = source_img_path
        self.use_my_labels = use_my_labels
        self.need_to_change_image_size = need_to_change_image_size
        self.image_width = image_width
        self.image_height = image_height

    def produceImage(self, file_in, file_out):
        image = Image.open(file_in)
        resized_image = image.resize((self.image_width, self.image_height), Image.ANTIALIAS)
        resized_image.save(file_out)

    def data_change(self):
        xmlpath = self.rootpath + '/Annotations_'
        imgpath = self.rootpath + '/JPEGImages'
        imgsetpath = self.rootpath + '/ImageSets'
        txtpath = imgsetpath + '/Main'

        if not osp.exists(xmlpath):
            os.makedirs(xmlpath)
        if not osp.exists(imgpath):
            os.makedirs(imgpath)
        if not osp.exists(imgsetpath):
            os.makedirs(imgsetpath)
        if not osp.exists(txtpath):
            os.makedirs(txtpath)

        json_file_arr = os.listdir(self.source_json_path)
        img_file_arr = os.listdir(self.source_img_path)

        json_arr = []
        img_arr = []

        for name in json_file_arr:
            if '.json' in name:
                json_arr.append(name)
        for name in img_file_arr:
            if '.jpg' in name or '.png' in name:
                img_arr.append(name)

        fixed_file_arr = []
        fixed_file_type = []

        for json in json_arr:
            for img in img_arr:
                if json.split('.')[0] == img.split('.')[0]:
                    fixed_file_arr.append(json.split('.')[0])
                    fixed_file_type.append('.' + img.split('.')[1])

        annotation_arr = np.array([])

        for i in range(len(fixed_file_arr)):

            if self.need_to_change_image_size:
                # self.produceImage(self.source_img_path + '/' + fixed_file_arr[i] + fixed_file_type[i],
                #                   imgpath + '/' + fixed_file_arr[i] + fixed_file_type[i])
                self.produceImage(self.source_img_path + '/' + fixed_file_arr[i] + fixed_file_type[i],
                                  imgpath + '/' + str(i) + fixed_file_type[i])
            else:
                # copyfile(self.source_img_path + '/' + fixed_file_arr[i] + fixed_file_type[i],
                #          imgpath + '/' + fixed_file_arr[i] + fixed_file_type[i])
                copyfile(self.source_img_path + '/' + fixed_file_arr[i] + fixed_file_type[i],
                         imgpath + '/' + str(i) + fixed_file_type[i])

            f = open(self.source_json_path + '/' + fixed_file_arr[i] + '.json', 'r', encoding='utf-8')
            my_dic = js.load(f)
            annotation_arr = np.append(annotation_arr, (fixed_file_arr[i], my_dic))
            f.close()

        annotation_arr = annotation_arr.reshape(-1, 2)

        f1 = open(txtpath + '/test.txt', 'w')
        f2 = open(txtpath + '/trainval.txt', 'w')
        f3 = open(txtpath + '/person_trainval.txt', 'w')
        f4 = open(txtpath + '/train.txt', 'w')
        f5 = open(txtpath + '/val.txt', 'w')
        f6 = open(self.rootpath + '/ground_truth.txt', 'w')

        if not os.path.exists(os.getcwd() + '/Main/'):
            os.makedirs(os.getcwd() + '/Main/')
        f7 = open(os.getcwd() + '/Main/' + 'train.txt', 'w')

        for i in range(annotation_arr.shape[0]):
            f1.write(annotation_arr[i][0] + '\n')
            f2.write(annotation_arr[i][0] + '\n')
            f3.write(annotation_arr[i][0] + ' 1\n')
            f4.write(str(i) + '\n')
            f5.write(annotation_arr[i][0] + '\n')
            f6.write('\nGROUND TRUTH FOR: ' + annotation_arr[i][0] + '\n')
            f7.write(str(i) + '.jpg\n')
            # f = open(xmlpath + '/' + annotation_arr[i][0] + '.xml', 'w')
            f = open(xmlpath + '/' + str(i) + '.xml', 'w')
            f.write('<annotation>\n')
            f.write('\t<folder>VOC2007</folder>\n')
            f.write('\t<filename>' + annotation_arr[i][0] + '</filename>\n')
            f.write('\t<size>\n')
            if self.need_to_change_image_size:
                f.write('\t\t<width>%s</width>\n' % self.image_width)
                f.write('\t\t<height>%s</height>\n' % self.image_height)
            else:
                f.write('\t\t<width>%s</width>\n' % annotation_arr[i][1]['Area']['shape'][0])
                f.write('\t\t<height>%s</height>\n' % annotation_arr[i][1]['Area']['shape'][1])
            f.write('\t\t<depth>3</depth>\n')
            f.write('\t</size>\n')
            f.write('\t<segmented>0</segmented>\n')
            if len(annotation_arr[i][1]['Area']['labels']) > 0:
                for j in range(len(annotation_arr[i][1]['Area']['labels'])):
                    f6.write('label: ')
                    f.write('\t<object>\n')
                    if self.use_my_labels:
                        f.write('\t\t<name>%s</name>\n' % my_labels[int(annotation_arr[i][1]['Area']['labels'][j][0])])
                    else:
                        f.write('\t\t<name>person</name>\n')
                    f.write('\t\t<pose>Unspecified</pose>\n')
                    f.write('\t\t<truncated>0</truncated>\n')
                    f.write('\t\t<difficult>0</difficult>\n')
                    f.write('\t\t<bndbox>\n')
                    if self.need_to_change_image_size:
                        f6.write('%d' % int(annotation_arr[i][1]['Area']['polygons'][j][0][0] * self.image_width /
                                            annotation_arr[i][1]['Area']['shape'][0]))
                        f6.write(' || ')
                        f6.write('%d' % int(annotation_arr[i][1]['Area']['polygons'][j][0][1] * self.image_width /
                                            annotation_arr[i][1]['Area']['shape'][1]))
                        f6.write(' || ')
                        f6.write('%d' % int(annotation_arr[i][1]['Area']['polygons'][j][2][0] * self.image_width /
                                            annotation_arr[i][1]['Area']['shape'][0]))
                        f6.write(' || ')
                        f6.write('%d' % int(annotation_arr[i][1]['Area']['polygons'][j][2][1] * self.image_width /
                                            annotation_arr[i][1]['Area']['shape'][1]))
                        f6.write(' || ')
                        f6.write(my_labels[int(annotation_arr[i][1]['Area']['labels'][j][0])])
                        f6.write('\n')
                        f.write('\t\t\t<xmin>%s</xmin>\n' % int(
                            annotation_arr[i][1]['Area']['polygons'][j][0][0] * self.image_width /
                            annotation_arr[i][1]['Area']['shape'][0]))
                        f.write('\t\t\t<ymin>%s</ymin>\n' % int(
                            annotation_arr[i][1]['Area']['polygons'][j][0][1] * self.image_height /
                            annotation_arr[i][1]['Area']['shape'][1]))
                        f.write('\t\t\t<xmax>%s</xmax>\n' % int(
                            annotation_arr[i][1]['Area']['polygons'][j][2][0] * self.image_width /
                            annotation_arr[i][1]['Area']['shape'][0]))
                        f.write('\t\t\t<ymax>%s</ymax>\n' % int(
                            annotation_arr[i][1]['Area']['polygons'][j][2][1] * self.image_height /
                            annotation_arr[i][1]['Area']['shape'][1]))
                    else:
                        f6.write('%d' % annotation_arr[i][1]['Area']['polygons'][j][0][0])
                        f6.write(' || ')
                        f6.write('%d' % annotation_arr[i][1]['Area']['polygons'][j][0][1])
                        f6.write(' || ')
                        f6.write('%d' % annotation_arr[i][1]['Area']['polygons'][j][2][0])
                        f6.write(' || ')
                        f6.write('%d' % annotation_arr[i][1]['Area']['polygons'][j][2][1])
                        f6.write(' || ')
                        f6.write('person\n')
                        f.write('\t\t\t<xmin>%s</xmin>\n' % annotation_arr[i][1]['Area']['polygons'][j][0][0])
                        f.write('\t\t\t<ymin>%s</ymin>\n' % annotation_arr[i][1]['Area']['polygons'][j][0][1])
                        f.write('\t\t\t<xmax>%s</xmax>\n' % annotation_arr[i][1]['Area']['polygons'][j][2][0])
                        f.write('\t\t\t<ymax>%s</ymax>\n' % annotation_arr[i][1]['Area']['polygons'][j][2][1])
                    f.write('\t\t</bndbox>\n')
                    f.write('\t</object>\n')
            f.write('</annotation>')
            f.close()
        f1.close()
        f2.close()
        f3.close()
        f4.close()
        f5.close()
        f6.close()
        f7.close()


try:
    shutil.rmtree('annotations')
except:
    pass
try:
    shutil.rmtree('images')
except:
    pass
try:
    shutil.rmtree('labels')
except:
    pass
try:
    shutil.rmtree('Main')
except:
    pass

datachange = DataChange(os.getcwd(), os.getcwd() + '/sources/', os.getcwd() + '/sources/', True)
datachange.data_change()

wd = getcwd()

for year, image_set in sets:
    if not os.path.exists(os.getcwd() + '/labels/val2014/'):
        os.makedirs(os.getcwd() + '/labels/val2014/')
    image_ids = open(os.getcwd() + '/ImageSets/Main/%s.txt' % (image_set)).read().strip().split()
    list_file = open(os.getcwd() + '/%s.txt' % (image_set), 'w')
    for image_id in image_ids:
        list_file.write(os.getcwd() + '/JPEGImages/%s.jpg\n' % (image_id))
        convert_annotation(year, image_id)
    list_file.close()

# os.system("cat 2007_train.txt 2007_val.txt 2012_train.txt 2012_val.txt > train.txt")
# os.system("cat 2007_train.txt 2007_val.txt 2007_test.txt 2012_train.txt 2012_val.txt > train.all.txt")

im_path = os.getcwd() + "/imgs/"
trainimg = os.getcwd() + "/images/val2014/"
if not os.path.exists(im_path):
    os.makedirs(im_path)
if not os.path.exists(trainimg):
    os.makedirs(trainimg)
if not os.path.exists(im_path + 'img0/'):
    os.makedirs(im_path + 'img0/')

imgs_list = os.listdir(os.getcwd() + '/JPEGImages/')
for file in imgs_list:
    copyfile(os.getcwd() + '/JPEGImages/' + file, os.getcwd() + '/imgs/img0/' + file)

xmls_list = os.listdir(os.getcwd() + '/Annotations_/')
for file in xmls_list:
    copyfile(os.getcwd() + '/Annotations_/' + file, os.getcwd() + '/imgs/img0/' + file)

cmax = 0
dirpath = os.listdir(im_path)

for imgdir in dirpath:

    f1 = os.listdir(trainimg)
    for file in f1:
        cmax = max(cmax, int(file.split(".")[0]))
    count = 1

    for file in os.listdir(im_path + imgdir):

        if file.split(".")[1] == "jpg":
            oldname = os.path.join(im_path + imgdir, file)
            jpgname = os.path.join(trainimg, file)
            shutil.copyfile(oldname, jpgname)
            readxml(dataset, os.path.join(im_path + imgdir, file.split(".")[0] + ".xml"), count + cmax, file)
            count += 1

for i in range(1, 81):
    dataset.setdefault("categories", []).append({
        'id': i,
        'name': 1,
        'supercategory': 'No'
    })

folder = os.path.join(os.getcwd() + '/annotations/')
if not os.path.exists(folder):
    os.makedirs(folder)
json_name = os.path.join(folder + 'instances_minival2014.json')
with open(json_name, 'w') as f:
    js.dump(dataset, f)

shutil.rmtree(os.getcwd() + '/Annotations_')
shutil.rmtree(os.getcwd() + '/ImageSets')
shutil.rmtree(os.getcwd() + '/JPEGImages')
shutil.rmtree(os.getcwd() + '/imgs')
os.remove(os.getcwd() + '/ground_truth.txt')
