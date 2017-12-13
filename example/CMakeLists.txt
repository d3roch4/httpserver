cmake_minimum_required(VERSION 2.8)

project(test_httpserver)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

file(GLOB CPP_FILES *.cpp)
add_executable(${PROJECT_NAME} ${CPP_FILES})

target_link_libraries(${PROJECT_NAME} httpserver)
