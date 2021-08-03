include(FetchContent)

FetchContent_Declare(
  libdivide
  GIT_REPOSITORY https://github.com/ridiculousfish/libdivide
  GIT_TAG v4.0.0
  )

FetchContent_MakeAvailable(libdivide)
