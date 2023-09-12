cmake_minimum_required(VERSION 3.10)

# set the project name
project(cs6120_assignments)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add include for all projects
include_directories("${PROJECT_SOURCE_DIR}/include")

# add the executable
add_executable(l2 "${PROJECT_SOURCE_DIR}/lesson2/count_branches.cpp")
add_executable(l3_tdce "${PROJECT_SOURCE_DIR}/lesson3/tdce.cpp")
add_executable(l3_lvn "${PROJECT_SOURCE_DIR}/lesson3/lvn.cpp")
add_executable(l4_dataflow "${PROJECT_SOURCE_DIR}/lesson4/main.cpp")