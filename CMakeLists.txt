cmake_minimum_required (VERSION 2.6)
set(CMAKE_VERBOSE_MAKEFILE ON)
project (arm-learn)
SET(GCC_FLAGS "-Wl --verbose -v --wackyargument")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC__FLAGS}" )
SET(LINK_FLAGS "-t ")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )
add_executable(arm-learn src/arm_learn.cpp)
target_link_libraries (arm-learn wiringPi wiringPiDev pthread /usr/local/lib/libraspicam.so ncurses)