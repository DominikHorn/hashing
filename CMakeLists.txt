cmake_minimum_required(VERSION 3.19)
project("hashing" VERSION 1.0
        DESCRIPTION "A header only cmake/c++ library that exposes various state-of-the-art non-cryptographic hash functions and reduction algorithms."
        HOMEPAGE_URL "https://github.com/DominikHorn/hashing")


# Declare library & directories to include. See
# http://mariobadr.com/creating-a-header-only-library-with-cmake.html for more
# info/install instructions
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# ==== Dependencies ====
include(FetchContent)
FetchContent_Declare(
  libdivide
  GIT_REPOSITORY https://github.com/ridiculousfish/libdivide.git
  GIT_TAG b322221677351ebb11f0a42fe9a9a2794da5bfe5
  )

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(BUILD_FUZZERS OFF)
set(BUILD_TESTS OFF)
FetchContent_MakeAvailable(libdivide)

# Link to dependencies
target_link_libraries(${PROJECT_NAME} INTERFACE libdivide)

# Make IDE friendly
target_sources(${PROJECT_NAME} INTERFACE hashing.hpp include/)

# Benchmark code
get_directory_property(hasParent PARENT_DIRECTORY)
if (NOT hasParent)
  include(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/googlebenchmark.cmake)

  add_subdirectory(src)
endif()
