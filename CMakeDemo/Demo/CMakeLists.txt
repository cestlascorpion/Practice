cmake_minimum_required(VERSION 3.16)

project(Demo C CXX)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wconversion -Wsizeof-pointer-memaccess \
                     -Wfloat-equal -Wconversion-null -Woverflow -Wshadow \
                     -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -pthread -g -O3")

add_executable(Demo 
        main.cpp)
