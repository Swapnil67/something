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

// TODO: Turn displayf into println style
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


struct RGBA32 {
  uint8_t r,g,b,a;
};

RGBA32 decode_pixel(Uint32 pixel, SDL_PixelFormat *format) {
  RGBA32 result = {};
  // * parsing red component
  result.r = ((pixel & format->Rmask) >> format->Rshift) << format->Rloss;
  // * parsing green component
  result.g = ((pixel & format->Gmask) >> format->Gshift) << format->Gloss;
  // * parsing blue component
  result.b = ((pixel & format->Bmask) >> format->Bshift) << format->Bloss;
  // * parsing alpha component
  result.a = ((pixel & format->Amask) >> format->Ashift) << format->Aloss;
  return result;
}

Uint32 encode_pixel(RGBA32 pixel, SDL_PixelFormat *format) {
  Uint32 result = 0;
  result |= (pixel.r >> format->Rloss) << format->Rshift;
  result |= (pixel.g >> format->Gloss) << format->Gshift;
  result |= (pixel.b >> format->Bloss) << format->Bshift;
  result |= (pixel.a >> format->Aloss) << format->Ashift;
  return result;
}

void enemy_spritesheet(SDL_Surface *spritesheet_surface) {
  assert(spritesheet_surface);
  assert(spritesheet_surface->format);
  assert(spritesheet_surface->format->BytesPerPixel == 4);
  printf("%d %d %d %d\n", spritesheet_surface->h, spritesheet_surface->w, spritesheet_surface->pitch, spritesheet_surface->format->BytesPerPixel);
  for (int y = 0; y < spritesheet_surface->h; ++y) {
    for(int x = 0; x < spritesheet_surface->w; ++x) {

      const int pixel_index = y * spritesheet_surface->pitch + x * spritesheet_surface->format->BytesPerPixel;
      Uint8 *pixels = (Uint8 *)spritesheet_surface->pixels;
      Uint32 *pixel = (Uint32 *)(pixels + pixel_index);

      auto pixel_rgba32 = decode_pixel(*pixel, spritesheet_surface->format);
      pixel_rgba32.r = min((Uint32)pixel_rgba32.r + 200u, 255u);
      *pixel = encode_pixel(pixel_rgba32, spritesheet_surface->format);
    }
  }
}

enum class Debug_Draw_State {
  Idle = 0,
  Create,
  Delete
};

struct Game_State {
  Vec2i gravity;
  bool quit;
  Vec2i collision_probe;
  Vec2i mouse_position;
  Debug_Draw_State state;

  TTF_Font *debug_font;

  Sprite ground_grass_texture;
  Sprite ground_texture;
};

const int ENEMY_COUNT = 4;
const int ENEMY_ENTITY_IDX_OFFSET = 1;
const int PLAYER_ENTITY_IDX = 0;
const int PLAYER_SPEED = 4;

