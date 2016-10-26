add_executable(
  fakemos
  maze.cpp
  main.cpp
  PbmDraw.h

  TerminalFake.cpp
  )




include_directories("${CMAKE_SOURCE_DIR}/flipmos")
