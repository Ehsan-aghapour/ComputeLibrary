# cp rknn-api-1.4.0/rknn-api/rknn-api/Android/rknn_api/lib/librknn_api.so .
arch=arm-linux-androideabi
arch64=aarch64-linux-android
target_arch=${arch}

main_dir=../../..
android_ndk_dir=${main_dir}/../../android-ndk-r21e-linux-x86_64/android-ndk-r21e


cp ${android_ndk_dir}/Tools/sysroot/usr/lib/${target_arch}/libc++_shared.so .
cp ${android_ndk_dir}/Tools/sysroot/usr/lib/${target_arch}/23/liblog.so .
cp ${android_ndk_dir}/Tools/sysroot/usr/lib/${target_arch}/23/libstdc++.so .
