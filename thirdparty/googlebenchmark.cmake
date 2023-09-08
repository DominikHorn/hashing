include(ExternalProject)
find_package(Git REQUIRED)

# library name
set(GOOGLEBENCHMARK_LIBRARY ha_googlebenchmark)

ExternalProject_Add(
        ${GOOGLEBENCHMARK_LIBRARY}_src
        PREFIX external/${GOOGLEBENCHMARK_LIBRARY}
        GIT_REPOSITORY "https://github.com/google/benchmark.git"
        GIT_TAG v1.8.3
        TIMEOUT 10
        CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/external/${GOOGLEBENCHMARK_LIBRARY}
        -DCMAKE_INSTALL_LIBDIR=${CMAKE_BINARY_DIR}/external/${GOOGLEBENCHMARK_LIBRARY}/lib
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DBENCHMARK_ENABLE_GTEST_TESTS=0
        UPDATE_COMMAND ""
        BUILD_BYPRODUCTS <INSTALL_DIR>/lib/libbenchmark.a
)

# path to installed artifacts
ExternalProject_Get_Property(${GOOGLEBENCHMARK_LIBRARY}_src install_dir)
set(GOOGLEBENCHMARK_INCLUDE_DIR ${install_dir}/include)
set(GOOGLEBENCHMARK_LIBRARY_PATH ${install_dir}/lib/libbenchmark.a)

# build library from external project
file(MAKE_DIRECTORY ${GOOGLEBENCHMARK_INCLUDE_DIR})
add_library(${GOOGLEBENCHMARK_LIBRARY} STATIC IMPORTED)
set_property(TARGET ${GOOGLEBENCHMARK_LIBRARY} PROPERTY IMPORTED_LOCATION ${GOOGLEBENCHMARK_LIBRARY_PATH})
set_property(TARGET ${GOOGLEBENCHMARK_LIBRARY} APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${GOOGLEBENCHMARK_INCLUDE_DIR})
add_dependencies(${GOOGLEBENCHMARK_LIBRARY} ${GOOGLEBENCHMARK_LIBRARY}_src)

message(STATUS "[GOOGLEBENCHMARK] settings")
message(STATUS "    GOOGLEBENCHMARK_LIBRARY = ${GOOGLEBENCHMARK_LIBRARY}")
message(STATUS "    GOOGLEBENCHMARK_INCLUDE_DIR = ${GOOGLEBENCHMARK_INCLUDE_DIR}")
message(STATUS "    GOOGLEBENCHMARK_LIBRARY_PATH = ${GOOGLEBENCHMARK_LIBRARY_PATH}")
