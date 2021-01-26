cmake_minimum_required(VERSION 3.17)
project(textToBin)

set(CMAKE_CXX_STANDARD 17)

add_executable(textToBin main.cpp progopt.cpp progopt.hpp buildfiles.cpp buildfiles.hpp)
