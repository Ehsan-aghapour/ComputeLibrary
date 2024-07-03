The link for Demo Video:
https://drive.google.com/file/d/1aogrloqdXvyLwd_TPfvzyiypv9dsnt1I/view?usp=sharing

# ARM-CO-UP (ARM Co-Operative utilization of Processors)
(Deep Edge Resource Optimizer)
***ARM-CO-UP*** is an integrated, efficient CPU-GPU-NPU CNN inference design specifically developed for ARM-based Heterogeneous Multi-Processor System-on-Chips (HMPSoCs), along with any additional accelerator. It is designed to optimize the cooperative utilization of edge processors, delivering exceptional performance and energy efficiency. 

The main design concept revolves around partitioning the network graph into partitions and executing them in parallel, using a pipeline design. The pipeline design ensures optimal utilization of all resources simultaneously, resulting in maximum throughput (measured in FPS) and energy efficiency. 

Alternatively, the layers can be mapped to the most suitable resource and switched dynamically between resources. layer-wise switching enables mapping each layer to its most appropriate processor, thereby optimizing inference latency while maintaining energy efficiency. Additionally, it supports layer-wise DVFS (dynamic voltage and frequency scaling) to dynamically adjust the performance and power of resources based on the requirements of each layer.

*ARM-CO-UP* extends its support beyond ARM little and big CPU clusters and ARM GPUs to include NPUs (Neural Processing Units) from various vendors, even if they employ vendor-specific libraries and are not open source. The framework has been implemented using an abstract NPU model that utilizes generic APIs, enabling seamless integration with NPUs of different types and specific vendor APIs.


Additionally, ARM-CO-UP offers straightforward runtime command options for executing the CNN with specific configurations. These options include mapping layers to resource types, setting the frequency of resources for each layer, and choosing between pipelines or switching inference modes. Furthermore, ARM-CO-UP generates comprehensive reports that include the average throughput (measured in FPS) and latency of inference. It also provides the flexibility to configure reporting of execution times for each stage of the pipeline or execution time for each individual layer. These detailed reports serve as essential guides for identifying the optimal design configuration based on the CNN model and hardware platform.

The extension of ARM-CO-UP built on top of ARM-CL leverages extended classes, providing a seamless migration to new versions of ARM-CL. By incorporating these extended classes, additional functionalities are introduced into the core of the ARM-CL simulator. Consequently, there is no need to implement partitioning and mapping for each individual CNN model. Instead, ARM-CO-UP automatically partitions the CNN model and maps it to the target processor type based on user-specified configurations.

ARM-CO-UP efficiently handles the execution of these partitions, either in sequential order or as a pipeline. It manages the data transfer between partitions, ensuring compatibility with the data types and formats of each processor involved. Furthermore, ARM-CO-UP intelligently addresses complex dependencies that may arise between partitions assigned to different processor types. For instance, if a partition relies on input data from one or more previous partitions, including those with branch connections (shortcuts) to the current partition, ARM-CO-UP handles synchronization and data transfer to manage these dependencies seamlessly.  



# Run the *ARM-CO-UP*
We conducted tests on ARM-CO-UP using various popular CNN models, including AlexNet, GoogleNet, MobileNet, ResNet50, and SqueezeNet. The original versions of these CNNs can be found in the examples directory, while the ARM-CO-UP versions are located in the ARM-CO-UP directory within the example directory. Notably, the implementation of ARM-CO-UP resides at the core of the framework, making it model-agnostic. This means that ARM-CO-UP works seamlessly with any model, eliminating the need for separate implementations for each new model. However, to differentiate between the original ARM-CL and ARM-CO-UP versions, a minimal annotation is added to the CNN files. As a result, the ARM-CO-UP versions of the CNN files have very slight modifications compared to their original counterparts.

git clone https://github.com/Ehsan-aghapour/ComputeLibrary -b Yolov3

After compiling the source code and preparing the libraries based on your platform run the following command:

./graph_AlexNet_all_pip_sync --threads=4 --threads2=2 --total_cores=6 --partition_point=3 --partition_point2=5 --order=G-L-B --n=60 --image=data_dir/images/ --data=data_dir/ --labels=data_dir/label.txt
<br/>
<br/>
--threads: Number of threads for Big cluster.<br/>
--threads2: Number of threads for the little cluster.<br/>
--total_cores: Number of all cores of CPU. <br/>
--partition_point: The first partitioning point. The first partitioning will happen after the layer specified with this argument.<br/>
--partition_point2: The second partitioning point. second partitioning will happen after the layer specified with this argument.<br/>
--order: The order of components in the pipeline. (G: GPU, B: CPU Big cluster, L: CPU Little cluster). For example, G-B-L order means the first subgraph runs in GPU, the Second subgraph runs in CPU Big cluster and the third subgraph runs in CPU little cluster.<br/>
--n: Number or runs. For example, 60 means running a graph for 60 frames.<br/>

