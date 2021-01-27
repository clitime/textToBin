cmake_minimum_required(VERSION 3.17)
project(textToBin)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost COMPONENTS iostreams REQUIRED)
find_package(ZLIB REQUIRED)

add_executable(${PROJECT_NAME} main.cpp progopt.cpp progopt.hpp buildfiles.cpp buildfiles.hpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} Boost::iostreams ${ZLIB_LIBRARIES})
