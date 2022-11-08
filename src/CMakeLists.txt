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

add_compile_options(-O0 -Wall -Wextra -pedantic -fconcepts -fPIE -fvisibility=default)

SET(PROJECT_ROOT "${PROJECT_SOURCE_DIR}")
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    TYPE INCLUDE
    PATTERN "*.in" EXCLUDE)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/
    TYPE INCLUDE
    PATTERN "*.in" EXCLUDE)

# set(UNIFEX_CXX_CONCEPTS OFF CACHE BOOL " Compile unit tests ") # Configurable by user
# set(OTHER_CONSTANT OFF) # Not configurable by user
# configure_file(config.h.in config.h)

# include_directories(
# " ${CONAN_INCLUDE_DIRS} " ,
# " ${CONAN_INCLUDE_DIRS_LIBUNIFEX} "
# "${PROJECT_ROOT}/include"
# )

# add_library(bongo
# PRIVATE
# # ${PROJECT_SOURCE_DIR}/scurlholder.cpp
# # ${PROJECT_SOURCE_DIR}/shttputil.cpp
# # ${PROJECT_SOURCE_DIR}/smultipart_parser.cpp
# # ${PROJECT_SOURCE_DIR}/scurlclient.cpp
# # ${PROJECT_SOURCE_DIR}/shttpproxies.cpp
# # ${PROJECT_SOURCE_DIR}/shttperror.cpp
# # ${PROJECT_SOURCE_DIR}/scookies.cpp
# )

# link_directories(
# "/usr/local/lib/"
# "${PROJECT_BINARY_DIR}/bin"
# )
# target_link_libraries(bongo
# PRIVATE
# pthread
# ${CONAN_LIBS_LIBCURL}
# ${PROJECT_BINARY_DIR}/bin/libunifex.a
# )