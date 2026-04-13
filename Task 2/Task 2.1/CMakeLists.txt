cmake_minimum_required(VERSION 3.23)

project(Task2)

add_executable(main)
target_sources(main PRIVATE main.cpp)
target_compile_options(main PRIVATE "-fopenmp")
target_compile_options(main PRIVATE "-std=c++20") 