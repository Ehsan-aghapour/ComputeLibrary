## ./go.sh --order=BBBBGGGG
./b64.sh 23
if [ $? -eq 0 ]; then
    echo "Build succeeded"
    #read -p "Continue? " name

    #adb push build/examples/graph_yolov3 /data/data/com.termux/files/home/ARMCL-RockPi/test_graph/
    #adb shell /data/data/com.termux/files/home/ARMCL-RockPi/test_graph/graph_yolov3 --data=/data/data/com.termux/files/home/ARMCL-RockPi/assets/yolov3 --image=/data/data/com.termux/files/home/ARMCL-RockPi/assets/ppm_images_608/  --labels=/data/data/com.termux/files/home/ARMCL-RockPi/assets/coco.names
    
    adb push build/examples/Pipeline/graph_yolov3_pipeline /data/data/com.termux/files/home/ARMCL-RockPi/test_graph/
    adb shell /data/data/com.termux/files/home/ARMCL-RockPi/test_graph/graph_yolov3_pipeline --order=BBBBBBBGGGGGBBBBBLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBLLLLLLLLLLLLLLLLLLLLLLGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG --data=/data/data/com.termux/files/home/ARMCL-RockPi/assets/yolov3 --image=/data/data/com.termux/files/home/ARMCL-RockPi/assets/ppm_images_608  --labels=/data/data/com.termux/files/home/ARMCL-RockPi/assets/coco.names
    #adb shell /data/data/com.termux/files/home/ARMCL-RockPi/test_graph/graph_yolov3_pipeline --order=BBBBBBBGGGGGBBBBBLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG --data=/data/data/com.termux/files/home/ARMCL-RockPi/assets/yolov3 --image=/data/data/com.termux/files/home/ARMCL-RockPi/assets/ppm_images_608  --labels=/data/data/com.termux/files/home/ARMCL-RockPi/assets/coco.names
    #adb shell /data/data/com.termux/files/home/ARMCL-RockPi/test_graph/graph_yolov3_pipeline --order=BBBBBBBGGGGGBBBBBLLLLLLLLL --data=/data/data/com.termux/files/home/ARMCL-RockPi/assets/yolov3 --image=/data/data/com.termux/files/home/ARMCL-RockPi/assets/ppm_images_608  --labels=/data/data/com.termux/files/home/ARMCL-RockPi/assets/coco.names
    #adb shell /data/data/com.termux/files/home/ARMCL-RockPi/test_graph/graph_yolov3_pipeline --order=BBBBBBBBBB --data=/data/data/com.termux/files/home/ARMCL-RockPi/assets/yolov3 --image=/data/data/com.termux/files/home/ARMCL-RockPi/assets/ppm_images_608  --labels=/data/data/com.termux/files/home/ARMCL-RockPi/assets/coco.names
else
    echo "Command failed"
fi