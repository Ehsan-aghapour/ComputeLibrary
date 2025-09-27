
## ./rr.sh --order=BBBBGGGG --data=/data/data/com.termux/files/home/ARMCL-RockPi/assets/alexnet --image=/data/data/com.termux/files/home/ARMCL-RockPi/assets/ppm_images_227  --labels=/data/data/com.termux/files/home/ARMCL-RockPi/assets/labels.txt
#./b64.sh 23
##adb push build/examples/Pipeline/graph_yolov3_n_pipe_npu /data/data/com.termux/files/home/ARMCL-RockPi/test_graph/
##adb shell /data/data/com.termux/files/home/ARMCL-RockPi/test_graph/graph_yolov3_n_pipe_npu $1 $2 $3 $4 $5
#adb push build/examples/Pipeline/graph_alexnet_pipeline /data/data/com.termux/files/home/ARMCL-RockPi/test_graph/
#adb shell /data/data/com.termux/files/home/ARMCL-RockPi/test_graph/graph_alexnet_pipeline $1 $2 $3 $4 $5 $6 $7
adb shell /data/data/com.termux/files/home/ARMCL-RockPi/test_graph/graph_alexnet_pipeline --order=BBBBBLLBBBBBGGGG --data=/data/data/com.termux/files/home/ARMCL-RockPi/assets/alexnet --image=/data/data/com.termux/files/home/ARMCL-RockPi/assets/ppm_images_227  --labels=/data/data/com.termux/files/home/ARMCL-RockPi/assets/labels.txt
