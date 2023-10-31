/*
 * Copyright (c) 2018-2019 Arm Limited.
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
//Ehsan
#include "arm_compute/graph/frontend/IStreamPipeline.h"

#include "arm_compute/graph/Utils.h"
#include "arm_compute/graph/frontend/ILayer.h"

namespace arm_compute
{
namespace graph
{
namespace frontend
{






void IStreamPipeline::add_layer(ILayer &layer)
{
    auto nid   = layer.create_layer(*this);
    std::cerr<<"(IStreamPipeline) Adding layer "<<layer.name()<<" "<<_tail_node<<"->"<<nid<<std::endl;
    _tail_node=nid;
}

#define Operation 1
#define Conv 2
//Select Granularity Here:
#define Granularity Conv



#if Granularity == Operation
bool IStreamPipeline::is_next_layer(std::string name){




		//Method 1:
		std::string formatPattern = "^(?!.*_g\\d*)(?!.*relu)(?!$)";



		std::regex pattern(formatPattern, std::regex_constants::icase);
		if(regex_search(name,pattern)){
			std::cerr << "layer: "<<name << std::endl;
			return true;
		}
		else{
			std::cerr << "skipping layer: "<<name << std::endl;
			return false;
		}




		//Method2:
		if (name==""){
					std::cerr << "Skipping layer: "<<name << std::endl;
					return false;
		}
		// patterns that are skipped:
		std::vector<std::string> formats={
				".*_g\\d*",	//all names with _g then a digit like _g2 (for group convs)
				".*relu.*",	//all names with relu because in python models relu layers all not separate layers
		};
		std::vector<std::regex> patterns;
		for(auto format:formats){
			patterns.push_back(std::regex(format, std::regex_constants::icase));
		}
		bool contains = false;
		for(auto pattern:patterns){
			contains=contains || (regex_search(name,pattern));
		}
		if (contains){
    		std::cerr << "Skipping layer: "<<name << std::endl;
    		return false;
		}
		else{
			std::cerr << "layer: "<<name << std::endl;
			return true;
		}


		/*
		 * if (name==""){
					std::cerr << "Skipping layer: "<<name << std::endl;
					return false;
		}
    	std::string const formatPattern = ".*(_g\\d*|relu).*";
    	////std::string formatPattern = ".*_g\\d*|.*relu.*";
    	std::regex pattern(formatPattern, std::regex_constants::icase);
    	if (regex_search(name, pattern)) {
    		std::cerr << "Skipping layer: "<<name << std::endl;
    		return false;
    	}
    	else{
    		return true;
    	}*/
    }
#endif


