cmake_minimum_required(VERSION 2.8)

project(exemplo_httpserver)

add_executable(${PROJECT_NAME} exemplo.cpp)
target_link_libraries(${PROJECT_NAME} httpserver)
