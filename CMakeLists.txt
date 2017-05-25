cmake_minimum_required(VERSION 2.8.10)

project(http_server)

file(GLOB CPP_FILES src/*.cpp)

set(INCLUDES_DIR    ${PROJECT_SOURCE_DIR}/src
                    ${Boost_INCLUDE_DIRS}
                    ${CMAKE_INSTALL_PREFIX}/include)

include_directories(${INCLUDES_DIR})
link_directories(${LINK_DIRECTORIES}
                    ${CMAKE_INSTALL_PREFIX}/lib
                    ${CMAKE_INSTALL_PREFIX}/lib64 )
                    
install ( DIRECTORY src/
  DESTINATION include/
  FILES_MATCHING PATTERN "*.hpp")

# Expose B's public includes (including Boost transitively) to other
# subprojects through cache variable.
set(${PROJECT_NAME}_INCLUDE_DIRS ${INCLUDES_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)
set(${PROJECT_NAME}_LIBRARY_DIRS
                    ${CMAKE_INSTALL_PREFIX}/lib
                    ${CMAKE_INSTALL_PREFIX}/lib64
    CACHE INTERNAL "${PROJECT_NAME}: Library Directories" FORCE)
   


include(ExternalProject)
ExternalProject_Add (
    cppnetlib
    URL  "https://github.com/cpp-netlib/cpp-netlib/archive/cpp-netlib-0.11.2-final.tar.gz"
    CMAKE_ARGS 
        "-DBOOST_INCLUDEDIR=${BOOST_INCLUDEDIR}"
        "-DBOOST_LIBRARYDIR=${BOOST_LIBRARYDIR}"
        "-DCPP-NETLIB_BUILD_TESTS=OFF"
        "-DCPP-NETLIB_BUILD_EXPERIMENTS=OFF"
        "-DCPP-NETLIB_BUILD_EXAMPLES=OFF" )
set_target_properties(cppnetlib  PROPERTIES POSITION_INDEPENDENT_CODE ON)    