void render_debug_overlay(Game_State game_state, SDL_Renderer *renderer, Uint32 fps)
{
  sec(SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255));

  const int COLLISION_PROBE_SIZE = 10;
  const SDL_Rect collision_probe_rect = {
    game_state.collision_probe.x - COLLISION_PROBE_SIZE,
    game_state.collision_probe.y - COLLISION_PROBE_SIZE,
    COLLISION_PROBE_SIZE * 2,
    COLLISION_PROBE_SIZE * 2,
  };
  sec(SDL_RenderFillRect(renderer, &collision_probe_rect));

  const SDL_Rect tile_rect = {
    game_state.mouse_position.x / TILE_SIZE * TILE_SIZE,
    game_state.mouse_position.y / TILE_SIZE * TILE_SIZE,
    TILE_SIZE, TILE_SIZE
  };
  sec(SDL_RenderDrawRect(renderer, &tile_rect));

  // * Level Boundary
  const SDL_Rect level_boundary = {
    0, 0, LEVEL_WIDTH * TILE_SIZE, LEVEL_HEIGHT * TILE_SIZE
  };
  sec(SDL_RenderDrawRect(renderer, &level_boundary));

  const int PADDING = 10;
  // TODO fps rendering is broken
  // const Uint32 fps = dt ? 1000 / dt : 0;
  displayf(renderer, game_state.debug_font,
           {255, 0, 0, 255}, vec2(PADDING, PADDING),
           "FPS: %d", fps);
  displayf(renderer, game_state.debug_font,
           {255, 0, 0, 255}, vec2(PADDING, PADDING * 4),
           "Mouse Position (%d, %d)",
           game_state.mouse_position.x, game_state.mouse_position.y);
  displayf(renderer, game_state.debug_font,
           {255, 0, 0, 255}, vec2(PADDING, PADDING * 8),
           "Collision Porbe (%d, %d)",
           game_state.collision_probe.x, game_state.collision_probe.y);
  displayf(renderer, game_state.debug_font,
           {255, 0, 0, 255}, vec2(PADDING, PADDING * 12),
           "Projectiles: %d", count_alive_projectiles());

  for (size_t i = 0; i < entities_count; ++i) {
    if (entities[i].state == Entity_State::Ded)
      continue;
    sec(SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255));
    auto dstrect = entity_dstrect(entities[i]);
    sec(SDL_RenderDrawRect(renderer, &dstrect));

    sec(SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255));
    auto hitbox = entity_hitbox(entities[i]);
    sec(SDL_RenderDrawRect(renderer, &hitbox));
  }
}

void render_game_state(const Game_State game_state,
                  SDL_Renderer *renderer)
{
  sec(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255));
  sec(SDL_RenderClear(renderer));

  render_level(renderer, game_state.ground_grass_texture, game_state.ground_texture);
  render_entities(renderer);
  render_projectiles(renderer);
}

void update_game_state(const Game_State game_state, Uint32 dt) {
  for (int i = 0; i < ENEMY_COUNT; ++i) {
    entity_shoot(&entities[ENEMY_ENTITY_IDX_OFFSET + i]);
  }

  update_entities(game_state.gravity, dt);
  update_projectiles(dt);
}

