cmake_minimum_required(VERSION 3.10)

# set the project name
project(cs6120_assignments)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)

set(LLVM_DIR "${LLVM_HOME}/build/lib/cmake/llvm")
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION} in ${LLVM_DIR}")

# Include the part of LLVM's CMake libraries that defines
# `add_llvm_pass_plugin`.
include(AddLLVM)

# add LLVM includes
include_directories("${LLVM_INCLUDE_DIRS}")

# add local includes for all projects
include_directories("${PROJECT_SOURCE_DIR}/include")

# add the libraries
add_library(cfg_utils OBJECT "${PROJECT_SOURCE_DIR}/src/lib/cfg_utils.cpp")
add_library(cfg OBJECT "${PROJECT_SOURCE_DIR}/src/lib/cfg.cpp")
add_library(dataflow_analysis OBJECT "${PROJECT_SOURCE_DIR}/src/lib/dataflow_analysis.cpp")
add_library(l3_helpers OBJECT "${PROJECT_SOURCE_DIR}/src/lib/lesson3/helpers.cpp")

# add the executables
add_executable(l2 "${PROJECT_SOURCE_DIR}/src/main/lesson2.cpp")
add_executable(l3_tdce "${PROJECT_SOURCE_DIR}/src/main/lesson3_tdce.cpp")
add_executable(l3_lvn "${PROJECT_SOURCE_DIR}/src/main/lesson3_lvn.cpp")
add_executable(l4_dataflow "${PROJECT_SOURCE_DIR}/src/main/lesson4.cpp")
add_executable(l5_global "${PROJECT_SOURCE_DIR}/lesson5/main.cpp")
add_executable(l6_ssa "${PROJECT_SOURCE_DIR}/lesson6/main.cpp")

add_llvm_pass_plugin(L7Pass
    # List your source files here.
    lesson7/function_pass.cpp
)

target_link_libraries(cfg cfg_utils)
target_link_libraries(l3_lvn l3_helpers)
target_link_libraries(l3_tdce l3_helpers)
target_link_libraries(dataflow_analysis cfg cfg_utils)
target_link_libraries(l4_dataflow -fsanitize=address -static-libasan dataflow_analysis cfg cfg_utils)
target_link_libraries(l6_ssa cfg cfg_utils)
