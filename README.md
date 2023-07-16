# ARMCL-DERO (Deep Edge Resource Optimizer)
***DERO*** is an integrated, efficient CPU-GPU-NPU CNN inference design specifically developed for ARM-based Heterogeneous Multi-Processor System-on-Chips (HMPSoCs), along with any additional accelerator. It is designed to optimize the utilization of edge resources, delivering exceptional performance and energy efficiency. 

The main design concept revolves around partitioning the network graph into partitions and executing them in parallel, using a pipeline design. The pipeline design ensures optimal utilization of all resources simultaneously, resulting in maximum throughput (measured in FPS) and energy efficiency. 

Alternatively, the layers can be mapped to the most suitable resource and switched dynamically between resources. layer-wise switching enables mapping each layer to its most suitable resource, thereby optimizing inference latency while maintaining energy efficiency. Additionally, it supports layer-wise DVFS (dynamic voltage and frequency scaling) to dynamically adjust the performance and power of resources based on the requirements of each layer.

*DERO* extends its support beyond ARM little and big CPU clusters and ARM GPUs to include NPUs (Neural Processing Units) from various vendors, even if they employ vendor-specific libraries and are not open source. The framework has been implemented using an abstract NPU model that utilizes generic APIs, enabling seamless integration with NPUs of different types and specific vendor APIs.


Additionally, DERO offers straightforward runtime command options for executing the CNN with specific configurations. These options include mapping layers to resource types, setting the frequency of resources for each layer, and choosing between pipeline or switching inference modes. Furthermore, DERO generates comprehensive reports that include the average throughput (measured in FPS) and latency of inference. It also provides the flexibility to configure reporting of execution times for each stage of the pipeline or execution time for each individual layer. These detailed reports serve as essential guides for identifying the optimal design configuration based on the CNN model and hardware platform.

The extension of DERO built on top of ARM-CL leverages extended classes, providing seamless migration to new versions of ARM-CL. By incorporating these extended classes, additional functionalities are introduced into the core of the ARM-CL simulator. Consequently, there is no need to implement partitioning and mapping for each individual CNN model. Instead, DERO automatically partitions the CNN model and maps it to the target processor type based on user-specified configurations.

DERO efficiently handles the execution of these partitions, either in sequential order or as a pipeline. It manages the data transfer between partitions, ensuring compatibility with the data types and formats of each processor involved. Furthermore, DERO intelligently addresses complex dependencies that may arise between partitions assigned to different processor types. For instance, if a partition relies on input data from one or more previous partitions, including those with branch connections (shortcuts) to the current partition, DERO handles synchronization and data transfer to manage these dependencies seamlessly.  



# Run the *DERO*
We conducted tests on DERO using various popular CNN models, including AlexNet, GoogleNet, MobileNet, ResNet50, and SqueezeNet. The original versions of these CNNs can be found in the examples directory, while the DERO versions are located in the DERO directory within the example directory. Importantly, the implementation of DERO resides at the core of the framework, making it model-agnostic. This means that DERO works seamlessly with any model, eliminating the need for separate implementations for each new model. However, to differentiate between the original ARM-CL and DERO versions, a minimal annotation is added to the CNN files. As a result, the DERO versions of the CNN files have very slight modifications compared to their original counterparts.

git clone https://github.com/Ehsan-aghapour/ComputeLibrary -b Yolov3

After compiling the source code and preparing the libraries based on your platform run the following command:

./graph_AlexNet_all_pip_sync --threads=4 --threads2=2 --total_cores=6 --partition_point=3 --partition_point2=5 --order=G-L-B --n=60 --image=data_dir/images/ --data=data_dir/ --labels=data_dir/label.txt
<br/>
<br/>
--threads: Number of threads for Big cluster.<br/>
--threads2: Number of threads for little cluster.<br/>
--total_cores: Number of all cores of CPU. <br/>
--partition_point: The first partitioning point. First partitioning will happen after layer specified with this argument.<br/>
--partition_point2: The second partitioning point. second partitioning will happen after layer specified with this argument.<br/>
--order: The order of components in the pipeline. (G:GPU, B:CPU Big cluster, L:CPU Little cluster). For example G-B-L order means first subgraph runs in GPU, Second subgraph runs in CPU Big cluster and third subgraph runs in CPU little cluster.<br/>
--n: Number or runs. For example 60 means running graph for 60 frames.<br/>

