#ifndef DVFS_H
#define DVFS_H


#define debuglevel 0

#define kernelspace 0
#define userspace 1
#define mode kernelspace

#define rockpi 0
#define khadas 1
#define board rockpi

#if board == khadas
    //const std::string CPU_PATH = "/sys/devices/system/cpu/cpufreq/";
    // Frequency tables for Khadas board
    inline const std::vector<int> LittleFrequencyTable = {500000, 667000, 1000000, 1200000, 1398000, 1512000, 1608000, 1704000, 1800000};
    inline const std::vector<int> BigFrequencyTable = {500000, 667000, 1000000, 1200000, 1398000, 1512000, 1608000, 1704000, 1800000, 1908000, 2016000, 2100000, 2208000};
#elif board == rockpi
	inline const std::vector<int> LittleFrequencyTable = {408000, 600000, 816000, 1008000, 1200000, 1416000};
    inline const std::vector<int> BigFrequencyTable = {408000, 600000, 816000, 1008000, 1200000, 1416000, 1608000, 1800000};
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<sys/ioctl.h>
#include<iostream>
#include <fstream>
#include <inttypes.h>


#define Pandoon_MAGIC (0xAA)
#define next_state _IO(Pandoon_MAGIC,'c')
#define capture_freqs _IOR(Pandoon_MAGIC,'n',struct f )
#define Apply_freqs _IOW(Pandoon_MAGIC,'a', union ptr)

//extern int fd;

//static std::string governor_module="your_governor";

/*Ehsan: data frequency strucutre for kernel IOCTL API *************************/
struct f{
        uint64_t gf;
        uint32_t f1;
        uint32_t f2;
	uint64_t  capturing=0;
};


//Ehsan pointer wrapper for IOCTL
union ptr{
	int8_t* a;
	uint64_t padding;
};


class DVFS{
public:
	DVFS(){
	}
	uint64_t diff_time(timespec start, timespec stop){
	        uint64_t out;
	        out=((uint64_t)stop.tv_sec*(uint64_t)1.0e9+(uint64_t)stop.tv_nsec)-((uint64_t)start.tv_sec*(uint64_t)1.0e9+(uint64_t)start.tv_nsec);
	        return out;
	}

	int open_pandoon(){
	        //int fddd=-1;
	        fd = open("/dev/pandoon_device", O_RDWR);
	        std::cerr<<"Pandoon opened at "<<fd<<std::endl;
	        if (fd<0){
	                printf("DRPM_Ehsan: Pandoon Not Opened. ERROR CODE:%d, ERROR MEANING:%s\n",errno,strerror(errno));
	                close(fd);
	                return -1;
	        }
	        return fd;
	}


	int get_freq(){
	        static struct f freqs;

	        int jj=ioctl(fd,capture_freqs, &freqs);
	        //printf("second capture_freqs:%d\n",jj);
	        if(jj<0){
	                std::cerr<<"error \n";
	        }

	        printf("Freqs Captured, FA53:%u, FA73:%u, F_GPU:%" PRIu64", Capturing:%" PRIu64,freqs.f1,freqs.f2,freqs.gf,freqs.capturing);
	        printf("\n");
	        //printf("freqs:%" PRIu64,freqs.capturing);
	        //printf("\n");
	        return 0;
	}


