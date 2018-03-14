project(httpserver)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
option(BUILD_EXAMPLE "Build example code" OFF)

file(GLOB CPP_FILES httpserver/*.cpp)
file(GLOB HPP_FILES httpserver/*.h)
add_library(${PROJECT_NAME} ${CPP_FILES} ${HPP_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ./)

find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} pthread)

execute_process(COMMAND git submodule update --init WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
execute_process(COMMAND git submodule update --remote --recursive deps/mor WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(deps/mor)
target_link_libraries(${PROJECT_NAME} mor)

option(RAPIDJSON_BUILD_EXAMPLES "" OFF)
execute_process(COMMAND cd deps && wget https://github.com/Tencent/rapidjson/archive/v1.1.0.tar.gz -O - | tar -xz && cd - WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(deps/rapidjson-1.1.0)
target_include_directories(${PROJECT_NAME} PUBLIC deps/rapidjson-1.1.0/include)

if(BUILD_EXAMPLE)
  add_subdirectory(exemplo)
endif()
