include(FetchContent)

FetchContent_Declare (d3util
    GIT_REPOSITORY "https://github.com/d3roch4/d3util.git"
)
FetchContent_GetProperties(d3util)
FetchContent_Populate(d3util)
add_subdirectory(${d3util_SOURCE_DIR} ${d3util_BINARY_DIR} EXCLUDE_FROM_ALL)



find_package(OpenSSL QUIET)
if(NOT OPENSSL_FOUND)
    FetchContent_Declare (openssl
            GIT_REPOSITORY "https://github.com/janbar/openssl-cmake.git"
            )
    FetchContent_GetProperties(openssl)
    FetchContent_Populate(openssl)
    add_subdirectory(${openssl_SOURCE_DIR} ${openssl_BINARY_DIR} EXCLUDE_FROM_ALL)
    set(OPENSSL_SSL_LIBRARY ssl)
    set(OPENSSL_CRYPTO_LIBRARY crypto)
    set(OPENSSL_INCLUDE_DIR "${openssl_BINARY_DIR}/include" "${openssl_BINARY_DIR}")
    set(OPENSSL_FOUND ON)
    message(STATUS "Build OpenSSL: ${openssl_BINARY_DIR}")
endif()
set (HAVE_OPENSSL 1)
include_directories (${OPENSSL_INCLUDE_DIR})