The following image, data and lablels should specified if you want to run the graph for real data. But if you want to run the network for dummy data (random data and image) do not specify this arguments:<br/>
--image: dir which include image files. graph will run for images inside this dir.<br/>
--data: dir of graph parameters.<br/>
--labels: label file<br/>
<!--![plot](Pipe-All_0.png?width=100)-->


<br/>
<br/>
The following parts explain compiling and running ARMCL for android and linux platforms.
<br/>
<br/>

# Compiling for Android

First it is required to prepare cross compile tools to compile source code in linux system for android target. Here is the steps to download and settup tools.

1- Download Android NDK:<br/>
https://developer.android.com/ndk/downloads

2- We should create a standalone toolchains for compiling source code for android. Based on your platform set --arch to arm or arm64 in the following command. $corss-compile-dir is your arbitrary dir at which cross compile toolchains will be created.<br/>

$NDK/build/tools/make_standalone_toolchain.py --arch arm/arm64 --api 23 --stl gnustl --install-dir $cross_compile_dir<br/>

This command create cross compile toolchains at $cross-compile-dir.

3- Add $cross-compile-dir/bin to the path:<br/>
export PATH=$cross-compile-dir/bin/:$PATH

4- Go to the ARMCL source dir (cd $ARMCL-source-dir) and use the following command to compile it. Based on your platform set arch to armv7a or arm64-v8a in this command.<br/>
CXX=clang++ CC=clang scons Werror=0 debug=0 asserts=0 neon=1 opencl=1 os=android arch=armv7a/arm64-v8a -j8

# Compiling for linux
For cross compiling the source code in linux host for linux host you require:<br/>
gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf for 32 bit target <br/>
gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu for 64 bit target

Then use the following command to compile. Based on your platform set arch to armv7a or arm64-v8a in this command.<br/>
scons Werror=0 -j16 debug=0 asserts=0 neon=1 opencl=1 os=linux arch=armv7a/arm64-v8a


# Running in Android
For android it is required to specify the path of libOpenCL.so. First copy this library into an arbitrary dir ($lib_dir) and set LD_LIBRARY_PATH to this dir:<br/>
cp /system/lib64/egl/libGLES_mali.so $lib_dir/libOpenCL.so <br/>
export LD_LIBRARY_PATH=$lib_dir

Now it is ready to run built binaries in build dir of ARMCL. <br/> <br/>
For AlexNet there is a zip file of model paramters, sample images and a label file. So you can run this graph for real data and see the results. For this purpose first download this zip file at: <br/> https://developer.arm.com/-/media/Arm%20Developer%20Community/Images/Tutorial%20Guide%20Diagrams%20and%20Screenshots/Machine%20Learning/Running%20AlexNet%20on%20Pi%20with%20Compute%20Library/compute_library_alexnet.zip?revision=c1a232fa-f328-451f-9bd6-250b83511e01&la=en&hash=7371AEC619F8192A9DE3E42FE6D9D18B5119E30C

make a directory and extract this zip file: <br/>
mkdir $assets_alexnet <br/>
unzip compute_library_alexnet.zip -d $assets_alexnet
<br/> <br/>
Run the AlexNet graph with this command. Select NEON or CL to run it on CPU or GPU respectively: <br/>
./build/examples/graph_alexnet Neon/CL $assets_alexnet $assets_alexnet/go_kart.ppm $assets_alexnet/labels.txt


# Running in Linux
For linux in addition to libOpencL.so, these three libraries should be copied into target. So first copy these libraries from the ARMCL dir:<br/>
cp build/libarm_compute.so build/libarm_compute_core.so build/libarm_compute_graph.so $lib_dir
<br/>
Then copy libOpenCL.so into $lib_dir and set LD_LIBRARY_PATH to them:<br/>
cp /system/lib64/egl/libGLES_mali.so $lib_dir/libOpenCL.so <br/>
export LD_LIBRARY_PATH=$lib_dir

Now it is ready to run built binaries in build dir of ARMCL. <br/> <br/>
For AlexNet there is a zip file of model paramters, sample images and a label file. So you can run this graph for real data and see the results. For this purpose first download this zip file at: <br/> https://developer.arm.com/-/media/Arm%20Developer%20Community/Images/Tutorial%20Guide%20Diagrams%20and%20Screenshots/Machine%20Learning/Running%20AlexNet%20on%20Pi%20with%20Compute%20Library/compute_library_alexnet.zip?revision=c1a232fa-f328-451f-9bd6-250b83511e01&la=en&hash=7371AEC619F8192A9DE3E42FE6D9D18B5119E30C

