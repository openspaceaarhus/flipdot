CC = gcc
OBJS = game.o main.o sdl_game.o
CFLAGS = -g -Wall -O1 $(shell pkg-config --cflags sdl SDL_image)
LDFLAGS = $(shell pkg-config --libs sdl SDL_image)

tetris: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)
