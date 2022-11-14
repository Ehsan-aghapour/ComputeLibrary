compiler=arm-linux-androideabi-clang++
target=armv7a-linux-androideabi$1-clang++
p=/home/ehsan/UvA/ARMCL/android-ndk-r21e-linux-x86_64/android-ndk-r21e/toolchains/llvm/prebuilt/linux-x86_64/bin/
cp $p/$target $p/$compiler

#XX=clang++ CC=clang scons Werror=0 -j16 debug=0 asserts=0 neon=1 opencl=1 os=android arch=armv7a 

#$compiler $2 -Iinclude/applib/ovxinc/include/ -Iinclude/service/ovx_inc/  -Llib/ -lovxlib -ljpeg_t -Wl,-rpath,/system/usr/lib/
$compiler -Iinclude/applib/ovxinc/include/ -Iinclude/service/ovx_inc/ -shared -undefined dynamic_lookup -o lib/libvnn_inceptionv3.so vnn_pre_process.c vnn_post_process.c vnn_inceptionv3.c

$compiler main.c -Iinclude/applib/ovxinc/include/ -Iinclude/service/ovx_inc/  -Llib/ -lovxlib -ljpeg_t -lvnn_inceptionv3 -Wl,-rpath,vendor/lib/
rm $p/$compiler