make a directory and extract this zip file: <br/>
mkdir $assets_alexnet <br/>
unzip compute_library_alexnet.zip -d $assets_alexnet
<br/> <br/>
Run the AlexNet graph with this command. Select NEON or CL to run it on CPU or GPU respectively: <br/>
./build/examples/graph_alexnet Neon/CL $assets_alexnet $assets_alexnet/go_kart.ppm $assets_alexnet/labels.txt

<br/>
<br/>


<div align="center">
 <img src="https://raw.githubusercontent.com/ARM-software/ComputeLibrary/gh-pages/ACL_logo.png"><br><br>
</div>

Release repository: https://github.com/arm-software/ComputeLibrary

Development repository: https://review.mlplatform.org/#/admin/projects/ml/ComputeLibrary

Please report issues here: https://github.com/ARM-software/ComputeLibrary/issues

**Make sure you are using the latest version of the library before opening an issue. Thanks**

News:

- [Gian Marco's talk on Performance Analysis for Optimizing Embedded Deep Learning Inference Software](https://www.embedded-vision.com/platinum-members/arm/embedded-vision-training/videos/pages/may-2019-embedded-vision-summit)
- [Gian Marco's talk on optimizing CNNs with Winograd algorithms at the EVS](https://www.embedded-vision.com/platinum-members/arm/embedded-vision-training/videos/pages/may-2018-embedded-vision-summit-iodice)
- [Gian Marco's talk on using SGEMM and FFTs to Accelerate Deep Learning](https://www.embedded-vision.com/platinum-members/arm/embedded-vision-training/videos/pages/may-2016-embedded-vision-summit-iodice)

Related projects:

- [Arm NN SDK](https://github.com/arm-software/armnn)

Tutorials:

- [Tutorial: Cartoonifying Images on Raspberry Pi with the Compute Library](https://community.arm.com/graphics/b/blog/posts/cartoonifying-images-on-raspberry-pi-with-the-compute-library)
- [Tutorial: Running AlexNet on Raspberry Pi with Compute Library](https://community.arm.com/processors/b/blog/posts/running-alexnet-on-raspberry-pi-with-compute-library)

Documentation (API, changelogs, build guide, contribution guide, errata, etc.) available at https://github.com/ARM-software/ComputeLibrary/wiki/Documentation.

Binaries available at https://github.com/ARM-software/ComputeLibrary/releases.

### Supported Architectures/Technologies

- Arm® CPUs:
    - Arm® Cortex®-A processor family using Arm® Neon™ technology
    - Arm® Cortex®-R processor family with Armv8-R AArch64 architecture using Arm® Neon™ technology
    - Arm® Cortex®-X1 processor using Arm® Neon™ technology

- Arm® Mali™ GPUs:
    - Arm® Mali™-G processor family
    - Arm® Mali™-T processor family

- x86

### Supported OS

- Android™
- Bare Metal
- Linux®
- macOS®
- Tizen™

## License and Contributions

The software is provided under MIT license. Contributions to this project are accepted under the same license.

### Public mailing list
For technical discussion, the ComputeLibrary project has a public mailing list: acl-dev@lists.linaro.org
The list is open to anyone inside or outside of Arm to self subscribe.  In order to subscribe, please visit the following website:
https://lists.linaro.org/mailman/listinfo/acl-dev

### Developer Certificate of Origin (DCO)
Before the ComputeLibrary project accepts your contribution, you need to certify its origin and give us your permission. To manage this process we use the Developer Certificate of Origin (DCO) V1.1 (https://developercertificate.org/)

To indicate that you agree to the the terms of the DCO, you "sign off" your contribution by adding a line with your name and e-mail address to every git commit message:

```Signed-off-by: John Doe <john.doe@example.org>```

You must use your real name, no pseudonyms or anonymous contributions are accepted.

## Trademarks and Copyrights

Android is a trademark of Google LLC.

Arm, Cortex and Mali are registered trademarks or trademarks of Arm Limited (or its subsidiaries) in the US and/or elsewhere.

Linux® is the registered trademark of Linus Torvalds in the U.S. and other countries.

Mac and macOS are trademarks of Apple Inc., registered in the U.S. and other
countries.

Tizen is a registered trademark of The Linux Foundation.