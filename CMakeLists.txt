cmake_minimum_required(VERSION 3.0)
project(cmdly VERSION 1.0)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (BUILD_TESTING)
    enable_testing()
endif()

add_subdirectory(libcmdly)

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
