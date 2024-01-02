cmake_minimum_required(VERSION 3.17)
project(GameServer)

set(CMAKE_CXX_STANDARD 17)
set(CONAN_DISABLE_CHECK_COMPILER true)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

if (UNIX)
    add_compile_options(-pthread -fPIC)
endif ()

#find_package(OpenCL REQUIRED)
#message(${OpenCL_LIBRARY})
#message(${OpenCL_LIBRARY}/OpenCL.lib)
#message(${Vulkan_LIBRARY})
#message(${Vulkan_LIBRARY}/Vulkan-1.lib)
#link_directories(${OpenCL_LIBRARY})

#message(${PROJECT_SOURCE_DIR})


#set(DynamicCompute_Lib "C:\\Users\\Jon\\Source\\dynamics-io\\DynamicCompute\\build\\lib")
#set(DynamicCompute_Lib "C:\\Users\\Jon\\Source\\dynamics-io\\Dynamics.io-Testbench\\CPP_Bench\\DynamicCompute\\build\\lib")
#set(OCL "C:\\Users\\Jon\\Source\\dynamics-io\\DynamicCompute\\lib\\OpenCL_SDK\\win32")
#set(VK "C:\\Users\\Jon\\Source\\dynamics-io\\DynamicCompute\\lib\\vulkan_1.3.239.0\\Lib")
#set(DynamicCompute_include "C:\\Users\\Jon\\Source\\dynamics-io\\DynamicCompute\\source\\public")
#link_directories(${DynamicCompute_Lib})
#link_directories(${OCL})
#link_directories(${VK})
#include_directories(${DynamicCompute_include})


add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/Zc:preprocessor>)

add_subdirectory(source)



