PKGS=sdl2 libpng SDL2_ttf
CXXFLAGS=-Wall -Wextra -pedantic -ggdb -std=c++17 `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs $(PKGS)` -lm

something: src/scu.cpp src/main.cpp src/vec.cpp src/sprite.cpp src/error.cpp src/level.cpp src/projectile.cpp src/entity.cpp
	$(CXX) $(CXXFLAGS) -o something src/scu.cpp $(LIBS)