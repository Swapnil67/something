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

void print1(FILE *stream, char c) {
  fputc(c, stream);
}

void print1(FILE *stream, int x) {
  fprintf(stream, "%d", x);
}

void print1(FILE *stream, long unsigned int x) {
  fprintf(stream, "%lu", x);
}

void print1(FILE *stream, uint32_t x) {
  fprintf(stream, "%u", x);
}

void print1(FILE *stream, const char *cstr) {
  fputs(cstr, stream);
}


struct Pad {
  size_t n;
  char c;
};

void print1(FILE *stream, Pad pad) {
  for (size_t i = 0; i < pad.n; ++i) {
    fputc(pad.c, stream);
  }
}

template <typename... T>
void print([[maybe_unused]] FILE *stream, T... args) {
  // (void) stream;
  (print1(stream, args), ...);
}

template <typename... T>
void println(FILE *stream, T... args) {
  print(stream, args...);
  fputc('\n', stream);
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