The following image, data, and labels should be specified if you want to run the graph for real data. But if you want to run the network for dummy data (random data and image) do not specify these arguments:<br/>
--image: dir which includes image files. graph will run for images inside this dir.<br/>
--data: dir of graph parameters.<br/>
--labels: label file<br/>
<!--![plot](Pipe-All_0.png?width=100)-->


<br/>
<br/>
The following parts explain compiling and running ARMCL for Android and Linux platforms.
<br/>
<br/>

# Compiling for Android

First, it is required to prepare cross-compile tools to compile source code in the Linux system for the Android target. Here are the steps to download and set up tools.

1- Download Android NDK:<br/>
https://developer.android.com/ndk/downloads

2- We should create a standalone toolchain for compiling source code for Android. Based on your platform set --arch to arm or arm64 in the following command. $corss-compile-dir is your arbitrary dir at which cross-compile toolchains will be created.<br/>

$NDK/build/tools/make_standalone_toolchain.py --arch arm/arm64 --api 23 --stl gnustl --install-dir $cross_compile_dir<br/>

This command creates cross-compile toolchains at $cross-compile-dir.

3- Add $cross-compile-dir/bin to the path:<br/>
export PATH=$cross-compile-dir/bin/:$PATH

4- Go to the ARMCL source dir (cd $ARMCL-source-dir) and use the following command to compile it. Based on your platform set arch to armv7a or arm64-v8a in this command.<br/>
CXX=clang++ CC=clang scons Werror=0 debug=0 asserts=0 neon=1 opencl=1 os=android arch=armv7a/arm64-v8a -j8

# Compiling for Linux
For cross-compiling the source code in Linux host for Linux host you require:<br/>
gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf for 32 bit target <br/>
gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu for 64-bit target

Then use the following command to compile. Based on your platform set arch to armv7a or arm64-v8a in this command.<br/>
scons Werror=0 -j16 debug=0 asserts=0 neon=1 opencl=1 os=linux arch=armv7a/arm64-v8a


# Running in Android
For Android, it is required to specify the path of libOpenCL.so. First copy this library into an arbitrary dir ($lib_dir) and set LD_LIBRARY_PATH to this dir:<br/>
cp /system/lib64/egl/libGLES_mali.so $lib_dir/libOpenCL.so <br/>
export LD_LIBRARY_PATH=$lib_dir

Now it is ready to run built binaries in the build dir of ARMCL. <br/> <br/>
For AlexNet there is a zip file of model parameters, sample images, and a label file. So you can run this graph for real data and see the results. For this purpose first download this zip file at <br/> https://developer.arm.com/-/media/Arm%20Developer%20Community/Images/Tutorial%20Guide%20Diagrams%20and%20Screenshots/Machine%20Learning/Running%20AlexNet%20on%20Pi%20with%20Compute%20Library/compute_library_alexnet.zip?revision=c1a232fa-f328-451f-9bd6-250b83511e01&la=en&hash=7371AEC619F8192A9DE3E42FE6D9D18B5119E30C

make a directory and extract this zip file: <br/>
mkdir $assets_alexnet <br/>
unzip compute_library_alexnet.zip -d $assets_alexnet
<br/> <br/>
Run the AlexNet graph with this command. Select NEON or CL to run it on CPU or GPU respectively: <br/>
./build/examples/graph_alexnet Neon/CL $assets_alexnet $assets_alexnet/go_kart.ppm $assets_alexnet/labels.txt


# Running in Linux
For Linux in addition to libOpencL.so, these three libraries should be copied into the target. So first copy these libraries from the ARMCL dir:<br/>
cp build/libarm_compute.so build/libarm_compute_core.so build/libarm_compute_graph.so $lib_dir
<br/>
Then copy libOpenCL.so into $lib_dir and set LD_LIBRARY_PATH to them:<br/>
cp /system/lib64/egl/libGLES_mali.so $lib_dir/libOpenCL.so <br/>
export LD_LIBRARY_PATH=$lib_dir