const uint32_t STEP_DEBUG_FPS = 60;
int main() {
  sec(SDL_Init(SDL_INIT_VIDEO));

  // * Create a SDL window
  SDL_Window *window = sec(SDL_CreateWindow("Something", 0, 0, 800, 600, SDL_WINDOW_RESIZABLE));

  // * Create a SDL renderer
  SDL_Renderer *renderer = sec(SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED));

  // * Get tile texture from png
  SDL_Texture *tileset_texture = load_texture_from_png_file(renderer, "assets/sprites/fantasy_tiles.png");


  load_spritesheets(renderer);

  auto plasma_pop_animation = load_animation_file("./assets/animats/plasma_pop.txt");
  auto plasma_bolt_animation = load_animation_file("./assets/animats/plasma_bolt.txt");
  auto walking = load_animation_file("./assets/animats/walking.txt");
  auto idle = load_animation_file("./assets/animats/idle.txt");

  init_projectiles(plasma_bolt_animation, plasma_pop_animation);

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

  entities[PLAYER_ENTITY_IDX].state = Entity_State::Alive;
  entities[PLAYER_ENTITY_IDX].texbox = texbox;
  entities[PLAYER_ENTITY_IDX].hitbox = hitbox;
  entities[PLAYER_ENTITY_IDX].idle = idle;
  entities[PLAYER_ENTITY_IDX].walking = walking;
  entities[PLAYER_ENTITY_IDX].current = &entities[PLAYER_ENTITY_IDX].idle;

  for (int i = 0; i < ENEMY_COUNT; ++i) {
    entities[ENEMY_ENTITY_IDX_OFFSET + i].state = Entity_State::Alive;
    entities[ENEMY_ENTITY_IDX_OFFSET + i].texbox = texbox;
    entities[ENEMY_ENTITY_IDX_OFFSET + i].hitbox = hitbox;
    entities[ENEMY_ENTITY_IDX_OFFSET + i].walking = walking;
    entities[ENEMY_ENTITY_IDX_OFFSET + i].idle = idle;
    entities[ENEMY_ENTITY_IDX_OFFSET + i].current = &entities[ENEMY_ENTITY_IDX_OFFSET].idle;
    static_assert(LEVEL_WIDTH >= 2);
    entities[ENEMY_ENTITY_IDX_OFFSET + i].pos = vec2(LEVEL_WIDTH - 2 - i, 0) * TILE_SIZE;
    entities[ENEMY_ENTITY_IDX_OFFSET + i].dir = Entity_Dir::Left;
  }

  stec(TTF_Init());
  const int DEBUG_FONT_SIZE = 18;

  const Uint8 *keyboard = SDL_GetKeyboardState(NULL);

  Game_State game_state = {};
  game_state.gravity = {0, 1};
  game_state.debug_font = stec(TTF_OpenFont("assets/Comic-Sans-MS.ttf", DEBUG_FONT_SIZE));
  game_state.ground_grass_texture = {
    .srcrect = {120, 128, 16, 16},
    .texture = tileset_texture
  };
  game_state.ground_texture = {
    .srcrect = {120, 128 + 16, 16, 16},
    .texture = tileset_texture
  };

  bool debug = false;
  bool step_debug = false;
  Uint32 prev_dt = 0;
  while (!game_state.quit) {
    const Uint32 begin = SDL_GetTicks();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: {
          game_state.quit = true;
        } break;
        case SDL_KEYDOWN: {
          switch (event.key.keysym.sym) {
            case SDLK_SPACE: {
              entities[PLAYER_ENTITY_IDX].vel.y = -20;
            } break;
            case SDLK_q: {
              debug = !debug;
            } break;
            case SDLK_z: {
              step_debug = !step_debug;
            } break;
            case SDLK_x: {
              if(step_debug) {
                update_game_state(game_state, 1000 / STEP_DEBUG_FPS);
              }
            } break;
            case SDLK_e: {
              entity_shoot(&entities[PLAYER_ENTITY_IDX]);
            } break;
            case SDLK_r: {
              entities[PLAYER_ENTITY_IDX].pos = vec2(0, 0);
              entities[PLAYER_ENTITY_IDX].vel.y = 0;
              for (int i = 0; i < ENEMY_COUNT; ++i) {
                entities[ENEMY_ENTITY_IDX_OFFSET + i].pos.y = 0;
                entities[ENEMY_ENTITY_IDX_OFFSET + i].vel.y = 0;
              }
            } break;
          }
        } break;
        case SDL_MOUSEMOTION: {
          game_state.mouse_position = {event.motion.x, event.motion.y};
          game_state.collision_probe = game_state.mouse_position;
          resolve_point_collision(&game_state.collision_probe);

          Vec2i tile = vec2(event.button.x, event.button.y) / TILE_SIZE;
          switch(game_state.state) {
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
                game_state.state = Debug_Draw_State::Create;
                level[tile.y][tile.x] = Tile::Wall; 
              }
              else {
                game_state.state = Debug_Draw_State::Delete;
                level[tile.y][tile.x] = Tile::Empty;
              }
            }
          }
        } break;
        case SDL_MOUSEBUTTONUP: {
          game_state.state = Debug_Draw_State::Idle;
        } break;
      }
    }

    if (keyboard[SDL_SCANCODE_D]) {
      entity_move(&entities[PLAYER_ENTITY_IDX], PLAYER_SPEED);
    } else if (keyboard[SDL_SCANCODE_A]) {
      entity_move(&entities[PLAYER_ENTITY_IDX], -PLAYER_SPEED);
    } else {
      entity_stop(&entities[PLAYER_ENTITY_IDX]);
    }

    // * render the current state
    render_game_state(game_state, renderer);
    if(debug) {
      render_debug_overlay(game_state, renderer, step_debug ? STEP_DEBUG_FPS : 1000 / prev_dt);
    }
    SDL_RenderPresent(renderer);

    // * calculate the next state
    if(!step_debug) {
      const Uint32 dt = SDL_GetTicks() - begin;
      update_game_state(game_state, dt);
      prev_dt = dt;
    }
  }
  SDL_Quit();
  // dump_level(stdout);
  // dump_animation(walking, "sdf", stdout);
  return 0;
}


