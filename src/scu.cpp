#include<cassert>
#include<cstdio>
#include<cstdlib>
#include <cerrno>

#include<png.h>
#include<SDL_ttf.h>
#include<SDL2/SDL.h>

template <typename T>
T min(T a, T b) {
  return a < b ? a : b;
}

// * Single Compilation Unit
#include "./error.cpp"
#include "./something_result.cpp"
#include "./something_string_view.cpp"
#include "./vec.cpp"
#include "./sprite.cpp"
#include "./level.cpp"
#include "./projectile.cpp"
#include "./entity.cpp"
#include "./main.cpp"

