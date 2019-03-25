cmake_minimum_required(VERSION 3.10)
project(gfxe_test VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

add_subdirectory(${CMAKE_SOURCE_DIR}/src/lib)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/test)