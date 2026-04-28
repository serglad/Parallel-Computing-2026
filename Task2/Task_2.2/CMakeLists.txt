cmake_minimum_required(VERSION 3.10)
project(Task_2.2)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(OpenMP REQUIRED)


add_executable(main main.cpp)


target_link_libraries(main PRIVATE OpenMP::OpenMP_CXX)