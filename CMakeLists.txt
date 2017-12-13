cmake_minimum_required(VERSION 2.8.10)

project(httpserver)

find_package(Boost REQUIRED COMPONENTS system)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
option(BUILD_EXAMPLE "Build example code" OFF)

file(GLOB CPP_FILES src/*.cpp)
add_library(${PROJECT_NAME} ${CPP_FILES})

target_link_libraries(${PROJECT_NAME} mor libjsoncpp ssl crypto pthread ${Boost_LIBRARIES})
if(MSVS OR MSYS OR MINGW)
    target_link_libraries(${PROJECT_NAME}   ws2_32 wsock32)
endif ()


###### dependencies ######
if(NOT EXISTS ${CMAKE_BINARY_DIR}/jsoncpp.cmake OR NOT EXISTS ${CMAKE_BINARY_DIR}/beast.cmake)
    file(DOWNLOAD https://github.com/d3roch4/CMakeTemplates/raw/master/jsoncpp.cmake ${CMAKE_BINARY_DIR}/jsoncpp.cmake)
    file(DOWNLOAD https://github.com/d3roch4/CMakeTemplates/raw/master/beast.cmake ${CMAKE_BINARY_DIR}/beast.cmake)
endif()
include(${CMAKE_BINARY_DIR}/jsoncpp.cmake)
include(${CMAKE_BINARY_DIR}/beast.cmake)
if(EXISTS ${PROJECT_SOURCE_DIR}/deps/mor/CMakeLists.txt AND NOT TARGET mor)
    add_subdirectory(deps/mor)
endif()
###### dependencies ######


set(${PROJECT_NAME}_INCLUDE
    ${PROJECT_SOURCE_DIR}/src
    ${JSONCPP_INCLUDE}
    ${BEAST_INCLUDE}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
)

include_directories(${httpserver_INCLUDE})
target_include_directories(${PROJECT_NAME} PUBLIC ${httpserver_INCLUDE})

install ( DIRECTORY src/
  DESTINATION include/
  FILES_MATCHING PATTERN "*.hpp")

set(${PROJECT_NAME}_LIBS ${PROJECT_NAME} libjsoncpp ssl crypto pthread
    CACHE INTERNAL "${PROJECT_NAME}: Library Directories" FORCE)
   

#### Test
if(BUILD_EXAMPLE)
    add_subdirectory(example)
endif()
