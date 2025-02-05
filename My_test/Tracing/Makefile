# Android NDK Path (adjust this to your actual NDK path)
NDK_PATH := /home/ehsan/UvA/ARMCL/android-ndk-r21e-linux-x86_64/android-ndk-r21e
TOOLCHAIN := $(NDK_PATH)/toolchains/llvm/prebuilt/linux-x86_64
SYSROOT := $(TOOLCHAIN)/sysroot

# Android Compiler and Flags
ARCH := aarch64-linux-android
# Set your target API level
API_LEVEL := 23
CC := $(TOOLCHAIN)/bin/$(ARCH)$(API_LEVEL)-clang++

# C++ Flags (you can customize these based on your project needs)
CXXFLAGS := --sysroot=$(SYSROOT) -I$(SYSROOT)/usr/include -fPIE -fPIC -Wall -O2

# Linker Flags
LDFLAGS := -L$(SYSROOT)/usr/lib -llog -landroid -pie

# Output Target
TARGET := trace_test

# Source Files
SRC := trace_test.cpp

# Build Rule
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)


push:
	adb push $(TARGET) /data/local/tmp/
	adb shell chmod +x /data/local/tmp/$(TARGET)

# Clean Rule
clean:
	rm -f $(TARGET)