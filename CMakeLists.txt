project(httpserver)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} $ENV{CXXFLAGS} $ENV{CPPFLAGS} -std=c++14")
option(BUILD_EXAMPLE "Build example code" OFF)

add_compile_definitions(BOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED=1)
set(Boost_USE_STATIC_LIBS   ON)
find_package(Boost COMPONENTS system thread regex)

include_directories(./)

file(GLOB CPP_FILES httpserver/*/*.cpp httpserver/*.cpp)
file(GLOB HPP_FILES httpserver/*/*.h httpserver/*.h)
add_library(${PROJECT_NAME} ${CPP_FILES} ${HPP_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ./)

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} pthread ssl crypto)

if(NOT TARGET d3util)
    add_subdirectory(deps/d3util d3util)
endif()
target_link_libraries(${PROJECT_NAME} d3util z)

include(jsoncpp.cmake)
target_include_directories(${PROJECT_NAME} PUBLIC ${JSONCPP_INCLUDE})

if(BUILD_EXAMPLE)
  add_subdirectory(exemplo)
endif()


install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ${HPP_FILES} DESTINATION include/${PROJECT_NAME})
