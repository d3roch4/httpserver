cmake_minimum_required(VERSION 2.8)

project(exemplo_httpserver)

add_executable(exemplo exemplo.cpp)
target_link_libraries(exemplo httpserver)
