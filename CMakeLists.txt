cmake_minimum_required(VERSION 3.10)

# set the project name
project(cs6120_assignments)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)

# add include for all projects
include_directories("${PROJECT_SOURCE_DIR}/include")

# add the libraries
add_library(cfg OBJECT "${PROJECT_SOURCE_DIR}/src/lib/cfg.cpp")
add_library(dataflow_analysis OBJECT "${PROJECT_SOURCE_DIR}/src/lib/dataflow_analysis.cpp" $<TARGET_OBJECTS:cfg>)

# add the executables
add_executable(l2 "${PROJECT_SOURCE_DIR}/lesson2/count_branches.cpp")
add_executable(l3_tdce "${PROJECT_SOURCE_DIR}/lesson3/tdce.cpp")
add_executable(l3_lvn "${PROJECT_SOURCE_DIR}/lesson3/lvn.cpp")
add_executable(l4_dataflow "${PROJECT_SOURCE_DIR}/src/main/lesson4.cpp" $<TARGET_OBJECTS:cfg> $<TARGET_OBJECTS:dataflow_analysis>)
add_executable(l5_global "${PROJECT_SOURCE_DIR}/lesson5/main.cpp")
add_executable(l6_ssa "${PROJECT_SOURCE_DIR}/lesson6/main.cpp" $<TARGET_OBJECTS:cfg>)

target_link_libraries(l4_dataflow -fsanitize=address -static-libasan)