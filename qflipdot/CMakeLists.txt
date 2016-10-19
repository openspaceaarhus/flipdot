find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)
find_package(Qt5Network)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

add_executable(
  qflip
  main.cpp
  fddialog.h
  fddialog.cpp
  qflipdot.h
  qflipdot.cpp
  )
qt5_use_modules(qflip Core Gui Widgets Network)
