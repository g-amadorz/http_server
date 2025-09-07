cmake_minimum_required(VERSION 3.10)
project(server)

set(CMAKE_C_STANDARD 11) 

# Set source files
set(SOURCES
    src/main.c
    src/server.c
    src/utils.c)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Add debug flags
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")


add_executable(server ${SOURCES})



