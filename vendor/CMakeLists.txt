#
# vendor/CMakeLists.txt
#

# Libraries

# zlib-ng
set(WITH_GZFILEOP OFF CACHE BOOL "")
set(ZLIB_ENABLE_TESTS OFF CACHE BOOL "")
set(ZLIB_COMPAT ON CACHE BOOL "")
add_subdirectory(zlib-ng)

# AES Library
add_library(AES256 STATIC
    aes256/aes256.c
    aes256/aes256.h
)
target_compile_definitions(AES256 PRIVATE BACK_TO_TABLES)