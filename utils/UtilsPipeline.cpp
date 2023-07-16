/*
 * Copyright (c) 2017-2019 Arm Limited.
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
#include "UtilsPipeline.h"

//DIR
#include <dirent.h>

namespace arm_compute
{
namespace utils
{



void read_directory(const std::string& name, stringvec& v)
{
	if(arm_compute::utility::endswith(name, ".ppm") || arm_compute::utility::endswith(name, ".jpg")){
		v.push_back(name);
		return;
	}
    DIR* dirp = opendir(name.c_str());
    struct dirent * dp;
    while ((dp = readdir(dirp)) != NULL) {
        if(arm_compute::utility::endswith(dp->d_name, ".ppm") || arm_compute::utility::endswith(dp->d_name, ".jpg")){
           v.push_back(name+(dp->d_name));
           //std::cerr<<"UtilsPipeline.cpp- file name: "<<name+(dp->d_name)<<std::endl;
        }
    }
    closedir(dirp);
}




#define Frequency_Setting 0
#ifndef BENCHMARK_EXAMPLES
int run_example_pipeline(int argc, char **argv, std::unique_ptr<Example_Pipeline> example)
{
    //std::cerr << "\n"<< argv[0] << "\n\n";



    try
    {
#if Frequency_Setting
        system("echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor");
        system("echo userspace > /sys/devices/system/cpu/cpufreq/policy4/scaling_governor");
        system("echo userspace > /sys/devices/platform/ff9a0000.gpu/devfreq/ff9a0000.gpu/governor");
        int f_i=1;
#endif
        bool status = example->do_setup(argc, argv);
        std::cerr<<"setup finished\n\n";
        if(!status)
        {
            return 1;
        }
#if Frequency_Setting
        //Min
        int LFreq=408000, BFreq=408000, GFreq=200000000;
        //Max
        //int LFreq=1416000, BFreq=1800000, GFreq=800000000;

        std::string cmd="";

        /*
        //Set Little CPU Frequency
		cmd="echo " + to_string(LFreq) + " > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed";
		system(cmd.c_str());

		//Set Big CPU Frequency
		cmd="echo " + to_string(BFreq) + " > /sys/devices/system/cpu/cpufreq/policy4/scaling_setspeed";
		system(cmd.c_str());

		//Set GPU Frequency
		cmd="echo " + to_string(GFreq) + " > /sys/devices/platform/ff9a0000.gpu/devfreq/ff9a0000.gpu/userspace/set_freq";
		system(cmd.c_str());
		*/
        std::cin>>LFreq;
		std::cin>>BFreq;
		std::cin>>GFreq;

        while (BFreq && LFreq && GFreq){
        	std::cerr<<f_i++<<" Running Graph with Frequency: "<<LFreq<<','<<BFreq<<','<<GFreq<<std::endl;
			//Set Little CPU Frequency
			cmd="echo " + to_string(LFreq) + " > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed";
			system(cmd.c_str());
			//Set Big CPU Frequency
			cmd="echo " + to_string(BFreq) + " > /sys/devices/system/cpu/cpufreq/policy4/scaling_setspeed";
			system(cmd.c_str());
			//Set GPU Frequency
			cmd="echo " + to_string(GFreq) + " > /sys/devices/platform/ff9a0000.gpu/devfreq/ff9a0000.gpu/userspace/set_freq";
			system(cmd.c_str());
        	sleep(2);
        	example->do_run();
        	std::cin>>LFreq;
        	std::cin>>BFreq;
        	std::cin>>GFreq;

        }
        example->do_finish();
#else
        example->do_run();
        example->do_finish();
        example->do_teardown();
#endif

        std::cerr << "\nTest passed\n";
        return 0;
    }
#ifdef ARM_COMPUTE_CL
    catch(cl::Error &err)
    {
        std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cerr << std::endl
                  << "ERROR " << err.what() << "(" << err.err() << ")" << std::endl;
        std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
    }
#endif /* ARM_COMPUTE_CL */
    catch(std::runtime_error &err)
    {
        std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cerr << std::endl
                  << "ERROR " << err.what() << " " << (errno ? strerror(errno) : "") << std::endl;
        std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
    }

    std::cout << "\nTest FAILED\n";

    return -1;
}
#endif /* BENCHMARK_EXAMPLES */


bool Example_Pipeline::do_setup_pipeline(int argc, char **argv){
	std::cerr<<"bebin aval miam to"<<std::endl;
	return 0;
}

} // namespace utils
} // namespace arm_compute
