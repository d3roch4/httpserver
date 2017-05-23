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
   
