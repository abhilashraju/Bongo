cmake_minimum_required(VERSION 3.1)

project (bongo)

# set the C++ standard to C++ 11
set(CMAKE_CXX_STANDARD 17)
# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message("Setting build type to 'RelWithDebInfo' as none was specified.")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

add_compile_options(-Wall -Wextra -pedantic -fconcepts) 

SET (PROJECT_ROOT "${PROJECT_SOURCE_DIR}")
SET (PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
SET (PROJECT_EXAMPLES_DIR "${PROJECT_ROOT}/examples")
SET (PROJECT_BINARY_DIR "${PROJECT_BINARY_DIR}/bin")
MESSAGE ("Source path: ${PROJECT_SOURCE_DIR}")
MESSAGE ("Binary path: ${PROJECT_BINARY_DIR}")

include_directories("
    ${PROJECT_SOURCE_DIR}
    ")

# Add an executable
add_executable(bongo 
                ${PROJECT_EXAMPLES_DIR}/nse.cpp 
                ${PROJECT_SOURCE_DIR}/scurlholder.cpp 
                ${PROJECT_SOURCE_DIR}/shttputil.cpp 
                ${PROJECT_SOURCE_DIR}/smultipart_parser.cpp 
                ${PROJECT_SOURCE_DIR}/scurlclient.cpp 
                ${PROJECT_SOURCE_DIR}/shttpproxies.cpp 
                ${PROJECT_SOURCE_DIR}/shttperror.cpp 
                ${PROJECT_SOURCE_DIR}/scookies.cpp )

# link against the boost libraries
target_link_libraries( bongo
    PRIVATE
        # Boost::filesystem
        ssl
        crypto
        pthread
        curl
)