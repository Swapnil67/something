#include<cassert>
#include<cstdio>
#include<cstdlib>

#include<algorithm>

#include<png.h>
#include<SDL_ttf.h>
#include<SDL.h>

#include "vec.hpp"

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
constexpr int TILE_SIZE_SQR = TILE_SIZE * TILE_SIZE;

enum class Tile {
  Empty = 0,
  Wall
};

constexpr int LEVEL_WIDTH = 10;
constexpr int LEVEL_HEIGHT = 10;
constexpr SDL_Rect level_boundary = {0, 0, LEVEL_WIDTH *TILE_SIZE, LEVEL_HEIGHT *TILE_SIZE};
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
  Vec2i vel;
  // int dx, dy;
};

static inline
bool is_not_oob(Vec2i p) {
  return 0 <= p.x && p.x < LEVEL_WIDTH && 0 <= p.y && p.y < LEVEL_HEIGHT;
}

bool is_tile_empty(Vec2i p) {
  return !is_not_oob(p) || level[p.y][p.x] == Tile::Empty;
}

static inline
int sqr_dist(Vec2i p0, Vec2i p1) {
  auto d = p0 - p1;
  return (d.x * d.x) + (d.y * d.y);
}

void resolve_point_collision(Vec2i *p) {
  assert(p);

  // * Calculates which tile player is currently standing
  const auto tile = *p / TILE_SIZE;
  // printf("Position : %d\t%d\n", p->x, p->y);
  // printf("Tile x : %d\t Tile y : %d\n", tile.x, tile.y);

  // * Out of bound [No Tile To Walk]
  if (is_tile_empty(tile)) {
    return;
  }

  // * Calculates the tile hitbox points
  const auto p0 = tile * TILE_SIZE;
  const auto p1 = (tile + 1) * TILE_SIZE;
  // printf("%d\t%d\t%d\t%d\n", p0.x, p1.x, p0.y, p1.y);

  struct Side {
    int d;
    Vec2i np;     // * neighbor position
    Vec2i nd;     // * neighbor direction
    int dd;
  };

  // * distance x and y
  Side sides[] = {
      {sqr_dist({p0.x, 0}, {p->x, 0}), {p0.x, p->y}, {-1, 0}, TILE_SIZE_SQR},            // * Left side
      {sqr_dist({p1.x, 0}, {p->x, 0}), {p1.x, p->y}, {1, 0}, TILE_SIZE_SQR},             // * Right side
      {sqr_dist({0, p0.y}, {0, p->y}), {p->x, p0.y}, {0, -1}, TILE_SIZE_SQR},            // * Top side
      {sqr_dist({0, p1.y}, {0, p->y}), {p->x, p1.y}, {0, 1}, TILE_SIZE_SQR},             // * Bottom side
      {sqr_dist({p0.x, p0.y}, {p->x, p->y}), {p0.x, p0.y}, {-1, -1}, TILE_SIZE_SQR * 2}, // * Top left
      {sqr_dist({p1.x, p0.y}, {p->x, p->y}), {p1.x, p0.y}, {1, -1}, TILE_SIZE_SQR * 2},  // * Top right
      {sqr_dist({p0.x, p1.y}, {p->x, p->y}), {p0.x, p1.y}, {-1, 1}, TILE_SIZE_SQR * 2},  // * Bottom left
      {sqr_dist({p1.x, p1.y}, {p->x, p->y}), {p1.x, p1.y}, {1, 1}, TILE_SIZE_SQR * 2},   // * Bottom right
  };

  constexpr int SIDES_COUNT = sizeof(sides) / sizeof(sides[0]);

  // * Find which side is closest to player movement
  int closest = -1;
  for (int current = 0; current < SIDES_COUNT; ++current) {
      // printf("current %d\n", current);
    // * Find closes empty towards the current side of direction
    for (int i = 1;
         !is_tile_empty(tile + sides[current].nd * i);
         ++i)
    {
      // printf("sides[current].d %d\n", sides[current].d);
      sides[current].d +=  sides[current].dd;
    }
    if (closest < 0 || sides[closest].d >= sides[current].d) {
      closest = current;
    }
  }
  // printf("------- closest %d\n", closest);
  *p = sides[closest].np;
  // printf("Position : %d\t%d\n", p->x, p->y);

}

