#!/bin/bash
adb shell /data/local/ARM-CO-UP/test_graph/graph_yolov3_pipeline --data=/data/local/ARM-CO-UP/assets//yolov3/ --image=/data/local/ARM-CO-UP/assets//ppm_images_608/ --labels=/data/local/ARM-CO-UP/assets//coco.names --order=BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB --kernel_c=96 --power_profile_mode=layers --n=10
