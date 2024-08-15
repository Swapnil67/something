#include<cassert>
#include<cstdio>
#include<cstdlib>

#include<algorithm>

#include<png.h>
#include<SDL_ttf.h>
#include<SDL.h>

template <typename T>
T *stec(T *ptr) {
  if(ptr == nullptr) {
    fprintf(stderr, "SDL_ttf popped itself: %s\n", TTF_GetError());
    abort();
  }
  return ptr;
}

void stec(int code) {
  if(code < 0) {
    fprintf(stderr, "SDL_ttf popped itself: %s\n", TTF_GetError());
    abort();
  }
}

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

constexpr int LEVEL_WIDTH = 6;
constexpr int LEVEL_HEIGHT = 5;
Tile level[LEVEL_HEIGHT][LEVEL_WIDTH] = {
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Wall, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Wall, Tile::Wall, Tile::Wall},
    {Tile::Wall, Tile::Wall, Tile::Empty, Tile::Wall, Tile::Wall, Tile::Empty}};

struct Sprite {
  SDL_Rect srcrect;
  SDL_Texture *texture;
};

void render_sprite(SDL_Renderer *renderer, Sprite texture, SDL_Rect destrect, SDL_RendererFlip flip = SDL_FLIP_NONE) {
  sec(SDL_RenderCopyEx(renderer,
                       texture.texture,
                       &texture.srcrect,
                       &destrect, 0.0, nullptr, flip));
}

