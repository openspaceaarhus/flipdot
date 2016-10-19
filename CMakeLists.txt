cmake_minimum_required(VERSION 3.5)

project(FlipDots)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(qflipdot)
add_subdirectory(flyboy)
add_subdirectory(tetris)
add_subdirectory(flipmos)
