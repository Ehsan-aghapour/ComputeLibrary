#!/bin/bash
adb shell "cd /data/local/ARM-CO-UP/test_graph/; ./graph_vgg16_earlyexit --little_cores=4 --threads2=4 --big_cores=2 --threads=2 --order=BBBBB"
