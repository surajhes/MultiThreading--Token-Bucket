cmake_minimum_required(VERSION 2.8.4)
project(hw2)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

set(SOURCE_FILES
    cs402.h
    definitions.h
    my402list.c
    my402list.h
    packetCreationThread.c
    serverThread.c
    signalHandler.c
    tokenGeneratingThread.c
    warmup2.c)

add_executable(hw2 ${SOURCE_FILES})
target_link_libraries(hw2 m)