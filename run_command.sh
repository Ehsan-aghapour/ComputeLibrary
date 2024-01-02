#!/bin/bash
adb shell /data/local/ARM-CO-UP/test_graph/graph_mobilenet_pipeline --data=/data/local/ARM-CO-UP/assets//mobilenet/ --image=/data/local/ARM-CO-UP/assets//ppm_images_224/ --labels=/data/local/ARM-CO-UP/assets//labels.txt --order=GGGGGGGGGGGGGG --kernel_c=96 --power_profile_mode=layers --n=10
