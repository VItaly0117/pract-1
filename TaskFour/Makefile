CC = cc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -O2
RAYLIB = $(shell pkg-config --cflags --libs raylib)

all: guess

guess: main.c game.c game.h
	$(CC) $(CFLAGS) -o $@ main.c game.c

gui: guess_gui

guess_gui: gui.c game.c game.h
	$(CC) $(CFLAGS) -o $@ gui.c game.c $(RAYLIB)

clean:
	rm -f guess guess_gui

.PHONY: all gui clean