Now it is ready to run built binaries in the build dir of ARMCL. <br/> <br/>
For AlexNet there is a zip file of model parameters, sample images, and a label file. So you can run this graph for real data and see the results. For this purpose first download this zip file at <br/> https://developer.arm.com/-/media/Arm%20Developer%20Community/Images/Tutorial%20Guide%20Diagrams%20and%20Screenshots/Machine%20Learning/Running%20AlexNet%20on%20Pi%20with%20Compute%20Library/compute_library_alexnet.zip?revision=c1a232fa-f328-451f-9bd6-250b83511e01&la=en&hash=7371AEC619F8192A9DE3E42FE6D9D18B5119E30C

make a directory and extract this zip file: <br/>
mkdir $assets_alexnet <br/>
unzip compute_library_alexnet.zip -d $assets_alexnet
<br/> <br/>
Run the AlexNet graph with this command. Select NEON or CL to run it on CPU or GPU respectively: <br/>
./build/examples/graph_alexnet Neon/CL $assets_alexnet $assets_alexnet/go_kart.ppm $assets_alexnet/labels.txt

<br/>
<br/>


# Add new DL model
The ARM-COUP framework is designed with a core functionality that allows for the easy addition of new models. By leveraging the original ARM-CL operations, you can incorporate any arbitrary model into the framework with minimal effort. Below, we provide a guide on how to add a new model to the ARM-COUP framework.

You can refer to the example models in the examples directory of ARM-CL, such as graph_alexnet, graph_googlenet, graph_mobilenet, and others. These examples will help you understand the structure required for your desired model.

To add a new model, follow these instructions using graph_alexnet as a reference. The graph_alexnet.cpp file is located in the examples directory.

1. Make a copy of the model into the COUP dir and change the name (you can add COUP at the end of the file name)

2. First to utilize the ARM-COUP backbone include its util file, :
#include "utils/UtilsPipeline.h"
now the model could run using the ARM-COUP backbone

3. Then, you change the parent class to COUP framework:
change this:
class GraphAlexnetExample : public Example
to this:
class GraphAlexnetExample : public Example_Pipeline


4. The variable of the CNN class are moved to the parent class (Example_Pipeline); so first comment them in the end of the class:
/*
CommandLineParser  cmd_parser;
CommonGraphOptions common_opts;
CommonGraphParams  common_params;
Stream     graph;
*/
 SO, you do not need to initialize varaiables, rather it will happen in parent class:
change this:
: cmd_parser(), common_opts(cmd_parser), common_params(), graph(0, "AlexNet")
to this:
: Example_Pipeline(0, "AlexNet")

So the following lines that set and print the common_params also need to be commented:
cmd_parser.parse(argc, argv);
cmd_parser.validate();
common_params = consume_common_graph_parameters(common_opts);
if(common_params.help)
{
    cmd_parser.print_help(argv[0]);
    return false;
}
ARM_COMPUTE_EXIT_ON_MSG(arm_compute::is_data_type_quantized_asymmetric(common_params.data_type), "QASYMM8 not supported for this graph");
std::cout << common_params << std::endl;

5. Finally, in the main function, you just need to call run function of the COUP rather than original run function:
change this:
return arm_compute::utils::run_example<GraphAlexnetExample>(argc, argv);
to this:
return arm_compute::utils::run_example_pipeline<GraphAlexnetExample>(argc, argv);

Now the model will run in ARM-COUP framewrok and all the functionalities of the framework such as co-operative run on different processors, time and power profiling, DVFS, thread management, and ... supported for the model.

ARM-COUP has a script, which automatically handle pushing the model file into the board and run with the desired arguments; if you want to run your model using this script, just need to add your model file name into cases(case "${options[model]}" in). For exmaple for alexnet it is added like this:
"Alex" | "alex")
    graph="graph_alexnet_pipeline"
    ;;

which means when you define model=Alex or model=alex it will push and run graph_alexnet_pipeline file.

Additionlly, if you want to run with the weights data (extracted from pretrained model), first extract the weights data (using scripts provided by armcl) and push it into the baord ("$p/assets/cnn_data" directory). Then set data (_dt[X]), images, and label variables. For example for Alexnet we extracted the weights data and push it into board ("$p/assets/cnn_data/" directory) and have set variables:

"Alex" | "alex")
	lbl=${_lbl}/labels.txt
	img=${_img}/ppm_images_227/
	dt=$_dt
	graph="graph_alexnet_pipeline"
	;;

