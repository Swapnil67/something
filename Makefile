PKGS=sdl2 libpng
CFLAGS=-Wall -Wextra -pedantic -ggdb -std=c++17 `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs $(PKGS)` -lm

something: main.cpp
	gcc $(CFLAGS) -o something main.cpp $(LIBS)