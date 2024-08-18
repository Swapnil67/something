PKGS=sdl2 libpng SDL2_ttf
CFLAGS=-Wall -Wextra -pedantic -ggdb -std=c++17 `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs $(PKGS)` -lm

something: src/main.cpp
	$(CXX) $(CFLAGS) -o something src/main.cpp $(LIBS)