Then you can mapp model layers into a desired processor (Little CPU cluster, Big CPU cluster, GPU, or NPU). So you need to know the layers of the model. Now, when you run the model with the COUP framework (no arguments needed), the framework will print the list of layers of the model, so you know the name and number of layers:
./Run_CO-UP model=AlexNet
(./Run_CO-UP model=AlexNet compile=1 -> first compile and push the model into board then runs it)


As, user usually do not want to consider each single layer(operation) as one unit, you can change the granularity by defining the super layers. For this purpose, you will define the starting and ending layers for each super layer. You can define it with two methods; 
A) in utils/main_layer_checker.h file, there are two data structures starting_task_names and ending_task_names, that for each model the name of the starting layers(task) and ending layers are stored; which you can add those for your model. you will add a new record to the starting_task_names and ending_task_names maps; with the key of the model name and the value of starting layer names and ending layer names respectively. For the key use the model name that is defined in the model file class (for example for alexnet model: GraphAlexnetExample class in graph_alexnet.cpp). Here you can see it for alexnet for example.

starting layers:
inline std::map<std::string, std::unordered_set<std::string>> starting_task_names{
	{
		"alexnet",
		{ "conv1",
			//"conv2_g0",
			//"conv2_g1",
			"conv2",
			"conv3",
			//"conv4_g0",
			//"conv4_g1",
			"conv4",
			//"conv5_g0",
			//"conv5_g1",
			"conv5",
			"fc6",
			"fc7",
			"fc8",
		}
	},
    ...

ending layers:

inline std::map<std::string, std::unordered_set<std::string>> ending_task_names{
	{
		"alexnet",
		{
			"pool1",
			"pool2",
			"relu3",
			"relu4",
			"pool5",
			"relu6",
			"relu7",
			"prob"
		}
	},
    ...

B) The second method is to define them in Layers.conf file in the main directory. In this method you put the starting and ending layer of the super layer with the following format:
[model name]
starting_task_of_super_layer_1
ending_task_of_super_layer_1
starting_task_of_super_layer2
ending_task_of_super_layer2
...
 
For example for Alexnet model:

[AlexNet]
#Layer1
conv1
pool1
#Layer2
conv2
pool2
#Layer3
conv3
relu3
#Layer4
conv4
relu4
#Layer5
conv5
pool5
#Layer6
fc6
relu6
#Layer7
fc7
relu7
#Layer8
fc8
prob
#End of AlexNet

The comment lines(start with #) are not needed, they are just for better readability. This method is easier and more readable than the previsous method. You just need to run the model with ARM-COUP without needing any argument, it will prints model name and list of the layers. Copy and append them into the Layers.conf file and based on your desired define the super layers boundaries (start and end points). This file should be placed in the same directory as binary file of the model that you finally run. At runtime it read this file and filled the starting_task_names and ending_task_names automatically.

Now, that the super layers are defined, you can map them to desired processor using --order argument; for example --order=BBBLLGGG for mapping super layers of alexnet model into Big  cluster, Little cluster and GPU.

Also, it is possible to perform super-layer level DVFS when running the model. 





 


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

Documentation (API, changelogs, build guide, contribution guide, errata, etc.) is available at https://github.com/ARM-software/ComputeLibrary/wiki/Documentation.

Binaries are available at https://github.com/ARM-software/ComputeLibrary/releases.

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
The list is open to anyone inside or outside of Arm to self-subscribe.  In order to subscribe, please visit the following website:
https://lists.linaro.org/mailman/listinfo/acl-dev

### Developer Certificate of Origin (DCO)
Before the ComputeLibrary project accepts your contribution, you need to certify its origin and give us your permission. To manage this process we use the Developer Certificate of Origin (DCO) V1.1 (https://developercertificate.org/)

To indicate that you agree to the terms of the DCO, you "sign off" your contribution by adding a line with your name and e-mail address to every git commit message:

```Signed-off-by: John Doe <john.doe@example.org>```

You must use your real name, no pseudonyms or anonymous contributions are accepted.

## Trademarks and Copyrights

Android is a trademark of Google LLC.

Arm, Cortex and Mali are registered trademarks or trademarks of Arm Limited (or its subsidiaries) in the US and/or elsewhere.

Linux® is the registered trademark of Linus Torvalds in the U.S. and other countries.

Mac and macOS are trademarks of Apple Inc., registered in the U.S. and other
countries.

Tizen is a registered trademark of The Linux Foundation.