CXX = c++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O2
RAYLIB = $(shell pkg-config --cflags --libs raylib)

all: guess

guess: main.cpp game.h
	$(CXX) $(CXXFLAGS) -o $@ main.cpp

gui: guess_gui

guess_gui: gui.cpp game.h
	$(CXX) $(CXXFLAGS) -o $@ gui.cpp $(RAYLIB)

clean:
	rm -f guess guess_gui

.PHONY: all gui clean
