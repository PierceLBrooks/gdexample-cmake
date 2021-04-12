
cmake_minimum_required(VERSION 3.6)

project(GDExample)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Android)
	set(CMAKE_TOOLCHAIN_FILE "$ENV{ANDROID_NDK_ROOT}/build/cmake/android.toolchain.cmake" CACHE FILEPATH "Android NDK's CMake toolchain file")
	set(ANDROID_PLATFORM android-28 CACHE STRING "Android target platform API level")
	set(ANDROID_ABI armeabi-v7a CACHE STRING "Android target ABI architecture")
	set(ANDROID_TOOLCHAIN clang)
	set(ANDROID_STL c++_shared)
endif()

add_subdirectory(src)

