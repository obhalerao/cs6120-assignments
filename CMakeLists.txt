cmake_minimum_required(VERSION 3.10)

# set the project name
project(cs6120_assignments)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add the executable
add_executable(l2 "${PROJECT_SOURCE_DIR}/lesson2/count_branches.cpp")

# add include directories
target_include_directories(l2 PUBLIC
                          "${PROJECT_SOURCE_DIR}/include"
                          )