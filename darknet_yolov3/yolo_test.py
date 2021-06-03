import os
import cv2

label_list = os.listdir(os.getcwd() + '/labels/val2014/')

num = 0
total_num = 0

for label in label_list:
    with open(os.getcwd() + '/labels/val2014/' + label, 'r') as f:
        if len(f.readlines()) == 0:
            num += 1

            img = cv2.imread(os.getcwd() + '/images/val2014/' + label[:-3] + 'jpg')
            img = cv2.resize(img, (0, 0), fx=0.5, fy=0.5)

            cv2.imshow('test', img)

            cv2.waitKey(3000)

        total_num += 1

        print('\r' + str(num) + ' / ' + str(total_num), end='')

