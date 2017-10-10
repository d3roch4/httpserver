cmake_minimum_required(VERSION 2.8.10)

project(httpserver)

find_package(Boost REQUIRED COMPONENTS system)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include_directories(${PROJECT_SOURCE_DIR}/src)

file(GLOB CPP_FILES src/*.cpp)
add_library(${PROJECT_NAME} ${CPP_FILES})

###### dependencies ######
if(NOT EXISTS ${CMAKE_BINARY_DIR}/beast.cmake)
    file(DOWNLOAD https://github.com/d3roch4/CMakeTemplates/raw/master/beast.cmake ${CMAKE_BINARY_DIR}/beast.cmake)
    file(DOWNLOAD https://github.com/d3roch4/CMakeTemplates/raw/master/jsoncpp.cmake ${CMAKE_BINARY_DIR}/jsoncpp.cmake)
endif()
include(${CMAKE_BINARY_DIR}/beast.cmake)
include(${CMAKE_BINARY_DIR}/jsoncpp.cmake)
###### dependencies ######

install ( DIRECTORY src/
  DESTINATION include/
  FILES_MATCHING PATTERN "*.hpp")
  
set(${PROJECT_NAME}_INCLUDE ${PROJECT_SOURCE_DIR}/src ${BEAST_INCLUDE} ${JSONCPP_INCLUDE}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)
set(${PROJECT_NAME}_LIBS ${PROJECT_NAME} libjsoncpp ssl crypto
    CACHE INTERNAL "${PROJECT_NAME}: Library Directories" FORCE)
   
