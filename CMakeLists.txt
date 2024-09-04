cmake_minimum_required(VERSION 3.28)
project(shell2)

set(CMAKE_CXX_STANDARD 23)

add_executable(shell2 main.cpp
        shell.cpp
        shell.h
        shell.cpp)