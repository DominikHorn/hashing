cmake_minimum_required(VERSION 3.19)
project("hashing" VERSION 1.0
        DESCRIPTION "A header only cmake/c++ library that exposes various state-of-the-art non-cryptographic hash functions and reduction algorithms."
        HOMEPAGE_URL "https://github.com/andreaskipf/hashing")

# Declare library & directories to include. See
# http://mariobadr.com/creating-a-header-only-library-with-cmake.html for more
# info/install instructions
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# Make IDE friendly
target_sources(${PROJECT_NAME} INTERFACE hashing.hpp include/)

# Require c++20 for compilation
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

# Add thirdparty libraries 
include(${CMAKE_SOURCE_DIR}/thirdparty/googlebenchmark.cmake)

add_subdirectory(src)