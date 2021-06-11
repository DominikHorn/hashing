include(ExternalProject)
find_package(Git REQUIRED)

# library name
set(LIBDIVIDE_LIBRARY libdivide)

ExternalProject_Add(
        ${LIBDIVIDE_LIBRARY}_src
        PREFIX external/${LIBDIVIDE_LIBRARY}
        GIT_REPOSITORY "https://github.com/ridiculousfish/libdivide"
        GIT_TAG v4.0.0
        TIMEOUT 10
        CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/external/${LIBDIVIDE_LIBRARY}
)

# path to installed artifacts
ExternalProject_Get_Property(${LIBDIVIDE_LIBRARY}_src install_dir)

# build library from external project
add_library(${LIBDIVIDE_LIBRARY} INTERFACE)
add_dependencies(${LIBDIVIDE_LIBRARY} ${LIBDIVIDE_LIBRARY}_src)
target_include_directories(${LIBDIVIDE_LIBRARY} INTERFACE
        $<BUILD_INTERFACE:${install_dir}/include>
        )

message(STATUS "[LIBDIVIDE] settings")
message(STATUS "    LIBDIVIDE_LIBRARY = ${LIBDIVIDE_LIBRARY}")