void render_level(SDL_Renderer *renderer, Sprite wall_texture) {
  for (int y = 0; y < LEVEL_HEIGHT; ++y) {
    for (int x = 0; x < LEVEL_WIDTH; ++x) {
      switch(level[y][x]) {
        case Tile::Empty: {
           // * Do nothing
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
  // uint32_t *image_pixels = (uint32_t *) std::malloc(sizeof(uint32_t) * image.width * image.height);
  uint32_t *image_pixels = new uint32_t[image.width * image.height];
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

static inline void render_animation(SDL_Renderer *renderer, Animation animation, SDL_Rect dstrect, SDL_RendererFlip flip = SDL_FLIP_NONE) {
  render_sprite(renderer,
                animation.frames[animation.frame_current % animation.frame_count],
                dstrect,
                flip);
}

void update_animation(Animation *animation, uint32_t dt) {
  if (dt < animation->frame_cooldown) {
    animation->frame_cooldown -= dt;
  } else {
    animation->frame_current = (animation->frame_current + 1) % animation->frame_count;
    animation->frame_cooldown = animation->frame_duration;
  }
}

struct Player {
  SDL_Rect hitbox;
  int dx, dy;
};

static inline
bool is_not_oob(int x, int y) {
  return 0 <= x && x < LEVEL_WIDTH && 0 <= y && y < LEVEL_HEIGHT;
}

bool is_tile_empty(int x, int y) {
  return !is_not_oob(x, y) || level[y][x] == Tile::Empty;
}

static inline
int sqr_dist(int x0, int y0, int x1, int y1) {
  int dx = x0 - x1;
  int dy = y0 - y1;
  return (dx * dx) + (dy * dy);
}

void resolve_point_collision(int *x, int *y) {
  assert(x);
  assert(y);

  // * Calculates which tile player is currently standing
  const int tile_x = *x / TILE_SIZE;
  const int tile_y = *y / TILE_SIZE;

  // * Out of bound [No Tile To Walk]
  if (!is_not_oob(tile_x, tile_y) || level[tile_y][tile_x] == Tile::Empty) {
    return;
  }

  // printf("-------- *x : %d & *Y : %d \n", *x, *y);
  // printf(" tile_x: %d & tile_y: %d \n", tile_x, tile_y);

  // * Calculates the tile hitbox points
  const int tx0 = tile_x * TILE_SIZE;
  const int tx1 = (tile_x + 1) * TILE_SIZE;
  const int ty0 = tile_y * TILE_SIZE;
  const int ty1 = (tile_y + 1) * TILE_SIZE;

  // * tx0 = 0, tx1 = 64
  // * ty0 = 256, ty1 = 320

  // printf("-------- tx0 : %d & tx1 : %d \n", tx0, tx1);
  // printf("-------- ty0 : %d & ty1 : %d \n", ty0, ty1);

  struct Side {
    int d;
    int x;
    int y;
    int dx;
    int dy;
    int dd;
  };

  // * distance x and y
  Side sides[] = {
    {sqr_dist(tx0, 0, *x, 0), tx0, *y, -1, 0, TILE_SIZE * TILE_SIZE},          // * Left side
    {sqr_dist(tx1, 0, *x, 0), tx1, *y, 1, 0, TILE_SIZE * TILE_SIZE},           // * Right side
    {sqr_dist(0, ty0, 0, *y), *x, ty0, 0, -1, TILE_SIZE * TILE_SIZE},          // * Top side
    {sqr_dist(0, ty1, 0, *y), *x, ty1, 0, 1, TILE_SIZE * TILE_SIZE},           // * Bottom side
    {sqr_dist(tx0, ty0, *x, *y), tx0, ty0, -1, -1, TILE_SIZE * TILE_SIZE * 2}, // * Top left
    {sqr_dist(tx1, ty0, *x, *y), tx1, ty0, 1, -1, TILE_SIZE * TILE_SIZE * 2},  // * Top right
    {sqr_dist(tx0, ty1, *x, *y), tx0, ty1, -1, 1, TILE_SIZE * TILE_SIZE * 2},  // * Bottom left
    {sqr_dist(tx1, ty1, *x, *y), tx1, ty1, 1, 1, TILE_SIZE * TILE_SIZE * 2},   // * Bottom right
  };

  constexpr int SIDES_COUNT = sizeof(sides) / sizeof(sides[0]);
  // for (int i = 0; i < SIDES_COUNT; ++i) {
  //    printf("-------- d : %d\n", sides[i].d);
  // }
  // printf("==============================");

  // * Find which side is closest to player movement
  int closest = -1;
  for (int current = 0; current < SIDES_COUNT; ++current) {
    for (int i = 1;
         !is_tile_empty(tile_x + sides[current].dx * i,
                        tile_y + sides[current].dy * i);
         ++i)
    {
      sides[current].d +=  sides[current].dd;
    }
    if (closest < 0 || sides[closest].d >= sides[current].d) {
      closest = current;
    }
  }
  // printf("------- closest %d\n", closest);
  *x = sides[closest].x;
  *y = sides[closest].y;
  // printf("-------- *x : %d & *Y : %d \n", *x, *y);
}

// * Resolves player collision
void resolve_player_collision(Player *player) {
  assert(player);

  // * Player hitbox points
  int x0 = player->hitbox.x;
  int y0 = player->hitbox.y;
  int x1 = player->hitbox.x + player->hitbox.w - 1;
  int y1 = player->hitbox.y + player->hitbox.h - 1;

  // printf("%d\t%d\t%d\t%d\n", x0, x1, y0, y1);

  int mesh[][2] = {
    {x0, y0},
    {x1, y0},
    {x0, y1},
    {x1, y1},
  };

  /*
  * mesh = {
  *   { 0, 210 }
  *   { 47, 210 }
  *   { 0, 250 }
  *   { 47, 250 }
  * }
  */

  constexpr int MESH_COUNT = sizeof(mesh) / sizeof(mesh[0]);
  constexpr int X = 0;
  constexpr int Y = 1;

  for (int i = 0; i < MESH_COUNT; ++i) {
    int tx = mesh[i][X];
    int ty = mesh[i][Y];
    resolve_point_collision(&tx, &ty);

    // * Snaps the player to proper position & resolves the collision
    int dx = tx - mesh[i][X];
    int dy = ty - mesh[i][Y];

    constexpr int IMPACT_THRESHOLD = 5;
    if (std::abs(dx) >= IMPACT_THRESHOLD)
      player->dx = 0;
    if (std::abs(dy) >= IMPACT_THRESHOLD)
      player->dy = 0;

    for (int j = 0; j < MESH_COUNT; ++j) {
      mesh[j][X] += dx;
      mesh[j][Y] += dy;
    }
  }

  static_assert(MESH_COUNT >= 1);
  player->hitbox.x = mesh[0][X];
  player->hitbox.y = mesh[0][Y];
} 

SDL_Texture *render_text_as_texture(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color color) {
  SDL_Surface *surface = stec(TTF_RenderText_Blended(font, text, color));
  SDL_Texture *texture = stec(SDL_CreateTextureFromSurface(renderer, surface));
  SDL_FreeSurface(surface);
  return texture;
}

void render_texture(SDL_Renderer *renderer, SDL_Texture *texture, int x, int y) {
  // * get the width & height of texture
  int w, h;
  sec(SDL_QueryTexture(texture, NULL, NULL, &w, &h));
  // * box from texture
  SDL_Rect srcrect = {0, 0, w, h};

  // * box to destination
  SDL_Rect dstrect = {x, y, w, h};
  sec(SDL_RenderCopy(renderer, texture, &srcrect, &dstrect));
}

constexpr size_t DIGITS_COUNT = 10;
SDL_Texture *digits_textures[DIGITS_COUNT];

void render_digits_of_number(SDL_Renderer *renderer, uint64_t number, int x, int y) {
  if(number == 0) {
    static_assert(DIGITS_COUNT > 1);
    render_texture(renderer, digits_textures[0], x, y);
    return;
  }

  while (number != 0) {
    int d = number % 10; // * gives the last digit
    if (d < 0 || d > 9) {
      printf("%d\n", d);
    }
    // * pre-rendered glyphs
    int w;
    sec(SDL_QueryTexture(digits_textures[d], NULL, NULL, &w, NULL));
    render_texture(renderer, digits_textures[d], x, y);
    x -= w;
    number = number / 10;
  }
}

void displayf(SDL_Renderer *renderer, TTF_Font *font, SDL_Color color, int x, int y, const char *format, ...) {
  va_list args;
  va_start(args, format);

  char text[256];
  vsnprintf(text, sizeof(text), format, args);

  SDL_Texture *texture = render_text_as_texture(renderer, font, text, color);
  render_texture(renderer, texture, x, y);
  SDL_DestroyTexture(texture);

  va_end(args);
}

int main() {
  sec(SDL_Init(SDL_INIT_VIDEO));

  // * Create a SDL window
  SDL_Window *window = sec(SDL_CreateWindow("Something", 0, 0, 800, 600, SDL_WINDOW_RESIZABLE));

  // * Create a SDL renderer
  SDL_Renderer *renderer = sec(SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED));

  // * Get tile texture from png
  SDL_Texture *tileset_texture = load_texture_from_png_file(renderer, "assets/fantasy_tiles.png");

  Sprite wall_texture = {
    .srcrect = {120, 128, 16, 16},
    .texture = tileset_texture
  };

  // * Get the walking texture from png
  SDL_Texture *walking_texture = load_texture_from_png_file(renderer, "assets/walking-12px.png");

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
  walking.frame_duration = 150;
  
  // * Idle Animation
  Animation idle = {};
  idle.frames = walking_frames + 2; // * Pointer Arithmentic
  idle.frame_count = 1;
  idle.frame_duration = 100;

  // * Player
  Player player = {};
  player.dy = 0;
  player.hitbox = {0, 0, walking_frame_size, walking_frame_size};

  stec(TTF_Init());
  TTF_Font *font = stec(TTF_OpenFont("assets/Comic-Sans-MS.ttf", 69));

  int ddy = 1; // * gravity
  bool quit = false;
  bool debug = false;
  Animation *current = &idle;
  const Uint8 *keyboard = SDL_GetKeyboardState(NULL);
  SDL_RendererFlip player_dir = SDL_FLIP_NONE; 

  constexpr int PLAYER_SPEED = 4;
  constexpr int CURSOR_SIZE = 10;
  SDL_Rect cursor = {};
  SDL_Rect tile_rect = {};

  while (!quit) {
    const Uint32 begin = SDL_GetTicks();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: {
          quit = true;
        } break;
        case SDL_KEYDOWN: {
          switch (event.key.keysym.sym) {
            case SDLK_SPACE: {
              player.dy = -20;
            } break;
            case SDLK_q: {
              debug = !debug;
            } break;
            case SDLK_r: {
              player.hitbox.x = 0;
              player.hitbox.y = 0;
              player.dy = 0;
            } break;
          }
        } break;
        case SDL_MOUSEMOTION: {
          auto x = event.motion.x;
          auto y = event.motion.y;
          resolve_point_collision(&x, &y);
          cursor = {
              x - CURSOR_SIZE,
              y - CURSOR_SIZE,
              CURSOR_SIZE * 2,
              CURSOR_SIZE * 2};
          tile_rect = {
              event.motion.x / TILE_SIZE * TILE_SIZE,
              event.motion.y / TILE_SIZE * TILE_SIZE,
              TILE_SIZE,
              TILE_SIZE};
        } break;
      }
    }

    if(keyboard[SDL_SCANCODE_D]) {
      player.dx = PLAYER_SPEED;
      current = &walking;
      player_dir = SDL_FLIP_NONE;
    } else if(keyboard[SDL_SCANCODE_A]) {
      player.dx = -PLAYER_SPEED;
      current = &walking;
      player_dir = SDL_FLIP_HORIZONTAL;
    } else {
      player.dx = 0;
      current = &idle;
    }

    player.dy += ddy;
    player.hitbox.x += player.dx;
    player.hitbox.y += player.dy;

    resolve_player_collision(&player);


    sec(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255));
    sec(SDL_RenderClear(renderer));

    render_level(renderer, wall_texture);
    render_animation(renderer, *current, player.hitbox, player_dir);

    if(debug) {
      sec(SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255));
      sec(SDL_RenderDrawRect(renderer, &player.hitbox));

      sec(SDL_RenderFillRect(renderer, &cursor));
      sec(SDL_RenderDrawRect(renderer, &tile_rect));
    }
    
    // render_digits_of_number(renderer, -12345, 200, 0);
    displayf(renderer, font, {255, 255, 0, 255}, player.hitbox.x, 0, "Ticks: %dms", SDL_GetTicks() - begin);
    SDL_RenderPresent(renderer);

    const Uint32 dt = SDL_GetTicks() - begin;
    // printf("%d\n", dt);
    update_animation(current, dt);
  }
  SDL_Quit();
  return 0;
}