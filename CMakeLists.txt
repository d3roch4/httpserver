cmake_minimum_required(VERSION 3.14)
project(httpserver VERSION 1.4.2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} $ENV{CXXFLAGS} $ENV{CPPFLAGS}")
set(CMAKE_CXX_STANDARD 14) # -std=c++14
option(BUILD_EXAMPLE "Build example code" OFF)

include(deps.cmake)

#add_compile_definitions(BOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED=1)
find_package(Boost COMPONENTS system thread regex stacktrace_addr2line)
find_package(OpenSSL REQUIRED) 
find_package(ZLIB REQUIRED)

include_directories(./ ${OPENSSL_INCLUDE_DIRS})

file(GLOB CPP_FILES httpserver/*/*.cpp httpserver/*.cpp)
file(GLOB HPP_FILES httpserver/*/*.h httpserver/*.h)
add_library(${PROJECT_NAME} STATIC ${CPP_FILES} ${HPP_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ./)
target_link_libraries(${PROJECT_NAME}
        ${Boost_LIBRARIES}
        ${OPENSSL_SSL_LIBRARY}
        ${OPENSSL_CRYPTO_LIBRARY}
        d3util
        pthread
        ZLIB::ZLIB
)


#include(jsoncpp.cmake)
#target_include_directories(${PROJECT_NAME} PUBLIC ${JSONCPP_INCLUDE})

if(BUILD_EXAMPLE)
  add_subdirectory(exemplo)
endif()

if( CMAKE_COMPILER_IS_MINGW )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Og -Wa,-mbig-obj")
elseif( MSVC )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ${HPP_FILES} DESTINATION include/${PROJECT_NAME})
