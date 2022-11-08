cmake_minimum_required(VERSION 3.1)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
set(app example)
project(${app})

# set the C++ standard to C++ 17
set(CMAKE_CXX_STANDARD 17)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message("Setting build type to 'RelWithDebInfo' as none was specified.")
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)

    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
        "MinSizeRel" "RelWithDebInfo")
endif()

add_compile_options(-O0 -Wall -Wextra -pedantic -fconcepts -fPIE)

SET(PROJECT_ROOT "${PROJECT_SOURCE_DIR}")
set(UNIFEX_CXX_CONCEPTS OFF CACHE BOOL " Compile unit tests ") # Configurable by user
set(OTHER_CONSTANT OFF) # Not configurable by user
configure_file(config.h.in config.h)
include_directories(
    " ${CONAN_INCLUDE_DIRS_BONGO} " ,
    " ${CONAN_INCLUDE_DIRS_LIBUNIFEX} "
    " ${CONAN_INCLUDE_DIRS} "
    " ${PROJECT_BINARY_DIR}"
)

add_executable(${app}
    example.cpp
)

link_directories(
    "/usr/local/lib/"
    "${PROJECT_BINARY_DIR}/bin"
)
target_link_libraries(${app}
    PRIVATE
    pthread
    ${CONAN_LIBS_LIBCURL}

    # ${PROJECT_BINARY_DIR}/bin/libBongo.a
    ${PROJECT_BINARY_DIR}/bin/libunifex.a
)