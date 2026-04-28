cmake_minimum_required(VERSION 3.10)
project(Task_2.1)

# Enable C++11 or later
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
# Find OpenMP
find_package(OpenMP REQUIRED)

# Create executable
add_executable(main main.cpp)

# Link OpenMP
target_link_libraries(main PRIVATE OpenMP::OpenMP_CXX)