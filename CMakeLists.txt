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

if(NOT TARGET mor)
    add_subdirectory(../mor mor)
endif()
target_link_libraries(${PROJECT_NAME} mor)

include(rapidjson.cmake)
target_include_directories(${PROJECT_NAME} PUBLIC ${RAPIDJSON_INCLUDE})

if(BUILD_EXAMPLE)
  add_subdirectory(exemplo)
endif()