// * Resolves player collision
void resolve_player_collision(Player *player) {
  assert(player);

  // * Player hitbox points
  Vec2i p0 = vec2(player->hitbox.x, player->hitbox.y);
  Vec2i p1 = p0 + vec2(player->hitbox.w, player->hitbox.h);

  // printf("%d\t%d\t%d\t%d\n", p0.x, p1.x, p0.y, p1.y);
  // * 0       48       211      259
  Vec2i mesh[] = {
    p0,
    {p1.x, p0.y},
    {p0.x, p1.y},
    p1
  };

  constexpr int MESH_COUNT = sizeof(mesh) / sizeof(mesh[0]);
  for (int i = 0; i < MESH_COUNT; ++i) {
    Vec2i t = mesh[i];
    resolve_point_collision(&t);

    // * Snaps the player to proper position & resolves the collision
    Vec2i d = t - mesh[i];

    constexpr int IMPACT_THRESHOLD = 5;
    if (std::abs(d.y) >= IMPACT_THRESHOLD)
      player->vel.y = 0;
    if (std::abs(d.x) >= IMPACT_THRESHOLD)
      player->vel.x = 0;

    for (int j = 0; j < MESH_COUNT; ++j) {
      mesh[j] += d;
    }
  }

  static_assert(MESH_COUNT >= 1);
  player->hitbox.x = mesh[0].x;
  player->hitbox.y = mesh[0].y;
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

void displayf(SDL_Renderer *renderer, TTF_Font *font, SDL_Color color, Vec2i p, const char *format, ...) {
  va_list args;
  va_start(args, format);

  char text[256];
  vsnprintf(text, sizeof(text), format, args);

  SDL_Texture *texture = render_text_as_texture(renderer, font, text, color);
  render_texture(renderer, texture, p.x, p.y);
  SDL_DestroyTexture(texture);

  va_end(args);
}

int main2() {
  Vec2i p = vec2(48, 259);
  resolve_point_collision(&p);
  return 0;
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
  player.hitbox = {0, 0, walking_frame_size, walking_frame_size};

  stec(TTF_Init());
  constexpr int DEBUG_FONT_SIZE = 18;
  TTF_Font *debug_font = stec(TTF_OpenFont("assets/Comic-Sans-MS.ttf", DEBUG_FONT_SIZE));

  int ddy = 1; // * gravity
  bool quit = false;
  bool debug = false;
  Animation *current = &idle;
  const Uint8 *keyboard = SDL_GetKeyboardState(NULL);
  SDL_RendererFlip player_dir = SDL_FLIP_NONE; 

  constexpr int PLAYER_SPEED = 4;
  constexpr int COLLISION_PROBE_SIZE = 10;
  SDL_Rect collision_probe = {};
  Vec2i mouse_position = {};
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
              player.vel.y = -20;
            } break;
            case SDLK_q: {
              debug = !debug;
            } break;
            case SDLK_r: {
              player.hitbox.x = 0;
              player.hitbox.y = 0;
              player.vel.y = 0;
            } break;
          }
        } break;
        case SDL_MOUSEMOTION: {
          Vec2i p = {event.motion.x, event.motion.y};
          resolve_point_collision(&p);
          collision_probe = {
              p.x - COLLISION_PROBE_SIZE,
              p.y - COLLISION_PROBE_SIZE,
              COLLISION_PROBE_SIZE * 2,
              COLLISION_PROBE_SIZE * 2};
          tile_rect = {
              event.motion.x / TILE_SIZE * TILE_SIZE,
              event.motion.y / TILE_SIZE * TILE_SIZE,
              TILE_SIZE,
              TILE_SIZE};
          mouse_position = {event.motion.x, event.motion.y};
        } break;
        case SDL_MOUSEBUTTONDOWN: {
          if(debug) {
            Vec2i tile = vec2(event.button.x, event.button.y) / TILE_SIZE;
            if(is_not_oob(tile)) {
              if(level[tile.y][tile.x] == Tile::Empty) {
                level[tile.y][tile.x] = Tile::Wall;
              }
              else {
                level[tile.y][tile.x] = Tile::Empty;
              }
            }
          }
        }
      }
    }

    if(keyboard[SDL_SCANCODE_D]) {
      player.vel.x = PLAYER_SPEED;
      current = &walking;
      player_dir = SDL_FLIP_NONE;
    } else if(keyboard[SDL_SCANCODE_A]) {
      player.vel.x = -PLAYER_SPEED;
      current = &walking;
      player_dir = SDL_FLIP_HORIZONTAL;
    } else {
      player.vel.x = 0;
      current = &idle;
    }

    player.vel.y += ddy;
    player.hitbox.x += player.vel.x;
    player.hitbox.y += player.vel.y;

    resolve_player_collision(&player);

    sec(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255));
    sec(SDL_RenderClear(renderer));

    render_level(renderer, wall_texture);
    render_animation(renderer, *current, player.hitbox, player_dir);

    if(debug) {
      sec(SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255));
      sec(SDL_RenderDrawRect(renderer, &player.hitbox));

      sec(SDL_RenderFillRect(renderer, &collision_probe));
      sec(SDL_RenderDrawRect(renderer, &tile_rect));
      sec(SDL_RenderDrawRect(renderer, &level_boundary));

      const Uint32 t = SDL_GetTicks() - begin;
      const Uint32 fps = t ? 1000 / t : 0;
      constexpr int PADDING = 10;
      // displayf(renderer, debug_font, {255, 0, 0, 255}, vec2(PADDING, PADDING), "FPS: %d", fps);
      displayf(renderer, debug_font, {255, 0, 0, 255}, vec2(PADDING, PADDING * 4), "Mouse Position (%d, %d)", mouse_position.x, mouse_position.y);
      displayf(renderer, debug_font, {255, 0, 0, 255}, vec2(PADDING, PADDING * 8), "Collision Porbe (%d, %d)", collision_probe.x, collision_probe.y);
    }
    
    SDL_RenderPresent(renderer);

    const Uint32 dt = SDL_GetTicks() - begin;
    // printf("%d\n", dt);
    update_animation(current, dt);
  }
  SDL_Quit();
  return 0;
}