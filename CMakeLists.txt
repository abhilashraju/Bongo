cmake_minimum_required(VERSION 3.1)

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
# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message("Setting build type to 'RelWithDebInfo' as none was specified.")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()
# Add an executable
add_executable(httpex main.cpp scurlholder.cpp shttputil.cpp smultipart_parser.cpp scurlclient.cpp shttpproxies.cpp shttperror.cpp scookies.cpp )

# link against the boost libraries
target_link_libraries( httpex
    PRIVATE
        # Boost::filesystem
        ssl
        crypto
        pthread
        curl
)