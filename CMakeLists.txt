cmake_minimum_required(VERSION 3.23)
project(claster)
set(CMAKE_CXX_STANDARD 11)

add_executable(claster main.cpp tinyxml2.h tinyxml2.cpp)