	int commit_freq(int Little, int big, int GPU, std::string name=""){

			int duration=0;
	        static int l,b,g=0;
	        if (Little>-1){
	                l=Little;
	        }
	        if (big>-1){
	                b=big;
	        }
	        if(GPU>-1){
	                g=GPU;
	        }
			timespec t_start,t_end;
#if mode == kernelspace
	        union ptr ptr_freqs;

#if debuglevel == 1
			std::cerr<<"task: "<<name<<std::endl;
	        std::cout<<fd<<" req freqs, little:"<<Little<<", big:"<<big<<", GPU:"<<GPU<<std::endl;
	        std::cout<<fd<<" apply freqs, little:"<<l<<", big:"<<b<<", GPU:"<<g<<std::endl;
#endif
	        clock_gettime(CLOCK_MONOTONIC, &t_start);
	        g=4-g;
	        int8_t dfreqs[3];
	        dfreqs[0]=g;
	        dfreqs[1]=b;
	        dfreqs[2]=l;
	        ptr_freqs.padding=0;
	        ptr_freqs.a=dfreqs;
	        int jj=ioctl(fd, Apply_freqs,ptr_freqs.a);
	        if(jj<0){
	                printf("DRPM_Ehsan: Apply freqs ioctl error, ret=%d, error:%d, meaning: %s\n",jj,errno,strerror(errno));
	                printf("\n");
	                return -1;
	        }
	        clock_gettime(CLOCK_MONOTONIC, &t_end);
	        unsigned long int ioctltime=diff_time(t_start,t_end);
			duration=ioctltime/1000000;
#endif

#if mode == userspace

#if board == khadas
			std::string CPU_path="/sys/devices/system/cpu/cpufreq/";
			std::string GPU_path="/sys/class/mpgpu/";
			std::string Command="";
			int little_freq = LittleFrequencyTable[l];
    		int big_freq = BigFrequencyTable[b];

#if debuglevel == 1
			std::cerr<<"task: "<<name<<std::endl;
	        std::cout<<fd<<" req freqs, little:"<<Little<<", big:"<<big<<", GPU:"<<GPU<<std::endl;
	        std::cout<<fd<<" apply freqs, little_freq:"<<l<<", big_freq:"<<b<<std::endl;//", GPU:"<<g<<std::endl;
#endif
			//Google AOSP
			//Command="echo " + std::string(freq) + " > " + CPU_path + "policy0/scaling_setspeed";
			//Khadas AOSP
			//if you want to do it manually just return
			return 0;
			clock_gettime(CLOCK_MONOTONIC, &t_start);
			Command="echo " + std::to_string(little_freq) + " > " + CPU_path + "policy0/scaling_min_freq";
	        system(Command.c_str());
			Command="echo " + std::to_string(little_freq) + " > " + CPU_path + "policy0/scaling_max_freq";
	        system(Command.c_str());
			Command="echo " + std::to_string(big_freq) + " > " + CPU_path + "policy2/scaling_min_freq";
			system(Command.c_str());
			Command="echo " + std::to_string(big_freq) + " > " + CPU_path + "policy2/scaling_max_freq";
	        system(Command.c_str());
			//Khadas GPU: echo 4 > /sys/class/mpgpu/max_freq
			Command="echo " + std::to_string(g) + " > " + GPU_path + "min_freq";
			system(Command.c_str());
			Command="echo " + std::to_string(g) + " > " + GPU_path + "max_freq";
			system(Command.c_str());


			clock_gettime(CLOCK_MONOTONIC, &t_end);
			unsigned long int userspacetime=diff_time(t_start,t_end);
			duration=userspacetime/1000000;
#if debuglevel == 1 or true
			std::cerr<<"DVFS duration: "<<duration<<" ms"<<std::endl;
#endif

#endif


#endif
	        return 0;
	}

	int commit_freq(std::string s){
	        int Little,big,GPU;
	        int fff=std::stoi(s);
	        /*GPU=5-int(fff%5);
	        big=int(fff/5)%8;
	        Little=int(fff/40);*/
	        GPU=int(fff%10);
	        big=int(fff/10)%10;
	        Little=int(fff/100);
	        return commit_freq(Little, big, GPU);

	}

	int init_hikey(){
	        std::string CPU_path="/sys/devices/system/cpu/cpufreq/";
			std::string GPU_path="/sys/class/mpgpu/";
			//For Google AOSP (Khadas AOSP has not GPU governor)
			//std::string GPU_path="/sys/class/devfreq/ffe40000.gpu/";
	        //std::string GPU_path="/sys/devices/platform/e82c0000.mali/devfreq/e82c0000.mali";
			//For KHADAS AOSP:
			//echo 4 > /sys/class/mpgpu/max_freq
			//echo 4 > /sys/class/mpgpu/min_freq
			std::string Command="";
#if mode == kernelspace
	        std::string IOCTL_path="/dev/pandoon_device";
	        Command="chmod 666 " + IOCTL_path;
			system(Command.c_str());
	        Command="echo pandoon > " + CPU_path + "policy2/scaling_governor";
	        system(Command.c_str());
	        Command="echo pandoon > " + CPU_path + "policy0/scaling_governor";
	        system(Command.c_str());
	        Command="echo pandoon > " + GPU_path + "/governor";
			system(Command.c_str());
	        //Command= "echo 1 > " + IOCTL_path;
			//system(Command.c_str());
#endif
#if mode == userspace
			//Google AOSP:
			//Command="echo userspace > " + CPU_path + "policy2/scaling_governor";
			//Khadas AOSP
			Command="echo performance > " + CPU_path + "policy2/scaling_governor";
	        system(Command.c_str());
	        Command="echo performance > " + CPU_path + "policy0/scaling_governor";
	        system(Command.c_str());
			//Khadas GPU: echo 4 > /sys/class/mpgpu/min_freq
			Command="echo " + std::to_string(max_g) + " > " + GPU_path + "min_freq";
			system(Command.c_str());
	        //Command="echo pandoon > " + GPU_path + "/governor";
			//system(Command.c_str());
#endif
			Command="cat " + CPU_path + "policy0/scaling_governor";
			//printf("Little CPU governor: ");
	        system(Command.c_str());
	        Command="cat " + CPU_path + "policy2/scaling_governor";
			//printf("Big CPU governor: ");
	        system(Command.c_str());
			Command="cat " + GPU_path + "min_freq";
			//printf("Min GPU Frequency set to: ");
	        system(Command.c_str());
	        //Command="cat " + GPU_path + "/governor";
	        //system(Command.c_str());

	        return 0;
	}

