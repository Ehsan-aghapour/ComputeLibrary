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
#include "utils/main_layer_checker.h"

#include "arm_compute/graph/Utils.h"
#include "arm_compute/graph/frontend/ILayer.h"


#include <iostream>
#include <fstream> // For file I/O
#include <string>

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



/*bool IStreamPipeline::is_next_layer(std::string name){
		static int index=0;
		static bool ended=false;
		bool starting=false;
		//concat is considered for mobilenet graph which in layers there is concat after ending layer but in tasks this is not exist
		//So we ingonre it here in layers
		if(ended and name!="concat"){
			index++;
			std::string indent=(index%2)?"":"\t\t\t";
			std::cerr <<indent<< index<<" layer: "<<name << std::endl;
			ended=false;
			starting=true;
		}
		else{
			std::string indent=(index%2)?"":"\t\t\t";
			std::cerr <<indent<<index<< " skipping layer: "<<name << std::endl;
			//return false;
		}
		if (check_ending(graph_name, name)){
			ended=true;
		}
		return starting;

    }*/

void addNameToFile(const std::string& filename, const std::string& name) {
    // Open the file in append mode
    std::ofstream file(filename, std::ios::app);

    if (file.is_open()) {
        // Write the name to a new line in the file
        file << name << std::endl;
        file.close(); // Close the file after writing
    } else {
        std::cerr << "Error opening file: " << filename << std::endl;
    }
}


bool IStreamPipeline::is_next_layer(std::string name){
		static int index=-1;
		static bool header=false;
		std::string file_name="Layers.log";
		if (!header){
			parseConfigFile("./Layers.conf");
			std::cerr<<"\n\n\n*******************************************************************************************\n";
			std::cerr<<"                                   Layer Names                                                   \n";
			std::cerr<<"*******************************************************************************************\n\n";
			std::cerr<<"["<<graph_name<<"]"<<std::endl;
			//addNameToFile("Layers.log", graph_name);
			header=true;
		}
		bool starting=check_starting(graph_name,name);
		//std::cerr<<"check is next layer "<<starting<<std::endl;
		//addNameToFile("Layers.log", name);
		if(starting){
			index++;
			std::cerr<<"\n----------------------------------------------------------------\n";
			std::cerr<<"Layer "<<index;
			std::cerr<<"\n----------------------------------------------------------------\n";
			std::string indent=(index%2)?"":"\t\t\t";
			std::cerr <<indent<<name << std::endl;
			return true;
		}
		else{
			std::string indent=(index%2)?"":"\t\t\t";
			std::cerr<<indent<<name << std::endl;
			//std::cerr<<"dd\n";
			return false;
		}

    }
bool IStreamPipeline::is_end_layer(std::string name){
		static int index=0;
		if (check_ending(graph_name, name)){
			std::string indent=(index%2)?"":"\t\t\t";
			std::cerr <<indent<< index<<" layer: "<<name << std::endl;
			index++;
			return true;
		}
		else{
			std::string indent=(index%2)?"":"\t\t\t";
			std::cerr <<indent<<index<< " skipping layer: "<<name << std::endl;
			return false;
		}
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
