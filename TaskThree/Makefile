CXX = c++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O2
RAYLIB = $(shell pkg-config --cflags --libs raylib)

all: anketa

anketa: main.cpp anketa.h
	$(CXX) $(CXXFLAGS) -o $@ main.cpp

gui: anketa_gui

anketa_gui: gui.cpp anketa.h
	$(CXX) $(CXXFLAGS) -o $@ gui.cpp $(RAYLIB)

clean:
	rm -f anketa anketa_gui

.PHONY: all gui clean