	int init_rockpi(){
		//std::string Command="insmod " + governor_module;
		//system(Command.c_str());
	        std::string CPU_path="/sys/devices/system/cpu/cpufreq/";
	        std::string GPU_path="/sys/class/devfreq/ff9a0000.gpu/";
	        std::string IOCTL_path="/dev/pandoon_device";
	        std::string Command="chmod 666 " + IOCTL_path;
	        Command="echo pandoon > " + CPU_path + "policy4/scaling_governor";
	        system(Command.c_str());
	        Command="echo pandoon > " + CPU_path + "policy0/scaling_governor";
	        system(Command.c_str());
	        Command="echo pandoon > " + GPU_path + "/governor";
		system(Command.c_str());

	        Command="cat " + CPU_path + "policy0/scaling_governor";
	        system(Command.c_str());
	        Command="cat " + CPU_path + "policy4/scaling_governor";
	        system(Command.c_str());
	        Command="cat " + GPU_path + "/governor";
	        system(Command.c_str());
	        return 0;
	}

	void read_freqs(){
	        std::string CPU_path="/sys/devices/system/cpu/cpufreq/";
	        std::string GPU_path="/sys/class/devfreq/ff9a0000.gpu/";
	        std::string t="scaling_";
	        t="cpuinfo_";
	        //You can also open it and read/write it (using open and read c++ commands)
	        //Big CPU:
	        std::string Command="cat " + CPU_path + "policy0/" + t+"cur_freq";
	        system(Command.c_str());
	        //Little CPU
	        Command="cat " + CPU_path + "policy4/" + t+"cur_freq";
	        system(Command.c_str());
	        //GPU:
	        Command="cat " + GPU_path + "/cur_freq";
	        system(Command.c_str());

	}
	void init(){
#if board == rockpi
		//init_rockpi();
#endif
#if board ==khadas
		init_hikey();
#endif
		open_pandoon();
	}
	void finish(){
		close(fd);
	}

	int m(){
	        init_rockpi();
	        int fdd=open_pandoon();
	        std::string s;
	        /*while(true){
	                std::cout<<"Please enter freqs: ";
	                std::cin>>s;
	                set_freq(fd,s);
	                usleep(2500);
	                get_freq(fd);
	                read_freqs();
	        }*/
	        while(true){
	                int l,b,g;
	                std::cout<<"Please enter freqs: ";
	                std::cin>>l;
	                std::cin>>b;
	                std::cin>>g;
	                commit_freq(l,b,g);
	                usleep(4500);
	                get_freq();
	                read_freqs();
	        }
	        return 0;
	}
	/*static DVFS& get(){
		static DVFS instance;
		return instance;
	}*/

	int fd;

	static int get_max_l(){
		return max_l;
	}

	static int get_max_b(){
		return max_b;
	}

	static int get_max_g(){
		return max_g;
	}

#if board == rockpi
	inline static int max_l=5;
	inline static int max_b=7;
	inline static int max_g=4;
#elif board == khadas
	inline static int max_l=LittleFrequencyTable.size()-1;
	inline static int max_b=BigFrequencyTable.size()-1;
	inline static int max_g=4;
#endif
};

//int DVFS::fd=-1;

#endif // DVFS_H

