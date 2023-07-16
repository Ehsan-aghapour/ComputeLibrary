## ./go.sh --order=BBBBGGGG
./b64.sh 23
if [ $? -eq 0 ]; then
    echo "Build succeeded"
    read -p "Continue? " name

    
    #adb push build/examples/graph_googlenet /data/data/com.termux/files/home/ARMCL-RockPi/test_graph/
    #adb shell /data/data/com.termux/files/home/ARMCL-RockPi/test_graph/graph_googlenet --data=/data/data/com.termux/files/home/ARMCL-RockPi/assets/googlenet --image=/data/data/com.termux/files/home/ARMCL-RockPi/assets/ppm_images_224/  --labels=/data/data/com.termux/files/home/ARMCL-RockPi/assets/labels.txt

    adb push build/examples/Pipeline/graph_googlenet_pipeline /data/data/com.termux/files/home/ARMCL-RockPi/test_graph/
    #adb shell /data/data/com.termux/files/home/ARMCL-RockPi/test_graph/graph_googlenet_pipeline --order=BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB --data=/data/data/com.termux/files/home/ARMCL-RockPi/assets/googlenet --image=/data/data/com.termux/files/home/ARMCL-RockPi/assets/ppm_images_224  --labels=/data/data/com.termux/files/home/ARMCL-RockPi/assets/labels.txt
    #adb shell /data/data/com.termux/files/home/ARMCL-RockPi/test_graph/graph_googlenet_pipeline --order=BBBBBBBGGGGGBBBBBLLLLLLLLL --data=/data/data/com.termux/files/home/ARMCL-RockPi/assets/googlenet --image=/data/data/com.termux/files/home/ARMCL-RockPi/assets/ppm_images_224  --labels=/data/data/com.termux/files/home/ARMCL-RockPi/assets/labels.txt
    adb shell /data/data/com.termux/files/home/ARMCL-RockPi/test_graph/graph_googlenet_pipeline --order=BBBBBB --data=/data/data/com.termux/files/home/ARMCL-RockPi/assets/googlenet --image=/data/data/com.termux/files/home/ARMCL-RockPi/assets/ppm_images_224  --labels=/data/data/com.termux/files/home/ARMCL-RockPi/assets/labels.txt
else
    echo "Command failed"
fi

#PiPushtest ./build/examples/Pipeline/graph_googlenet_pipeline test_graph/ NEON 2 2 1 10 0 0 0 0 BBBBBBBNNNNNBBBBBLLLLLLLLL 0 2 4 Google
