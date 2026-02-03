cmake_minimum_required(VERSION 3.23)

project(Task1)

option(USE_DOUBLE "Set output value to double" ON)
if(USE_DOUBLE)
    add_definitions(-DVAL_DOUBLE)
endif()
add_executable(main)
target_sources(main PRIVATE main.cpp)