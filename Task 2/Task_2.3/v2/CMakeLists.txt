cmake_minimum_required(VERSION 3.10)
project(ver2)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(OpenMP REQUIRED)


add_executable(ver2 ver2.cpp)


target_link_libraries(ver2 PRIVATE OpenMP::OpenMP_CXX)