cmake_minimum_required(VERSION 3.0)
project(mprpc)

#设置项目可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 生成debug版本， 可以进行gdb调试
set(CMAKE_BUILD_TYPE "Debug")

#设置项目库文件输出的路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#设置项目编译头文件搜索路径  -I g++
include_directories(${PROJECT_SOURCE_DIR}/src/include)

#添加user.pb.cc .h文件
include_directories(${PROJECT_SOURCE_DIR}/example)  


#设置项目库文件搜索路径 -L g++
link_directories(${PROJECT_SOURCE_DIR}/lib)


# 框架所有代码
add_subdirectory(src)

# 使用者 和 消费者代码   caller 服务消费者    callee 服务提供者   业务代码
add_subdirectory(example)