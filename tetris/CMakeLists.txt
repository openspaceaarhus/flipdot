include(FindPkgConfig)
pkg_search_module(SDL REQUIRED sdl)
include_directories(${SDL_INCLUDE_DIRS})
pkg_search_module(SDLIMAGE REQUIRED SDL_image)
include_directories(${SDLIMAGE_INCLUDE_DIRS})

add_executable(
  tetris
  main.c
  game.c
  sdl_game.c
  )
target_link_libraries(tetris ${SDL_LIBRARIES} ${SDLIMAGE_LIBRARIES})
