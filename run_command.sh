#!/bin/bash
adb shell "cd /data/local/ARM-CO-UP/test_graph/; ./graph_googlenet_pipeline --data=/data/local/ARM-CO-UP/assets --image=/data/local/ARM-CO-UP/assets/images/jpg_images_224/ --labels=/data/local/ARM-CO-UP/assets/labels/labels.txt --little_cores=4 --threads2=4 --big_cores=2 --threads=2 --order=G"
