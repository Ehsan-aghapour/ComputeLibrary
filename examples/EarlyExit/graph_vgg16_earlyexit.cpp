/*
 * Copyright (c) 2017-2021 Arm Limited.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "arm_compute/graph.h"
#include "support/ToolchainSupport.h"
#include "utils/CommonGraphOptions.h"
#include "utils/GraphUtils.h"
#include "utils/Utils.h"
#include "utils/UtilsPipeline.h"

using namespace arm_compute::utils;
using namespace arm_compute::graph::frontend;
using namespace arm_compute::graph_utils;

/** Example demonstrating how to implement VGG16's network using the Compute Library's graph API */
class GraphVGG16EEExample : public Example_Pipeline
{
public:
    GraphVGG16EEExample()
        : Example_Pipeline(0, "VGG16")
    {
    }
    bool do_setup(int argc, char **argv) override
    {
        // Parse arguments
        /*cmd_parser.parse(argc, argv);
        cmd_parser.validate();

        // Consume common parameters
        common_params = consume_common_graph_parameters(common_opts);

        // Return when help menu is requested
        if(common_params.help)
        {
            cmd_parser.print_help(argv[0]);
            return false;
        }

        // Print parameter values
        std::cout << common_params << std::endl;*/

        // Get trainable parameters data path
        std::string data_path = common_params.data_path;

        // Create a preprocessor object
        const std::array<float, 3> mean_rgb{ { 123.68f, 116.779f, 103.939f } };
        std::unique_ptr<IPreprocessor> preprocessor = std::make_unique<CaffePreproccessor>(mean_rgb);

        // Create input descriptor
        const auto        operation_layout = common_params.data_layout;
        const TensorShape tensor_shape     = permute_shape(TensorShape(32U, 32U, 3U, 1U), DataLayout::NCHW, operation_layout);
        TensorDescriptor  input_descriptor = TensorDescriptor(tensor_shape, common_params.data_type).set_layout(operation_layout);

        // Set weights trained layout
        const DataLayout weights_layout = DataLayout::NCHW;

        bool early_exits=true;

        // Create graph
        graph << common_params.target
              << common_params.fast_math_hint
              << InputLayer(input_descriptor, get_input_accessor(common_params, std::move(preprocessor)))
              // Layer 1
              << ConvolutionLayer(
                  3U, 3U, 64U,
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv1_1_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv1_1_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv1_1")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv1_1/Relu");
			  if(early_exits){
					SubStream EE0(graph);
					EE0<< ConvolutionLayer(
						  3U, 3U, 64U,
						  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv1_1_w.npy", weights_layout),
						  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv1_1_b.npy"),
						  PadStrideInfo(2, 2, 1, 1))
					  .set_name("EE0/conv1")
					<< ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("EE0/conv1/Relu");
					EE0<< ConvolutionLayer(
						  3U, 3U, 32U,
						  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv1_1_w.npy", weights_layout),
						  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv1_1_b.npy"),
						  PadStrideInfo(1, 1, 1, 1))
					  .set_name("EE0/conv2")
					<< ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("EE0/conv2/Relu");
					EE0<< ConvolutionLayer(
						  3U, 3U, 32U,
						  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv1_1_w.npy", weights_layout),
						  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv1_1_b.npy"),
						  PadStrideInfo(1, 1, 1, 1))
					  .set_name("EE0/conv3")
					<< ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("EE0/conv3/Relu");
					EE0<< FullyConnectedLayer(
						  100U,
						  get_weights_accessor(data_path, "/cnn_data/vgg16_model/fc8_w.npy", weights_layout),
						  get_weights_accessor(data_path, "/cnn_data/vgg16_model/fc8_b.npy"))
					  .set_name("EE0/FC")
					  // Softmax
					  << SoftmaxLayer().set_name("EE0/prob")
					  << EarlyExitOutputLayer(get_output_accessor(common_params, 5));
			  }

              // Layer 2
         graph<< ConvolutionLayer(
                  3U, 3U, 64U,
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv1_2_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv1_2_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv1_2")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv1_2/Relu")
              << PoolingLayer(PoolingLayerInfo(PoolingType::MAX, 2, operation_layout, PadStrideInfo(2, 2, 0, 0))).set_name("pool1")
              // Layer 3
              << ConvolutionLayer(
                  3U, 3U, 128U,
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv2_1_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv2_1_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv2_1")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv2_1/Relu");

			  if(early_exits){
					SubStream EE1(graph);
					EE1<< ConvolutionLayer(
						  3U, 3U, 128U,
						  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv1_1_w.npy", weights_layout),
						  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv1_1_b.npy"),
						  PadStrideInfo(2, 2, 1, 1))
					  .set_name("EE1/conv1")
					<< ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("EE1/conv1/Relu");
					EE1<< ConvolutionLayer(
						  3U, 3U, 64U,
						  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv1_1_w.npy", weights_layout),
						  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv1_1_b.npy"),
						  PadStrideInfo(1, 1, 1, 1))
					  .set_name("EE1/conv2")
					<< ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("EE1/conv2/Relu");
					EE1<< FullyConnectedLayer(
						  100U,
						  get_weights_accessor(data_path, "/cnn_data/vgg16_model/fc8_w.npy", weights_layout),
						  get_weights_accessor(data_path, "/cnn_data/vgg16_model/fc8_b.npy"))
					  .set_name("EE1/FC")
					  // Softmax
					  << SoftmaxLayer().set_name("EE1/prob")
					  << EarlyExitOutputLayer(get_output_accessor(common_params, 5));
			  }


              // Layer 4
              graph<< ConvolutionLayer(
                  3U, 3U, 128U,
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv2_2_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv2_2_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv2_2")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv2_2/Relu")
              << PoolingLayer(PoolingLayerInfo(PoolingType::MAX, 2, operation_layout, PadStrideInfo(2, 2, 0, 0))).set_name("pool2")
              // Layer 5
              << ConvolutionLayer(
                  3U, 3U, 256U,
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv3_1_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv3_1_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv3_1")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv3_1/Relu")
              // Layer 6
              << ConvolutionLayer(
                  3U, 3U, 256U,
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv3_2_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv3_2_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv3_2")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv3_2/Relu")
              // Layer 7
              << ConvolutionLayer(
                  3U, 3U, 256U,
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv3_3_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv3_3_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv3_3")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv3_3/Relu")
              << PoolingLayer(PoolingLayerInfo(PoolingType::MAX, 2, operation_layout, PadStrideInfo(2, 2, 0, 0))).set_name("pool3")
              // Layer 8
              << ConvolutionLayer(
                  3U, 3U, 512U,
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv4_1_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv4_1_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv4_1")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv4_1/Relu")
              // Layer 9
              << ConvolutionLayer(
                  3U, 3U, 512U,
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv4_2_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv4_2_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv4_2")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv4_2/Relu")
              // Layer 10
              << ConvolutionLayer(
                  3U, 3U, 512U,
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv4_3_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv4_3_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv4_3")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv4_3/Relu")
              << PoolingLayer(PoolingLayerInfo(PoolingType::MAX, 2, operation_layout, PadStrideInfo(2, 2, 0, 0))).set_name("pool4")
              // Layer 11
              << ConvolutionLayer(
                  3U, 3U, 512U,
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv5_1_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv5_1_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv5_1")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv5_1/Relu")
              // Layer 12
              << ConvolutionLayer(
                  3U, 3U, 512U,
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv5_2_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv5_2_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv5_2")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv5_2/Relu")
              // Layer 13
              << ConvolutionLayer(
                  3U, 3U, 512U,
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv5_3_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/conv5_3_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv5_3")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv5_3/Relu")
              << PoolingLayer(PoolingLayerInfo(PoolingType::MAX, 2, operation_layout, PadStrideInfo(2, 2, 0, 0))).set_name("pool5")
              // Layer 14
              << FullyConnectedLayer(
                  4096U,
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/fc6_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/fc6_b.npy"))
              .set_name("fc6")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("Relu")
              // Layer 15
              << FullyConnectedLayer(
                  4096U,
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/fc7_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/fc7_b.npy"))
              .set_name("fc7")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("Relu_1")
              // Layer 16
              << FullyConnectedLayer(
                  100U,
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/fc8_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg16_model/fc8_b.npy"))
              .set_name("fc8")
              // Softmax
              << SoftmaxLayer().set_name("prob")
              << OutputLayer(get_output_accessor(common_params, 5));

        // Finalize graph
        GraphConfig config;
        config.num_threads      = common_params.threads;
        config.use_tuner        = common_params.enable_tuner;
        config.tuner_mode       = common_params.tuner_mode;
        config.tuner_file       = common_params.tuner_file;
        config.mlgo_file        = common_params.mlgo_file;
        config.convert_to_uint8 = (common_params.data_type == DataType::QASYMM8);

        graph.finalize(common_params.target, config);

        return true;
    }
    void do_run() override
    {
        // Run graph
        graph.run();
    }

private:
    /*CommandLineParser  cmd_parser;
    CommonGraphOptions common_opts;
    CommonGraphParams  common_params;
    Stream             graph;
    */
    void add_sepconv(SubStream &stream, int channels_in, int channels_out, int kernel_size, int stride, int padding, std::string pre_name="",int _i=-1){

        	/**stream<<ConvolutionLayer(
        			kernel_size, kernel_size, channels,
                    get_weights_accessor("", "", DataLayout::NCHW),
                    std::unique_ptr<arm_compute::graph::ITensorAccessor>(nullptr),
                    PadStrideInfo(stride, stride, padding, padding))
                .set_name("conv2/convolution");*/
        	static int i=0;
        	if(_i!=-1){
        		i=_i;
        	}
        	std::string _pre_name=pre_name+"sepConv_"+std::to_string(i++)+"/";
        	stream<<DepthwiseConvolutionLayer(kernel_size, kernel_size,
        			get_weights_accessor("", "", DataLayout::NCHW),
    				std::unique_ptr<arm_compute::graph::ITensorAccessor>(nullptr),
    				PadStrideInfo(stride, stride, padding, padding))
    			 .set_name(_pre_name+"depthwiseConv_0");
        	stream<<ConvolutionLayer(
        	    			1, 1, channels_in,
        	                get_weights_accessor("", "", DataLayout::NCHW),
        	                std::unique_ptr<arm_compute::graph::ITensorAccessor>(nullptr),
        	                PadStrideInfo(1, 1, 0, 0))
        	            .set_name(_pre_name+"pointwiseConv_0");
        	stream<< BatchNormalizationLayer(
                    get_weights_accessor("", ""),
    				get_weights_accessor("", ""),
    				get_weights_accessor("", ""),
    				get_weights_accessor("", ""),
                    0.0000100099996416f).set_name("BN");
        	stream << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("Relu");

        	/**stream<<ConvolutionLayer(
    				kernel_size, kernel_size, channels,
    				get_weights_accessor("", "", DataLayout::NCHW),
    				std::unique_ptr<arm_compute::graph::ITensorAccessor>(nullptr),
    				PadStrideInfo(stride, stride, padding, padding))
    			.set_name("conv2/convolution");*/
    		stream<<DepthwiseConvolutionLayer(kernel_size, kernel_size,
    				get_weights_accessor("", "", DataLayout::NCHW),
    				std::unique_ptr<arm_compute::graph::ITensorAccessor>(nullptr),
    				PadStrideInfo(1, 1, padding, padding))
    			 .set_name(_pre_name+"depthwiseConv_1");
        	stream<<ConvolutionLayer(
    						1, 1, channels_out,
    						get_weights_accessor("", "", DataLayout::NCHW),
    						std::unique_ptr<arm_compute::graph::ITensorAccessor>(nullptr),
    						PadStrideInfo(1, 1, 0, 0))
    					.set_name(_pre_name+"pointwiseConv_1");
        	stream<< BatchNormalizationLayer(
    				get_weights_accessor("", ""),
    				get_weights_accessor("", ""),
    				get_weights_accessor("", ""),
    				get_weights_accessor("", ""),
    				0.0000100099996416f).set_name("BN");
        	stream << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("Relu");

    		return ;


        }

        void add_attention(SubStream &stream, int channel_size){
        	//SubStream right(stream);
        	static int i=0;
        	std::string pre_name="attention_"+std::to_string(i++)+"/";
        	add_sepconv(stream, channel_size,channel_size, 3, 2, 1,pre_name,0);
        	stream<< BatchNormalizationLayer(
        	                get_weights_accessor("", ""),
        					get_weights_accessor("", ""),
        					get_weights_accessor("", ""),
        					get_weights_accessor("", ""),
        	                0.0000100099996416f).set_name("BN");
        	stream << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("Relu");
        	stream << ResizeLayer(InterpolationPolicy::BILINEAR, 2, 2).set_name("Upsample");
        	stream << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::LOGISTIC)).set_name("Sigmoid");
        	//stream << EltwiseLayer(std::move(stream), std::move(right), EltwiseOperation::Mul).set_name(pre_name+"mul");
        	//stream << EltwiseLayer(stream, right, EltwiseOperation::Mul).set_name(pre_name+"mul");
        }

        void add_scala(SubStream  &stream, std::vector<int> channel_sizes){
        	static int i=0;
        	std::string pre_name="scala_"+std::to_string(i++)+"/";
        	int k=0;
        	for(int channel_size:channel_sizes){
        		add_sepconv(stream, channel_size,2*channel_size, 3, 2, 1,pre_name,k);
        		k=-1;
        	}
        	stream << PoolingLayer(PoolingLayerInfo(PoolingType::AVG, DataLayout::NHWC)).set_name(pre_name+"poolAVG");
        }

};

/** Main program for VGG16
 *
 * Model is based on:
 *      https://arxiv.org/abs/1409.1556
 *      "Very Deep Convolutional Networks for Large-Scale Image Recognition"
 *      Karen Simonyan, Andrew Zisserman
 *
 * Provenance: www.robots.ox.ac.uk/~vgg/software/very_deep/caffe/VGG_ILSVRC_16_layers.caffemodel
 *
 * @note To list all the possible arguments execute the binary appended with the --help option
 *
 * @param[in] argc Number of arguments
 * @param[in] argv Arguments
 */
int main(int argc, char **argv)
{
    return arm_compute::utils::run_example_pipeline<GraphVGG16EEExample>(argc, argv);
}
