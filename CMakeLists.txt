
cmake_minimum_required(VERSION 3.6)

project(GDExample)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()

add_subdirectory(src)

