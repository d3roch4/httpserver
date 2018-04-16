project(httpserver)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
option(BUILD_EXAMPLE "Build example code" OFF)

file(GLOB CPP_FILES httpserver/*.cpp)
file(GLOB HPP_FILES httpserver/*.h)
add_library(${PROJECT_NAME} ${CPP_FILES} ${HPP_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ./)

set(Boost_USE_STATIC_LIBS   ON)
find_package(Boost COMPONENTS system thread regex)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} pthread ssl crypto)

if(NOT TARGET mor)
    add_subdirectory(../mor mor)
endif()
target_link_libraries(${PROJECT_NAME} mor)

include(jsoncpp.cmake)
target_include_directories(${PROJECT_NAME} PUBLIC ${JSONCPP_INCLUDE})

if(BUILD_EXAMPLE)
  add_subdirectory(exemplo)
endif()


install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ${HPP_FILES} DESTINATION include/${PROJECT_NAME})
