cmake_minimum_required(VERSION 3.0.0)

project(QFlip)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")


find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)
find_package(Qt5Network)

add_executable(
  qflip
  main.cpp
  fddialog.h
  fddialog.cpp
  qflipdot.h
  qflipdot.cpp
  )
qt5_use_modules(qflip Core Gui Widgets Network)
