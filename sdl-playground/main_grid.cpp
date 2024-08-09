#include<cstdio>
#include<cstdlib>

#include<SDL.h>

template <typename T>
T max(T a, T b) {
  return a > b ? a : b;
}

int sdl(int code) {
  if(code < 0) {
    fprintf(stderr, "SDL popped itself: %s\n", SDL_GetError());
    abort();
  }
  return code;
}

template <typename T>
T *sdl(T *ptr) {
  if(ptr == nullptr) {
    fprintf(stderr, "SDL popped itself: %s\n", SDL_GetError());
    abort(); 
  }
  return ptr;
} 

constexpr int TILE_SIZE = 64;

int main() {
  sdl(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS));

  SDL_Window *window = sdl(SDL_CreateWindow("Some Game", 0, 0, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE));

  SDL_Renderer *renderer = sdl(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));

  bool quit = false;
  while (!quit) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: {
          quit = true;
        }
        case SDL_MOUSEWHEEL: {
          TILE_SIZE = max(0, TILE_SIZE + event.wheel.y);
          char title[256];
          snprintf(title, 256, "tile size is %d", TILE_SIZE);
          SDL_SetWindowTitle(window, title);
        }
        break;
      }
    }

    sdl(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255));
    sdl(SDL_RenderClear(renderer));

    int window_width = 0, window_height = 0;
    SDL_GetWindowSize(window, &window_width,
                      &window_height);

    int columns = window_width / TILE_SIZE;
    int rows = window_height / TILE_SIZE;

    // printf("window_width %d\n", window_width);
    // printf("window_height %d\n", window_height);
    // printf("Rows %d\n", rows);

    sdl(SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255));
    for (int row = 0; row < rows; ++row) {
      sdl(SDL_RenderDrawLine(renderer,
                             0,
                             (row + 1) * TILE_SIZE,
                             window_width,
                             (row + 1) * TILE_SIZE));
    }

    for (int col = 0; col < columns; ++col) {
      sdl(SDL_RenderDrawLine(renderer,
                             (col + 1) * TILE_SIZE,
                             0,
                             (col + 1) * TILE_SIZE,
                             window_height));
    }

    SDL_RenderPresent(renderer);
  }

  SDL_Quit();
  return 0;
}