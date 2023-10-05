include(FetchContent)

FetchContent_Declare (d3util
    GIT_REPOSITORY "https://github.com/d3roch4/d3util.git"
)
FetchContent_GetProperties(d3util)
FetchContent_Populate(d3util)
add_subdirectory(${d3util_SOURCE_DIR} ${d3util_BINARY_DIR} EXCLUDE_FROM_ALL)

