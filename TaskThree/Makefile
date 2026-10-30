CC = cc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -O2
RAYLIB = $(shell pkg-config --cflags --libs raylib)
CORE = anketa.c sha256.c
HEADERS = anketa.h sha256.h

all: anketa

anketa: main.c $(CORE) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ main.c $(CORE)

gui: anketa_gui

anketa_gui: gui.c $(CORE) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ gui.c $(CORE) $(RAYLIB) -lm

clean:
	rm -f anketa anketa_gui

.PHONY: all gui clean
