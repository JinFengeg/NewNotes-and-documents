cmake_minimum_required(VERSION 3.5)

set(CMAKE_SYSTEM_NAME Linux) #设置目标系统名字
set(CMAKE_SYSTEM_PROCESSOR arm) #设置目标处理器架构

# 指定编译器的 sysroot 路径
set(TOOLCHAIN_DIR /usr/local/arm/gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf/bin/../lib/gcc/arm-none-linux-gnueabihf/9.2.1/include)
# 指定交叉编译器 arm-linux-gcc 和 arm-linux-g++
set(CMAKE_C_COMPILER /usr/local/arm/gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-gcc)

# 为编译器添加编译选项
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
#################################
# end
##################################
project(BLUE) #设置工程名称
include_directories(~/linux-kernel/include/linux/list_nulls.h)

add_executable(157blue blue.c)