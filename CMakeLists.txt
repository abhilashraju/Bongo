cmake_minimum_required(VERSION 3.1)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
project(bongo)

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

add_compile_options(-O0 -Wall -Wextra -pedantic)

SET(PROJECT_ROOT "${PROJECT_SOURCE_DIR}")
SET(PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
SET(PROJECT_EXAMPLES_DIR "${PROJECT_ROOT}/examples")

# SET(PROJECT_BINARY_DIR "${PROJECT_BINARY_DIR}/bin")
MESSAGE("Source path: ${PROJECT_SOURCE_DIR}")
MESSAGE("Binary path: ${PROJECT_BINARY_DIR}")
MESSAGE("Libunifex  : ${CONAN_INCLUDE_DIRS_LIBUNIFEX} ")

set(UNIFEX_CXX_CONCEPTS OFF CACHE BOOL " Compile unit tests ") # Configurable by user
set(OTHER_CONSTANT OFF) # Not configurable by user
configure_file(config.h.in config.h)

include_directories(
    " ${PROJECT_SOURCE_DIR} " ,
    " ${CONAN_INCLUDE_DIRS} " ,
    " ${PROJECT_BINARY_DIR} " ,
    " ${CONAN_INCLUDE_DIRS_LIBUNIFEX} " ,
    "/home/rabhil/.conan/data/libunifex/cci.20220430/_/_/package/13558dbc3d495e9231be308e4222af2a4b28bead/include"
)

# Add an executable
add_executable(bongo

    # ${PROJECT_EXAMPLES_DIR}/sender_reciever.cpp
    ${PROJECT_EXAMPLES_DIR}/bongo.cpp

    # ${PROJECT_EXAMPLES_DIR}/nse.cpp
    ${PROJECT_SOURCE_DIR}/scurlholder.cpp
    ${PROJECT_SOURCE_DIR}/shttputil.cpp
    ${PROJECT_SOURCE_DIR}/smultipart_parser.cpp
    ${PROJECT_SOURCE_DIR}/scurlclient.cpp
    ${PROJECT_SOURCE_DIR}/shttpproxies.cpp
    ${PROJECT_SOURCE_DIR}/shttperror.cpp
    ${PROJECT_SOURCE_DIR}/scookies.cpp
)

link_directories(
    " /usr/local/lib/ "
    " ${CONAN_LIB_DIRS} "
)

# link against the boost libraries
target_link_libraries(bongo
    PRIVATE
    pthread
    ${CONAN_LIBS_LIBUNIFEX}
    ${CONAN_LIBS_LIBCURL}
)