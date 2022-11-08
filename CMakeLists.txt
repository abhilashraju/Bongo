

cmake_minimum_required(VERSION 3.15)

# include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
project(Bongo CXX)
set(CMAKE_CXX_STANDARD 17)

add_compile_options(-O0 -Wall -Wextra -pedantic -fconcepts -fPIC)

# add_library(Bongo
# ${PROJECT_SOURCE_DIR}/src/scurlholder.cpp
# ${PROJECT_SOURCE_DIR}/src/shttputil.cpp
# ${PROJECT_SOURCE_DIR}/src/smultipart_parser.cpp
# ${PROJECT_SOURCE_DIR}/src/scurlclient.cpp
# ${PROJECT_SOURCE_DIR}/src/shttpproxies.cpp
# ${PROJECT_SOURCE_DIR}/src/shttperror.cpp
# ${PROJECT_SOURCE_DIR}/src/scookies.cpp
# )
# target_include_directories(Bongo PUBLIC include)
# install(TARGETS Bongo)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    TYPE INCLUDE
    PATTERN "*.in" EXCLUDE)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/
    TYPE INCLUDE
    PATTERN "*.in" EXCLUDE)
