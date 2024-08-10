#include<cstdio>
#include<cstdlib>
#include<png.h>

#include<SDL.h>

int sec(int code) {
  if(code < 0) {
    fprintf(stderr, "SDL popped itself: %s\n", SDL_GetError());
    abort();
  }
  return code;
}

template <typename T>
T *sec(T *ptr) {
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
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall}};

struct Sprite {
  SDL_Rect srcrect;
  SDL_Texture *texture;
};

void render_sprite(SDL_Renderer *renderer, Sprite texture, SDL_Rect destrect) {
  sec(SDL_RenderCopy(renderer,
                     texture.texture,
                     &texture.srcrect,
                     &destrect));
}

void render_level(SDL_Renderer *renderer, Sprite wall_texture) {
  for (int y = 0; y < LEVEL_HEIGHT; ++y) {
    for (int x = 0; x < LEVEL_WIDTH; ++x) {
      switch(level[y][x]) {
        case Tile::Empty: {
           
        } break;
        case Tile::Wall: {
          sec(SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255));
          SDL_Rect destrect = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
          render_sprite(renderer, wall_texture, destrect);
        }
      }
    }
  }
}

// * Create a image texture from png image
SDL_Texture *load_texture_from_png_file(SDL_Renderer *renderer, const char *image_filename) {
  // * Reading png into buffer
  png_image image;
  memset(&image, 0, sizeof(image));
  image.version = PNG_IMAGE_VERSION;
  if(!png_image_begin_read_from_file(&image, image_filename)) {
    fprintf(stderr, "Could not read file `%s`: %s\n", image_filename, image.message);
    abort();
  }
  image.format = PNG_FORMAT_RGBA;
  // printf("Width: %d, Height %d\n", image.width, image.height);
  uint32_t *image_pixels = (uint32_t *) std::malloc(sizeof(uint32_t) * image.width * image.height);
  if(!png_image_finish_read(&image, nullptr, image_pixels, 0, nullptr)) {
    fprintf(stderr, "libpng pooped itself: %s\n", image.message);
    abort();
  }

  SDL_Surface *image_surface = sec(SDL_CreateRGBSurfaceFrom(image_pixels,
                                                              image.width,
                                                              image.height,
                                                              32,
                                                              4 * image.width,
                                                              0x000000ff,
                                                              0x0000ff00,
                                                              0x00ff0000,
                                                              0xff000000));

  SDL_Texture *image_texture = sec(SDL_CreateTextureFromSurface(renderer, image_surface));
  SDL_FreeSurface(image_surface);
  return image_texture;
}

struct Animation {
  Sprite *frames;
  size_t   frame_count;
  size_t   frame_current;
  uint32_t frame_duration;
  uint32_t frame_cooldown;
};

static inline
void render_animation(SDL_Renderer *renderer, Animation animation, SDL_Rect dstrect) {
  render_sprite(renderer, animation.frames[animation.frame_current % animation.frame_count], dstrect);
}

void update_animation(Animation *animation, float dt) {
    if(dt < animation->frame_cooldown) {
      animation->frame_cooldown -= dt;
    } else {
      animation->frame_current = (animation->frame_current + 1) % animation->frame_count;
      animation->frame_cooldown = animation->frame_duration;
    }
}

int main() {
  sec(SDL_Init(SDL_INIT_VIDEO));

  // * Create a SDL window
  SDL_Window *window = sec(SDL_CreateWindow("Some Game", 0, 0, 800, 600, SDL_WINDOW_RESIZABLE));

  // * Create a SDL renderer
  SDL_Renderer *renderer = sec(SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED));

  // * Get tile texture from png
  SDL_Texture *tileset_texture = load_texture_from_png_file(renderer, "fantasy_tiles.png");

  Sprite wall_texture = {
    .srcrect = {120, 128, 16, 16},
    .texture = tileset_texture
  };

  // * Get the walking texture from png
  SDL_Texture *walking_texture = load_texture_from_png_file(renderer, "walking-12px.png");

  constexpr int walking_frame_size = 48;
  constexpr int walking_frame_count = 4;
  Sprite walking_frames[walking_frame_count];

  for (int i = 0; i < walking_frame_count; ++i) {
    walking_frames[i].srcrect = {
        i * walking_frame_size,
        0,
        walking_frame_size,
        walking_frame_size};
    walking_frames[i].texture = walking_texture;
  }

  // * Walking Animation
  Animation walking = {};
  walking.frames = walking_frames;
  walking.frame_count = 4;
  walking.frame_duration = 200;
  
  // * Idle Animation
  Animation idle = {};
  idle.frames = walking_frames + 2; // * Pointer Arithmentic
  idle.frame_count = 1;
  idle.frame_duration = 200;

  Animation *current = &idle;

  int x = 0;
  bool quit = false;
  const Uint8 *keyboard = SDL_GetKeyboardState(NULL);
  while (!quit) {
    const Uint32 begin = SDL_GetTicks();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: {
          quit = true;
        }
        break;
      }
    }

    if(keyboard[SDL_SCANCODE_D]) {
      x += 1;
      current = &walking;
    } else if(keyboard[SDL_SCANCODE_A]) {
      x -= 1;
      current = &walking;
    } else {
      current = &idle;
    }

    sec(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255));
    sec(SDL_RenderClear(renderer));

    render_level(renderer, wall_texture);
    SDL_Rect destrect = {x, 4 * TILE_SIZE - walking_frame_size, walking_frame_size, walking_frame_size};
    render_animation(renderer, *current, destrect);
    SDL_RenderPresent(renderer);

    const Uint32 dt = SDL_GetTicks() - begin;
    update_animation(current, dt);
  }
  SDL_Quit();
  return 0;
}