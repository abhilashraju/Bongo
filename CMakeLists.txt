cmake_minimum_required(VERSION 3.1)
set(CMAKE_DEBUG_POSTFIX d)
# find a boost install with the libraries filesystem and system
# find_package(Boost 1.33 REQUIRED COMPONENTS filesystem system)

# check if boost was found
# if(Boost_FOUND)
#     message ("boost found")
# else()
#     message (FATAL_ERROR "Cannot find Boost")
# endif()
# Set the project name
project (httpex)

# set the C++ standard to C++ 11
set(CMAKE_CXX_STANDARD 17)

# Add an executable
add_executable(httpex main.cpp)
set_target_properties(httpex PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
# link against the boost libraries
target_link_libraries( httpex
    PRIVATE
        # Boost::filesystem
        ssl
        crypto
        pthread
)