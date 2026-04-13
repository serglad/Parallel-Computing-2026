cmake_minimum_required(VERSION 3.10)
project(Task_2.2)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(OpenMP REQUIRED)


add_executable(main main.cpp)


target_link_libraries(main PRIVATE OpenMP::OpenMP_CXX)