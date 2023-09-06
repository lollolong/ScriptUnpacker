#
# src/CMakeLists.txt
#

# Source files
set(SOURCES
    main/main.cpp
    main/main.h

    data/stringhash.cpp
    data/stringhash.h

    data/util.cpp
    data/util.h

    data/resourceheader.h
)

# Executable
add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# Includes
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}

    # vendor
    ${VENDOR_DIR}/zlib-ng
    ${VENDOR_DIR}/aes256

    # Weird case - zconf.h issue
    # https://stackoverflow.com/questions/70781662/build-and-link-zlib-fails-because-cmake-renames-files
    # https://github.com/madler/zlib/issues/133
    #${CMAKE_BINARY_DIR}/vendor/zlib
)

# Libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    zlibstatic
    AES256
)

# Warnings
target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

# Forceinclude
#if(MSVC)
#    add_definitions(/FI"main/main.h")
#else()
#    add_definitions(-include "main/main.h")
#endif()