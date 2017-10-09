cmake_minimum_required(VERSION 2.8.10)

project(httpserver)

find_package(Boost REQUIRED COMPONENTS system)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

file(GLOB CPP_FILES src/*.cpp)

set(INCLUDES_DIR    ${PROJECT_SOURCE_DIR}/src
                    ${Boost_INCLUDE_DIRS}
                    ${CMAKE_INSTALL_PREFIX}/include)

include_directories(${INCLUDES_DIR})
link_directories(${LINK_DIRECTORIES}
                    ${CMAKE_INSTALL_PREFIX}/lib
                    ${CMAKE_INSTALL_PREFIX}/lib64 
)

file(GLOB CPP_FILES src/*.cpp)
add_library(${PROJECT_NAME} ${CPP_FILES})

###### dependencies ######
if(NOT EXISTS ${CMAKE_BINARY_DIR}/beast.cmake)
    file(DOWNLOAD https://github.com/d3roch4/CMakeTemplates/raw/master/beast.cmake ${CMAKE_BINARY_DIR}/beast.cmake)
    file(DOWNLOAD https://github.com/d3roch4/CMakeTemplates/raw/master/uri.cmake ${CMAKE_BINARY_DIR}/uri.cmake)
    file(DOWNLOAD https://github.com/d3roch4/CMakeTemplates/raw/master/jsoncpp.cmake ${CMAKE_BINARY_DIR}/jsoncpp.cmake)
endif()
include(${CMAKE_BINARY_DIR}/beast.cmake)
include(${CMAKE_BINARY_DIR}/uri.cmake)
include(${CMAKE_BINARY_DIR}/jsoncpp.cmake)
###### dependencies ######

install ( DIRECTORY src/
  DESTINATION include/
  FILES_MATCHING PATTERN "*.hpp")

# Expose B's public includes (including Boost transitively) to other
# subprojects through cache variable.
set(${PROJECT_NAME}_INCLUDE_DIRS ${INCLUDES_DIR} ${CPPNETLIB_INCLUDE} ${BEAST_INCLUDE} ${URI_INCLUDE}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)
set(${PROJECT_NAME}_LIBS ${PROJECT_NAME} ${CPPNETLIB_LIBS} ssl crypto
    CACHE INTERNAL "${PROJECT_NAME}: Library Directories" FORCE)
   