#if Granularity == Conv
bool IStreamPipeline::is_next_layer(std::string name){
		static int index=0;
		/* patterns that are skipped:
		 * .*_g\\d* all names with _g then a digit like _g2 (for group convs)
		 * .*relu.* all names with relu because in python models relu layers all not separate layers
		 * .*BatchNorm.* names that have batchnorm
		 * .*Linear.* for the last operations in yolov3 (maybe this filter should be added to operation level granularity also [I think it is some kind of activation])
		 */

    	/*
		//********************** ----> This does not work:
		//std::string formatPattern = ".*_g\\d*|.*relu.*|.*linear.*";
		//This works:
		std::string const formatPattern = ".*(_g\\d*|relu|linear).*";
		//However when I added the batchnorm, the later also not works!:
		//std::string const formatPattern = ".*(_g\\d*|relu|linear|batchnorm).*";
		std::regex pattern(formatPattern, std::regex_constants::icase);
		if (regex_search(name, pattern)) {
			std::cerr << "Skipping layer: "<<name << std::endl;
			return false;
		}
		else{
			return true;
		}*/




		//Method 1:
		const std::string formatPattern = "^(?!.*_g\\d*)(?!.*relu)(?!.*batchnorm)(?!.*linear)(.*conv.*|.*fc.*)";
		//If you do not want to skip input and output layers :
		//std::string formatPattern = "^(?!.*_g\\d*)(?!.*relu)(?!.*batchnorm)(?!.*linear)(.*conv.*|.*fc.*|$)";



		std::regex pattern(formatPattern, std::regex_constants::icase);
		if(regex_search(name,pattern)){
			index++;
			std::cerr << index<<" layer: "<<name << std::endl;
			return true;
		}
		else{
			std::cerr <<index<< " skipping layer: "<<name << std::endl;
			return false;
		}

		//Method 2
		if (name==""){
					std::cerr << "Skipping layer: "<<name << std::endl;
					return false;
				}
		// patterns that are skipped:
		std::vector<std::string> formats={
				".*_g\\d*",	//all names with _g then a digit like _g2 (for group convs)
				".*relu.*",	//all names with relu because in python models relu layers all not separate layers
				".*linear.*",	//some activation layers in yolo are linear
				".*batchnorm.*"	//names that have batchnorm
		};
		std::vector<std::regex> patterns;
		for(auto format:formats){
			patterns.push_back(std::regex(format, std::regex_constants::icase));
		}
		bool skip = false;
		for(auto pattern:patterns){
			skip=skip || (regex_search(name,pattern));
		}
		if (skip){
    		std::cerr << "Skipping layer: "<<name << std::endl;
    		return false;
		}



		// patterns that are accepted:
		std::vector<std::string> accepted_formats={
				".*conv.*",	//all names with conv (conv/leaky and conv/linear layers already skipped
				".*fc.*",	//all fully connected layers
		};
		std::vector<std::regex> accepted_patterns;
		for(auto format:accepted_formats){
			accepted_patterns.push_back(std::regex(format, std::regex_constants::icase));
		}
		bool accept = false;
		for(auto pattern:accepted_patterns){
			accept=accept || (regex_search(name,pattern));
		}
		if (accept){
			std::cerr << "layer: "<<name << std::endl;
			return true;
		}
		else{
			std::cerr << "Skipping layer: "<<name << std::endl;
			return false;
		}


		//Method 3:
		if (name==""){
					std::cerr << "Skipping layer: "<<name << std::endl;
					return false;
				}
    	/* pattern that are considered as start of a layer
    	 * .*conv.* all names with conv
    	 * .*fc.* all names with fc
    	 * ^$ all names wit empty string (for input layer) //for now we did not count it separately becaue make wrong output (test with alexnet ./Run_CO-UP model=Alex --order=BBGGBBBBL push=1 compile=1)
    	 */
    	//std::string formatPattern_conv = ".*conv.*|.*fc.*|^$";
    	/*std::string formatPattern_conv = ".*conv.*|.*fc.*";
    	std::regex pattern_conv(formatPattern_conv, std::regex_constants::icase);

    	if (regex_search(name, pattern)) {
    		std::cerr << "Skipping layer: "<<name << std::endl;
    		return false;
    	}
    	else{
    		if (regex_search(name, pattern_conv)){
    			std::cerr<<"layer name: "<<name<<std::endl;
    			return true;
    		}
    		std::cerr << "2-Skipping layer: "<<name << std::endl;
    		return false;
    	}*/

    	/*
    	 * or you can just write this:
    	std::string formatPattern = "^(?!.*_g\\d*)(?!.*relu).*conv.*";
		std::regex pattern(formatPattern, std::regex_constants::icase);
    	 */

    }
#endif
/*
const Graph &IStreamPipeline::graph() const
{
	//std::cerr<<"calling graph const\n";
	return this->graph();
	//return _g;
}

Graph &IStreamPipeline::graph()
{
    return this->graph();
	//return _g;
}

IStreamPipeline & IStreamPipeline::operator<<(ILayer &layer)
{

	std::cerr<<"(IStreamPipeline) "<<current_layer++<<" Layer Name:"<<layer.name()<<std::endl;
    add_layer(layer);
    std::cerr<<"*******************************\n";
    return *this;
}
IStreamPipeline & IStreamPipeline::operator<<(ILayer &&layer)
{

	std::cerr<<"(IStreamPipeline) "<<current_layer++<<" Layer Name:"<<layer.name()<<std::endl;
    add_layer(layer);
    std::cerr<<"*******************************\n";
    return *this;
}



NodeID IStreamPipeline::tail_node()
{
	//std::cerr<<"(IStreamPipeline) tail_node()- Tail_node: "<<Tail_node[target_graph]<<std::endl;
	//return Tail_node[target_graph];
	std::cerr<<"(IStreamPipeline) tail_node()- Tail_node: "<<_tail_node<<std::endl;
	return _tail_node;

}
*/


} // namespace frontend
} // namespace graph
} // namespace arm_compute
