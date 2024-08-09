#include<cstdio>
#include<cstdlib>
#include<png.h>

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

enum class Tile {
  Empty = 0,
  Wall
};

constexpr int LEVEL_WIDTH = 5;
constexpr int LEVEL_HEIGHT = 5;
Tile level[LEVEL_HEIGHT][LEVEL_WIDTH] = {
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Wall, Tile::Wall},
    {Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Wall, Tile::Wall, Tile::Wall, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty}};

struct Tile_Texture
{
  SDL_Rect srcrect;
  SDL_Texture *texture;
};

void render_tile_texture(SDL_Renderer *renderer, Tile_Texture texture, int x, int y) {
  SDL_Rect destrect = {x, y, TILE_SIZE, TILE_SIZE};
  sdl(SDL_RenderCopy(renderer,
                     texture.texture,
                     &texture.srcrect,
                     &destrect));
}

void
render_level(SDL_Renderer *renderer, Tile_Texture wall_texture)
{
  for (int y = 0; y < LEVEL_HEIGHT; ++y) {
    for (int x = 0; x < LEVEL_WIDTH; ++x) {
      switch(level[y][x]) {
        case Tile::Empty: {
           
        } break;
        case Tile::Wall: {
          sdl(SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255));
          render_tile_texture(renderer, wall_texture, x * TILE_SIZE, y * TILE_SIZE);
        }
      }
    }
  }
}

int main() {
  sdl(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS));

  // * Create a SDL window
  SDL_Window *window = sdl(SDL_CreateWindow("Some Game", 0, 0, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE));

  // * Create a SDL renderer
  SDL_Renderer *renderer = sdl(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));

  // * Reading png into buffer
  const char *tileset_filename = "fantasy_tiles.png";
  png_image tileset;
  memset(&tileset, 0, sizeof(tileset));
  tileset.version = PNG_IMAGE_VERSION;
  if(!png_image_begin_read_from_file(&tileset, tileset_filename)) {
    fprintf(stderr, "Could not read file `%s`: %s\n", tileset_filename, tileset.message);
    abort();
  }
  tileset.format = PNG_FORMAT_RGBA;
  printf("Width: %d, Height %d\n", 4 * tileset.width, tileset.height);
  uint32_t *tileset_pixels = (uint32_t *) std::malloc(sizeof(uint32_t) * tileset.width * tileset.height);
  if(!png_image_finish_read(&tileset, nullptr, tileset_pixels, 0, nullptr)) {
    fprintf(stderr, "libpng pooped itself: %s\n", tileset.message);
    abort();
  }

  SDL_Surface *tileset_surface = sdl(SDL_CreateRGBSurfaceFrom(tileset_pixels,
                                                              tileset.width,
                                                              tileset.height,
                                                              32,
                                                              4 * tileset.width,
                                                              0x000000ff,
                                                              0x0000ff00,
                                                              0x00ff0000,
                                                              0xff000000));

  SDL_Texture *tileset_texture = sdl(SDL_CreateTextureFromSurface(renderer, tileset_surface));

  Tile_Texture wall_texture = {
    .srcrect = {120, 128, 16, 16},
    .texture = tileset_texture
  };

  bool quit = false;
  while (!quit) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: {
          quit = true;
        }
        break;
      }
    }

    sdl(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255));
    sdl(SDL_RenderClear(renderer));

    // sdl(SDL_RenderCopy(renderer,
    //                    tileset_texture,
    //                    &tileset_surface->clip_rect,
    //                    &tileset_surface->clip_rect));

    render_level(renderer, wall_texture);

    SDL_RenderPresent(renderer);
  }

  SDL_Quit();
  return 0;
}