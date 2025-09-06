cmake_minimum_required(VERSION 3.10)
project(server)

set(CMAKE_C_STANDARD 11) 

set(SOURCES
    src/main.c
    src/server.c)

add_executable(server ${SOURCES})



