PKGS=sdl2
CFLAGS=-Wall -Wextra -ggdb -std=c++11 `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs $(PKGS)` -lm

something: main.cpp
	gcc $(CFLAGS) -o something main.cpp $(LIBS)