cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -static-libsan -fsanitize=address,leak,undefined")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Enable march=native if available
check_cxx_compiler_flag(-march=native HAS_MARCH_NATIVE)
if (HAS_MARCH_NATIVE)
  target_compile_options(${PROJECT_NAME} INTERFACE -march=native)
endif()

add_executable(ha_benchmarks benchmarks.cpp)
target_link_libraries(ha_benchmarks PRIVATE hashing ${GOOGLEBENCHMARK_LIBRARY})
