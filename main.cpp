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

const int TILE_SIZE = 64;
const int TILE_SIZE_SQR = TILE_SIZE * TILE_SIZE;

enum class Tile {
  Empty = 0,
  Wall
};

const int LEVEL_WIDTH = 10;
const int LEVEL_HEIGHT = 10;
const SDL_Rect level_boundary = {0, 0, LEVEL_WIDTH *TILE_SIZE, LEVEL_HEIGHT *TILE_SIZE};
Tile level[LEVEL_HEIGHT][LEVEL_WIDTH] = {
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
};

struct Sprite {
  SDL_Rect srcrect;
  SDL_Texture *texture;
};

void render_sprite(
    SDL_Renderer *renderer,
    Sprite texture,
    SDL_Rect destrect,
    SDL_RendererFlip flip = SDL_FLIP_NONE)
{
  sec(SDL_RenderCopyEx(renderer,
                       texture.texture,
                       &texture.srcrect,
                       &destrect, 0.0, nullptr, flip));
}

void render_sprite(
    SDL_Renderer *renderer,
    Sprite texture,
    Vec2i pos,
    SDL_RendererFlip flip = SDL_FLIP_NONE)
{
  SDL_Rect dstrect = {
      pos.x - texture.srcrect.w / 2, pos.y - texture.srcrect.h / 2,
      texture.srcrect.w, texture.srcrect.h};
  sec(SDL_RenderCopyEx(renderer,
                       texture.texture,
                       &texture.srcrect,
                       &dstrect, 0.0, nullptr, flip));
}

static inline
bool is_tile_inbounds(Vec2i p) {
  return 0 <= p.x && p.x < LEVEL_WIDTH && 0 <= p.y && p.y < LEVEL_HEIGHT;
}

bool is_tile_empty(Vec2i p) {
  return !is_tile_inbounds(p) || level[p.y][p.x] == Tile::Empty;
}

