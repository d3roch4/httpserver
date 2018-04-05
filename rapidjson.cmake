
if(NOT TARGET rapidjson)
    include(ExternalProject)

    ExternalProject_Add (
        rapidjson
        URL  "https://github.com/Tencent/rapidjson/archive/v1.1.0.tar.gz"
        CMAKE_ARGS "-G${CMAKE_GENERATOR}"
            "-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} -fPIC"
            "-DRAPIDJSON_BUILD_EXAMPLES=OFF"
            "-DRAPIDJSON_HAS_STDSTRING=ON"
            "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        BUILD_COMMAND ""
        BUILD_IN_SOURCE 1
        INSTALL_COMMAND ""
    )
    ExternalProject_Get_Property(rapidjson source_dir binary_dir)

    set(RAPIDJSON_INCLUDE "${source_dir}/include" CACHE INTERNAL "RAPIDJSON Include Directories" FORCE)
endif()

add_dependencies(${PROJECT_NAME} rapidjson)
include_directories(${RAPIDJSON_INCLUDE})
