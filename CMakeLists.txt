cmake_minimum_required(VERSION 3.17)
project(textToBin)

set(CMAKE_CXX_STANDARD 17)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost REQUIRED COMPONENTS
    iostreams
    )


add_executable(${PROJECT_NAME} main.cpp progopt.cpp progopt.hpp buildfiles.cpp buildfiles.hpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} Boost::iostreams)
