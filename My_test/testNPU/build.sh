
compiler=aarch64-linux-android-clang++
#compiler=arm-linux-androideabi-clang++
target=aarch64-linux-android23-clang++
#target=armv7a-linux-androideabi$1-clang++
p=/home/ehsan/UvA/ARMCL/android-ndk-r21e-linux-x86_64/android-ndk-r21e/toolchains/llvm/prebuilt/linux-x86_64/bin/
cp $p/$target $p/$compiler

 

$compiler main.cpp -Llibs -lrknn_api -static-libstdc++ -pie -Wl,-rpath,libs -o myapp
#scons 

rm $p/$compiler