void render_level(SDL_Renderer *renderer, Sprite top_ground_texture, Sprite bottom_ground_texture) {
  for (int y = 0; y < LEVEL_HEIGHT; ++y) {
    for (int x = 0; x < LEVEL_WIDTH; ++x) {
      switch(level[y][x]) {
        case Tile::Empty: {
           // * Do nothing
        } break;
        case Tile::Wall: {
          if(is_tile_empty(vec2(x, y-1))) {
            SDL_Rect destrect = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
            render_sprite(renderer, top_ground_texture, destrect);
          }
          else {
            SDL_Rect destrect = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
            render_sprite(renderer, bottom_ground_texture, destrect);
          }
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
  Sprite    *frames;
  size_t    frame_count;
  size_t    frame_current;
  uint32_t  frame_duration;
  uint32_t  frame_cooldown;
};

static inline void render_animation(
    SDL_Renderer *renderer,
    Animation animation,
    Vec2i pos,
    SDL_RendererFlip flip = SDL_FLIP_NONE)
{
  render_sprite(renderer,
                animation.frames[animation.frame_current % animation.frame_count],
                pos,
                flip);
}
static inline void render_animation(
    SDL_Renderer *renderer,
    Animation animation,
    SDL_Rect dstrect,
    SDL_RendererFlip flip = SDL_FLIP_NONE)
{
  render_sprite(renderer,
                animation.frames[animation.frame_current % animation.frame_count],
                dstrect,
                flip);
}

void update_animation(Animation *animation, uint32_t dt) {
  if (dt < animation->frame_cooldown) {
    animation->frame_cooldown -= dt;
  } else {
    // * To keep the animation frames bounded to frame count [%]
    animation->frame_current = (animation->frame_current + 1) % animation->frame_count;
    animation->frame_cooldown = animation->frame_duration;
  }
}

enum class Entity_Dir
{
  Right = 0,
  Left
};

struct Entity {
  SDL_Rect texbox;
  SDL_Rect hitbox;
  Vec2i pos;
  Vec2i vel;

  Animation idle;
  Animation walking;
  Animation *current;

  Entity_Dir dir;
};

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

  // * Calculates the tile texbox points
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

  const int SIDES_COUNT = sizeof(sides) / sizeof(sides[0]);

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
void resolve_entity_collision(Entity *entity) {
  assert(entity);

  // * Entity texbox points
  Vec2i p0 = vec2(entity->hitbox.x, entity->hitbox.y) + entity->pos;
  Vec2i p1 = p0 + vec2(entity->hitbox.w, entity->hitbox.h);

  // printf("%d\t%d\t%d\t%d\n", p0.x, p1.x, p0.y, p1.y);
  // * 0       48       211      259
  Vec2i mesh[] = {
    p0,
    {p1.x, p0.y},
    {p0.x, p1.y},
    p1
  };

  const int MESH_COUNT = sizeof(mesh) / sizeof(mesh[0]);
  for (int i = 0; i < MESH_COUNT; ++i) {
    Vec2i t = mesh[i];
    resolve_point_collision(&t);

    // * Snaps the entity to proper position & resolves the collision
    Vec2i d = t - mesh[i];

    const int IMPACT_THRESHOLD = 5;
    if (std::abs(d.y) >= IMPACT_THRESHOLD)
      entity->vel.y = 0;
    if (std::abs(d.x) >= IMPACT_THRESHOLD)
      entity->vel.x = 0;

    for (int j = 0; j < MESH_COUNT; ++j) {
      mesh[j] += d;
    }
    entity->pos += d;
  }
} 

SDL_Rect entity_dstrect(const Entity entity) {
  SDL_Rect dstrect = {
      entity.texbox.x + entity.pos.x, entity.texbox.y + entity.pos.y, entity.texbox.w, entity.texbox.h};
  return dstrect;
}

SDL_Rect entity_hitbox(const Entity entity) {
  SDL_Rect hitbox = {
      entity.hitbox.x + entity.pos.x, entity.hitbox.y + entity.pos.y, entity.hitbox.w, entity.hitbox.h};
  return hitbox;
}

void render_entity(SDL_Renderer *renderer, const Entity entity) {
  const SDL_Rect dstrect = entity_dstrect(entity);
  const SDL_RendererFlip flip = entity.dir == Entity_Dir::Right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
  render_animation(renderer, *entity.current, dstrect, flip);
}

void update_entity(Entity *entity, Vec2i gravity, uint32_t dt) {
  assert(entity);

  // * Add Gravity
  entity->vel += gravity;

  // * Add Velocity
  entity->pos += entity->vel;

  // * Resolve Collision
  resolve_entity_collision(entity);

  update_animation(entity->current, dt);
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

const size_t DIGITS_COUNT = 10;
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

enum class Debug_Draw_State {
  Idle,
  Create,
  Delete
};

Animation load_spritesheet_animation(SDL_Renderer *renderer, size_t frame_count, uint32_t frame_duration, const char *spritesheet_filepath) {
  Animation result = {};
  result.frames = new Sprite[frame_count];
  result.frame_count = frame_count;
  result.frame_duration = frame_duration;
  SDL_Texture *spritesheet = load_texture_from_png_file(renderer, spritesheet_filepath);
  int spritesheet_w = 0;
  int spritesheet_h = 0;
  sec(SDL_QueryTexture(spritesheet, NULL, NULL, &spritesheet_w, &spritesheet_h));
  int sprite_w = spritesheet_w / frame_count;
  int sprite_h = spritesheet_h; // * Note We only handle horizontal spritesheet

  for (int i = 0; i < (int)frame_count; ++i) {
    result.frames[i].srcrect = {i * sprite_w, 0, sprite_w, sprite_h};
    result.frames[i].texture = spritesheet;
  }

  return result;
}

enum class Projectile_State {
  Ded = 0,
  Active,
  Poof
};

struct Projectile {
  Projectile_State state;
  Vec2i pos;
  Vec2i vel;
  Animation active_animation;
  Animation poof_animation;
  Entity_Dir dir;
};

const size_t projectiles_count = 69;
Projectile projectiles[projectiles_count] = {};

void init_projectiles(Animation active_animation, Animation poof_animation) {
  for (size_t i = 0; i < projectiles_count; ++i) {
    projectiles[i].active_animation = active_animation;
    projectiles[i].poof_animation = poof_animation;
  }
}

int count_alive_projectiles(void) {
  int res = 0;
  for (size_t i = 0; i < projectiles_count; ++i) {
    if(projectiles[i].state != Projectile_State::Ded) {
      res++;
    }
  }
  return res;
}

void spwan_projectiles(Vec2i pos, Vec2i vel, Entity_Dir dir) {
   for (size_t i = 0; i < projectiles_count; ++i) {
    // * Find the first one which is in ded state & activate it & return.
    if(projectiles[i].state == Projectile_State::Ded) {
      projectiles[i].state = Projectile_State::Active;
      projectiles[i].pos = pos;
      projectiles[i].vel = vel;
      projectiles[i].dir = dir;
      return; 
    }
  } 
}

void render_projectiles(SDL_Renderer *renderer) {
  for (size_t i = 0; i < projectiles_count; ++i) {
    const SDL_RendererFlip flip = projectiles[i].dir == Entity_Dir::Right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    switch (projectiles[i].state) {
      case Projectile_State::Active: {
        render_animation(
            renderer,
            projectiles[i].active_animation,
            projectiles[i].pos,
            flip);
      } break;
      case Projectile_State::Poof: {
        render_animation(
            renderer,
            projectiles[i].poof_animation,
            projectiles[i].pos,
            flip);
      } break;
      case Projectile_State::Ded: {
      } break;
    }
  }
}

void update_projectiles(uint32_t dt) {
   for (size_t i = 0; i < projectiles_count; ++i) {
    switch (projectiles[i].state) {
      case Projectile_State::Active: {
        update_animation(&projectiles[i].active_animation, dt);
        projectiles[i].pos += projectiles[i].vel;
        auto projectile_tile=projectiles[i].pos / TILE_SIZE;
        if (!is_tile_empty(projectile_tile) ||
            !is_tile_inbounds(projectile_tile))
        {
          projectiles[i].state = Projectile_State::Poof; 
          projectiles[i].poof_animation.frame_current = 0;
          // * Destory tile
          // level[projectile_tile.y][projectile_tile.x] = Tile::Empty;
        }
      } break;
      case Projectile_State::Poof: {
        update_animation(&projectiles[i].poof_animation, dt);
        if (projectiles[i].poof_animation.frame_current == (projectiles[i].poof_animation.frame_count - 1)) {
          projectiles[i].state = Projectile_State::Ded;
        }
      } break;
      case Projectile_State::Ded: {
      } break;
    }
  } 
}

void dump_level() {
  std::printf("{\n");
  for (int y = 0; y < LEVEL_HEIGHT; ++y) {
    std::printf("{");
    for (int x = 0; x < LEVEL_WIDTH; ++x) {
      switch(level[y][x]) {
        case Tile::Empty: {
          std::printf("Tile::Empty, ");
        } break;
        case Tile::Wall: {
          std::printf("Tile::Wall, ");
        } break;
      }
    }
    std::printf("},");
    std::printf("\n");
  }
  std::printf("};\n");
}

void entity_move(Entity *entity, int speed) {
  assert(entity);
  if (speed < 0) {
    entity->dir = Entity_Dir::Left;
  } else if (speed > 0) {
    entity->dir = Entity_Dir::Right;
  }
  entity->vel.x = speed;
  // printf("%d\t%d\n", entity->vel.x, entity->vel.y);
  entity->current = &entity->walking;
}

void entity_stop(Entity *entity) {
  assert(entity);
  entity->vel.x = 0;
  entity->current = &entity->idle;
}

void entity_shoot(Entity *entity) {
  assert(entity);
  if (entity->dir == Entity_Dir::Right) {
    spwan_projectiles(entity->pos, vec2(10, 0), entity->dir);
  } else {
    spwan_projectiles(entity->pos, vec2(-10, 0), entity->dir);
  }
}

int main() {
  sec(SDL_Init(SDL_INIT_VIDEO));

  // * Create a SDL window
  SDL_Window *window = sec(SDL_CreateWindow("Something", 0, 0, 800, 600, SDL_WINDOW_RESIZABLE));

  // * Create a SDL renderer
  SDL_Renderer *renderer = sec(SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED));

  // * Get tile texture from png
  SDL_Texture *tileset_texture = load_texture_from_png_file(renderer, "assets/fantasy_tiles.png");

  Sprite ground_grass_texture = {
    .srcrect = {120, 128, 16, 16},
    .texture = tileset_texture
  };

  Sprite ground_texture = {
    .srcrect = {120, 128 + 16, 16, 16},
    .texture = tileset_texture
  };

  const size_t plasma_pop_frame_count = 4;
  Animation plasma_pop_animation = load_spritesheet_animation(
      renderer, plasma_pop_frame_count, 70,
      "./assets/destroy-sheet.png");

  const size_t plasma_bolt_frame_count = 5;
  Animation plasma_bolt_animation = load_spritesheet_animation(
      renderer, plasma_bolt_frame_count, 70,
      "./assets/spark-sheet.png");

  init_projectiles(plasma_bolt_animation, plasma_pop_animation);

  // * Get the walking texture from png
  SDL_Texture *walking_texture = load_texture_from_png_file(renderer, "assets/walking-12px.png");

  const int PLAYER_SPEED = 4;
  const int walking_frame_size = 48;
  const int walking_frame_count = 4;
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
  idle.frames = walking_frames + 2;
  idle.frame_count = 1;
  idle.frame_duration = 100;

  // * Entity
  const int PLAYER_TEXBOX_SIZE = 48;
  const int PLAYER_HITBOX_SIZE = PLAYER_TEXBOX_SIZE - 10;
  SDL_Rect texbox = {
      -(PLAYER_TEXBOX_SIZE / 2), -(PLAYER_TEXBOX_SIZE / 2),
      PLAYER_TEXBOX_SIZE, PLAYER_TEXBOX_SIZE};
  // printf("%d\t%d\t%d\t%d\n", texbox.x, texbox.x, texbox.y, texbox.y);
 
  SDL_Rect hitbox = {
      -(PLAYER_HITBOX_SIZE / 2), -(PLAYER_HITBOX_SIZE / 2),
      PLAYER_HITBOX_SIZE, PLAYER_HITBOX_SIZE};

  Entity player = {};
  player.texbox = texbox;
  player.hitbox = hitbox;

  player.idle = idle;
  player.walking = walking;
  player.current = &player.idle;

  Entity supposed_enemy = {};
  supposed_enemy.texbox = texbox;
  supposed_enemy.hitbox = hitbox;
  supposed_enemy.walking = walking;
  supposed_enemy.idle = idle;
  supposed_enemy.current = &supposed_enemy.idle;
  static_assert(LEVEL_WIDTH >= 2);
  supposed_enemy.pos = vec2(LEVEL_WIDTH - 2, 0) * TILE_SIZE;
  supposed_enemy.dir = Entity_Dir::Left;

  stec(TTF_Init());
  const int DEBUG_FONT_SIZE = 18;
  TTF_Font *debug_font = stec(TTF_OpenFont("assets/Comic-Sans-MS.ttf", DEBUG_FONT_SIZE));

  Vec2i gravity = {0, 1};
  bool quit = false;
  bool debug = false;
  const Uint8 *keyboard = SDL_GetKeyboardState(NULL);

  const int COLLISION_PROBE_SIZE = 10;
  SDL_Rect collision_probe = {};
  Vec2i mouse_position = {};
  SDL_Rect tile_rect = {};
  Debug_Draw_State state = Debug_Draw_State::Idle;

  Uint32 fps = 0;
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
            case SDLK_e: {
              entity_shoot(&player);
            } break;
            case SDLK_r: {
              player.pos = vec2(0, 0);
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

          Vec2i tile = vec2(event.button.x, event.button.y) / TILE_SIZE;
          switch(state) {
            case Debug_Draw_State::Create: {
              if (is_tile_inbounds(tile))
                level[tile.y][tile.x] = Tile::Wall; 
            } break;
            case Debug_Draw_State::Delete: {
              if (is_tile_inbounds(tile))
                level[tile.y][tile.x] = Tile::Empty; 
            } break;
            default: {}
          }

        } break;
        case SDL_MOUSEBUTTONDOWN: {
          if(debug) {
            Vec2i tile = vec2(event.button.x, event.button.y) / TILE_SIZE;
            if(is_tile_inbounds(tile)) {
              if(level[tile.y][tile.x] == Tile::Empty) {
                state = Debug_Draw_State::Create;
                level[tile.y][tile.x] = Tile::Wall; 
              }
              else {
                state = Debug_Draw_State::Delete;
                level[tile.y][tile.x] = Tile::Empty; 
              }
            }
          }
        } break;
        case SDL_MOUSEBUTTONUP: {
          state = Debug_Draw_State::Idle;
        } break;
      }
    }

    // entity_move(&supposed_enemy, -1);
    entity_shoot(&supposed_enemy);
    if(keyboard[SDL_SCANCODE_D]) {
      entity_move(&player, PLAYER_SPEED);
    } else if(keyboard[SDL_SCANCODE_A]) {
      entity_move(&player, -PLAYER_SPEED);
    } else {
      entity_stop(&player);
    }

    sec(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255));
    sec(SDL_RenderClear(renderer));

    render_level(renderer, ground_grass_texture, ground_texture);
    render_entity(renderer, player);
    render_entity(renderer, supposed_enemy);
    render_projectiles(renderer);

    if(debug) {
      sec(SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255));

      auto dstrect = entity_dstrect(player);
      sec(SDL_RenderDrawRect(renderer, &dstrect));

      sec(SDL_RenderFillRect(renderer, &collision_probe));
      sec(SDL_RenderDrawRect(renderer, &tile_rect));
      sec(SDL_RenderDrawRect(renderer, &level_boundary));

      const Uint32 t = SDL_GetTicks() - begin;
      const Uint32 fps_snapshot = t ? 1000 / t : 0;
      fps = (fps + fps_snapshot) / 2;
      const int PADDING = 10;
      displayf(renderer, debug_font, {255, 0, 0, 255}, vec2(PADDING, PADDING), "FPS: %d", fps);
      displayf(renderer, debug_font, {255, 0, 0, 255}, vec2(PADDING, PADDING * 4), "Mouse Position (%d, %d)", mouse_position.x, mouse_position.y);
      displayf(renderer, debug_font, {255, 0, 0, 255}, vec2(PADDING, PADDING * 8), "Collision Porbe (%d, %d)", collision_probe.x, collision_probe.y);
      displayf(renderer, debug_font, {255, 0, 0, 255}, vec2(PADDING, PADDING * 12), "Projectiles: %d", count_alive_projectiles());

      sec(SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255));
      auto hitbox = entity_hitbox(player);
      sec(SDL_RenderDrawRect(renderer, &hitbox));
    }
    
    SDL_RenderPresent(renderer);

    const Uint32 dt = SDL_GetTicks() - begin;
    // printf("%d\n", dt);

    update_entity(&player, gravity, dt);
    update_entity(&supposed_enemy, gravity, dt);
    update_projectiles(dt);
  }
  SDL_Quit();
  // dump_level();
  return 0;
}
