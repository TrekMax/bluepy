# toolchain.cmake

# 指定交叉编译系统
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# 设置交叉编译工具链
set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

# 设置工具链路径
set(TOOLCHAIN_PATH /home/listenai/Applications/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin)
set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_PATH})

# 设置搜索路径
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
