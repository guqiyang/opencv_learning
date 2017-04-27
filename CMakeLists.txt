cmake_minimum_required(VERSION 2.8)
project( learning-1 )
SET( CMAKE_CXX_COMPILER "g++") #设定编译器

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_GSTREAMER -L /usr/local/lib -std=c++11 -O2 -g -pg -lgobject-2.0")

#设定可执行二进制文件的目录
SET( EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#并且把该目录设为连接目录
LINK_DIRECTORIES( ${PROJECT_SOURCE_DIR}/lib )


#设定头文件目录
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/include  /usr/include/gstreamer-0.10  /usr/include/glib-2.0  /usr/lib/x86_64-linux-gnu/glib-2.0/include  /usr/include/libxml2 )

#增加子文件夹，也就是进入源代码文件夹继续构建

ADD_SUBDIRECTORY( ${PROJECT_SOURCE_DIR}/src)

