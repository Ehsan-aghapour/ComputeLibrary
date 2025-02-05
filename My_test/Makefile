

# Usage:
# make BOARD=Khadas
# make BOARD=RockPi

# Define paths for cross compilers
ROCKPI_CROSS_COMPILER_PATH := /home/ehsan/UvA/DRLPM/Master_Backup/NUS/android-ndk-r15c/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-
KHADAS_CROSS_COMPILER_PATH := /home/ehsan/UvA/DRLPM/Master_Backup/NUS/android-ndk-r15c/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-

CROSS_COMPILER_PATH := /home/ehsan/UvA/ARMCL/android-ndk-r21e-linux-x86_64/android-ndk-r21e/toolchains/llvm/prebuilt/linux-x86_64/bin/

# Default values (can be overridden by command line)
BOARD ?= Khadas

ifeq ($(BOARD), RockPi)
    ARCH := arm64
    CROSS_COMPILE := $(CROSS_COMPILER_PATH)/aarch64-linux-android23-clang++
else ifeq ($(BOARD), Khadas)
    ARCH := arm
    CROSS_COMPILE := $(CROSS_COMPILER_PATH)/armv7a-linux-androideabi23-clang++
else
    $(error "Unsupported BOARD specified. Please use 'make BOARD=RockPi' or 'make BOARD=Khadas'")
endif

MODULE_NAME := gpio
obj-m := $(MODULE_NAME).o

# Pass the BOARD definition to the C compiler
KBUILD_CFLAGS += -DBOARD=$(BOARD)

# Target to compile the gpio_test.cpp file
default:
	$(CROSS_COMPILE) gpio_test.cpp -o gpio_test

run:
	adb push gpio_test /system
	adb shell chmod +x /system/gpio_test
	adb shell /system/gpio_test

setup:
	adb shell "echo interactive > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor"
	adb shell "echo interactive > /sys/devices/system/cpu/cpufreq/policy4/scaling_governor"

	adb push $(MODULE_NAME).ko /data/local/tmp/
	adb shell "dmesg -C"
	# Check if the module is already inserted
	@if adb shell lsmod | grep -wq $(MODULE_NAME); then \
		echo "Module is already inserted, removing and reinserting..."; \
		adb shell rmmod $(MODULE_NAME); \
	fi
	adb shell insmod /data/local/tmp/$(MODULE_NAME).ko
	adb shell "lsmod"

	adb shell "echo pandoon > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor"
	adb shell "echo pandoon > /sys/devices/system/cpu/cpufreq/policy4/scaling_governor"
	adb shell "echo pandoon > /sys/class/devfreq/ff9a0000.gpu/governor"
	adb shell "dmesg -w"

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
