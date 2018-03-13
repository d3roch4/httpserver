cmake_minimum_required(VERSION 2.8)

project(exemplo)

add_executable(exemplo exemplo.cpp)
target_link_libraries(exemplo httpserver)
