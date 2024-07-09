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

/** Example demonstrating how to implement ResNet18 network using the Compute Library's graph API */
class GraphResNet18EEExample : public Example_Pipeline
{
public:
	GraphResNet18EEExample()
		: Example_Pipeline(0, "ResNet18")
    {
    }
    bool do_setup(int argc, char **argv) override
    {
        // Parse arguments
/*        cmd_parser.parse(argc, argv);
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
        std::cout << common_params << std::endl;
        */

        // Get trainable parameters data path
        std::string data_path = common_params.data_path;

        // Create a preprocessor object
        const std::array<float, 3> mean_rgb{ { 122.68f, 116.67f, 104.01f } };
        std::unique_ptr<IPreprocessor> preprocessor = std::make_unique<CaffePreproccessor>(mean_rgb,
                                                                                           false /* Do not convert to BGR */);

        // Create input descriptor
        const auto        operation_layout = common_params.data_layout;
        const TensorShape tensor_shape     = permute_shape(TensorShape(32U, 32U, 3U, 1U), DataLayout::NCHW, operation_layout);
        TensorDescriptor  input_descriptor = TensorDescriptor(tensor_shape, common_params.data_type).set_layout(operation_layout);

        // Set weights trained layout
        const DataLayout weights_layout = DataLayout::NCHW;

        graph << common_params.target
              << common_params.fast_math_hint
              << InputLayer(input_descriptor, get_input_accessor(common_params, std::move(preprocessor), false /* Do not convert to BGR */))
              << ConvolutionLayer(
                  3U, 3U, 64U,
                  get_weights_accessor(data_path, "/cnn_data/resnet50_model/conv1_weights.npy", weights_layout),
                  std::unique_ptr<arm_compute::graph::ITensorAccessor>(nullptr),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv1/convolution")
              << BatchNormalizationLayer(
                  get_weights_accessor(data_path, "/cnn_data/resnet50_model/conv1_BatchNorm_moving_mean.npy"),
                  get_weights_accessor(data_path, "/cnn_data/resnet50_model/conv1_BatchNorm_moving_variance.npy"),
                  get_weights_accessor(data_path, "/cnn_data/resnet50_model/conv1_BatchNorm_gamma.npy"),
                  get_weights_accessor(data_path, "/cnn_data/resnet50_model/conv1_BatchNorm_beta.npy"),
                  0.0000100099996416f)
              .set_name("conv1/BatchNorm")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv1/Relu");
              //<< PoolingLayer(PoolingLayerInfo(PoolingType::MAX, 3, operation_layout, PadStrideInfo(2, 2, 0, 1, 0, 1, DimensionRoundingType::FLOOR))).set_name("pool1/MaxPool");

        add_residual_block(data_path, "block1", weights_layout, 64, 2, 1);

        /*******************First early exit branch*********************/
        SubStream EE0(graph);
        add_attention(EE0, 64);
        add_scala(EE0, {64,128,256});
        EE0<< ConvolutionLayer(
				  1U, 1U, 100U,
				  get_weights_accessor(data_path, "/cnn_data/resnet50_model/logits_weights.npy", weights_layout),
				  get_weights_accessor(data_path, "/cnn_data/resnet50_model/logits_biases.npy"),
				  PadStrideInfo(1, 1, 0, 0))
			  .set_name("EE0/FC")
			  << FlattenLayer().set_name("EE0/predictions/Reshape")
			  << SoftmaxLayer().set_name("EE0/predictions/Softmax")
			  << EarlyExitOutputLayer(get_output_accessor(common_params, 5));
        /****************************************************************/


        add_residual_block(data_path, "block2", weights_layout, 128, 2, 2);

        /*******************Second early exit branch*********************/
		SubStream EE1(graph);
		add_attention(EE1, 128);
		add_scala(EE1, {128,256});
		EE1<< ConvolutionLayer(
				  1U, 1U, 100U,
				  get_weights_accessor(data_path, "/cnn_data/resnet50_model/logits_weights.npy", weights_layout),
				  get_weights_accessor(data_path, "/cnn_data/resnet50_model/logits_biases.npy"),
				  PadStrideInfo(1, 1, 0, 0))
			  .set_name("EE1/FC")
			  << FlattenLayer().set_name("EE1/predictions/Reshape")
			  << SoftmaxLayer().set_name("EE1/predictions/Softmax")
			  << EarlyExitOutputLayer(get_output_accessor(common_params, 5));
		/****************************************************************/


        add_residual_block(data_path, "block3", weights_layout, 256, 2, 2);

        /*******************Third early exit branch*********************/
		SubStream EE2(graph);
		add_attention(EE2, 256);
		add_scala(EE2, {256});
		EE2<< ConvolutionLayer(
				  1U, 1U, 100U,
				  get_weights_accessor(data_path, "/cnn_data/resnet50_model/logits_weights.npy", weights_layout),
				  get_weights_accessor(data_path, "/cnn_data/resnet50_model/logits_biases.npy"),
				  PadStrideInfo(1, 1, 0, 0))
			  .set_name("EE2/FC")
			  << FlattenLayer().set_name("EE2/predictions/Reshape")
			  << SoftmaxLayer().set_name("EE2/predictions/Softmax")
			  << EarlyExitOutputLayer(get_output_accessor(common_params, 5));
		/****************************************************************/


        add_residual_block(data_path, "block4", weights_layout, 512, 2, 2);


        graph << PoolingLayer(PoolingLayerInfo(PoolingType::AVG, operation_layout)).set_name("pool5")
              << ConvolutionLayer(
                  1U, 1U, 100U,
                  get_weights_accessor(data_path, "/cnn_data/resnet50_model/logits_weights.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/resnet50_model/logits_biases.npy"),
                  PadStrideInfo(1, 1, 0, 0))
              .set_name("logits/convolution")
              << FlattenLayer().set_name("predictions/Reshape")
              << SoftmaxLayer().set_name("predictions/Softmax")
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
    Stream             graph;*/

    void add_residual_block(const std::string &data_path, const std::string &name, DataLayout weights_layout,
                            unsigned int base_depth, unsigned int num_units, unsigned int stride)
    {
        for(unsigned int i = 0; i < num_units; ++i)
        {
            std::stringstream unit_path_ss;
            unit_path_ss << "/cnn_data/resnet50_model/" << name << "_unit_" << (i + 1) << "_bottleneck_v1_";
            std::stringstream unit_name_ss;
            unit_name_ss << name << "/unit" << (i + 1) << "/bottleneck_v1/";

            std::string unit_path = unit_path_ss.str();
            std::string unit_name = unit_name_ss.str();

            unsigned int middle_stride = 1;

            /*if(i == (num_units - 1))
            {
                middle_stride = stride;
            }*/

            if(i == 0 and stride!=1){
            	middle_stride=stride;
            }

            SubStream right(graph);
            //Change for resnet18
            /*right << ConvolutionLayer(
                      1U, 1U, base_depth,
                      get_weights_accessor(data_path, unit_path + "conv1_weights.npy", weights_layout),
                      std::unique_ptr<arm_compute::graph::ITensorAccessor>(nullptr),
                      PadStrideInfo(1, 1, 0, 0))
                  .set_name(unit_name + "conv1/convolution")
                  << BatchNormalizationLayer(
                      get_weights_accessor(data_path, unit_path + "conv1_BatchNorm_moving_mean.npy"),
                      get_weights_accessor(data_path, unit_path + "conv1_BatchNorm_moving_variance.npy"),
                      get_weights_accessor(data_path, unit_path + "conv1_BatchNorm_gamma.npy"),
                      get_weights_accessor(data_path, unit_path + "conv1_BatchNorm_beta.npy"),
                      0.0000100099996416f)
                  .set_name(unit_name + "conv1/BatchNorm")
                  << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name(unit_name + "conv1/Relu")*/
            right << ConvolutionLayer(
                      3U, 3U, base_depth,
                      get_weights_accessor(data_path, unit_path + "conv2_weights.npy", weights_layout),
                      std::unique_ptr<arm_compute::graph::ITensorAccessor>(nullptr),
                      PadStrideInfo(middle_stride, middle_stride, 1, 1))
                  .set_name(unit_name + "conv2/convolution")
                  << BatchNormalizationLayer(
                      get_weights_accessor(data_path, unit_path + "conv2_BatchNorm_moving_mean.npy"),
                      get_weights_accessor(data_path, unit_path + "conv2_BatchNorm_moving_variance.npy"),
                      get_weights_accessor(data_path, unit_path + "conv2_BatchNorm_gamma.npy"),
                      get_weights_accessor(data_path, unit_path + "conv2_BatchNorm_beta.npy"),
                      0.0000100099996416f)
                  .set_name(unit_name + "conv2/BatchNorm")
                  << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name(unit_name + "conv1/Relu")

                  << ConvolutionLayer(
                		  //Change for resnet18
                      //1U, 1U, base_depth * 4,
                	  1U, 1U, base_depth,
                      get_weights_accessor(data_path, unit_path + "conv3_weights.npy", weights_layout),
                      std::unique_ptr<arm_compute::graph::ITensorAccessor>(nullptr),
                      PadStrideInfo(1, 1, 0, 0))
                  .set_name(unit_name + "conv3/convolution")
                  << BatchNormalizationLayer(
                      get_weights_accessor(data_path, unit_path + "conv3_BatchNorm_moving_mean.npy"),
                      get_weights_accessor(data_path, unit_path + "conv3_BatchNorm_moving_variance.npy"),
                      get_weights_accessor(data_path, unit_path + "conv3_BatchNorm_gamma.npy"),
                      get_weights_accessor(data_path, unit_path + "conv3_BatchNorm_beta.npy"),
                      0.0000100099996416f)
                  .set_name(unit_name + "conv2/BatchNorm");

            //Change for resnet18
            if(i == 0 and stride!=1)
            {
                SubStream left(graph);
                left << ConvolutionLayer(
                		//Change for resnet18
                         //1U, 1U, base_depth * 4,
						 1U, 1U, base_depth,
                         get_weights_accessor(data_path, unit_path + "shortcut_weights.npy", weights_layout),
                         std::unique_ptr<arm_compute::graph::ITensorAccessor>(nullptr),
                         //PadStrideInfo(1, 1, 0, 0))
						 PadStrideInfo(middle_stride, middle_stride, 0, 0))
                     .set_name(unit_name + "shortcut/convolution")
                     << BatchNormalizationLayer(
                         get_weights_accessor(data_path, unit_path + "shortcut_BatchNorm_moving_mean.npy"),
                         get_weights_accessor(data_path, unit_path + "shortcut_BatchNorm_moving_variance.npy"),
                         get_weights_accessor(data_path, unit_path + "shortcut_BatchNorm_gamma.npy"),
                         get_weights_accessor(data_path, unit_path + "shortcut_BatchNorm_beta.npy"),
                         0.0000100099996416f)
                     .set_name(unit_name + "shortcut/BatchNorm");

                graph << EltwiseLayer(std::move(left), std::move(right), EltwiseOperation::Add).set_name(unit_name + "add");
            }
            else if(middle_stride > 1)
            {
                SubStream left(graph);
                left << PoolingLayer(PoolingLayerInfo(PoolingType::MAX, 1, common_params.data_layout, PadStrideInfo(middle_stride, middle_stride, 0, 0), true)).set_name(unit_name + "shortcut/MaxPool");

                graph << EltwiseLayer(std::move(left), std::move(right), EltwiseOperation::Add).set_name(unit_name + "add");
            }
            else
            {
                SubStream left(graph);
                graph << EltwiseLayer(std::move(left), std::move(right), EltwiseOperation::Add).set_name(unit_name + "add");
            }

            graph << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name(unit_name + "Relu");
        }
    }

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

/** Main program for ResNetV1_50
 *
 * Model is based on:
 *      https://arxiv.org/abs/1512.03385
 *      "Deep Residual Learning for Image Recognition"
 *      Kaiming He, Xiangyu Zhang, Shaoqing Ren, Jian Sun
 *
 * Provenance: download.tensorflow.org/models/resnet_v1_50_2016_08_28.tar.gz
 *
 * @note To list all the possible arguments execute the binary appended with the --help option
 *
 * @param[in] argc Number of arguments
 * @param[in] argv Arguments
 */
int main(int argc, char **argv)
{
    return arm_compute::utils::run_example_pipeline<GraphResNet18EEExample>(argc, argv);
}
