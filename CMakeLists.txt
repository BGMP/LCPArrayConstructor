cmake_minimum_required(VERSION 3.27)
project(LCPArrayConstructor)

set(CMAKE_CXX_STANDARD 17)

add_executable(LCPArrayConstructor
        src/Main.cpp
        include/Main.h
)

target_include_directories(LCPArrayConstructor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
