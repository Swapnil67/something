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

enum class Debug_Draw_State {
  Idle,
  Create,
  Delete
};

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

int main2() {
  auto input =  "    foo.bar.baz.hello.world = 67  \n"
                "   foo.bar.baz.hello.world = 67\n"
                "   \n"
                "    foo.bar.baz.hello.world = 67\n"_sv;
                
  printf("%zu\n", input.count);
    
  while(input.count != 0) {
    auto value = input.chop_by_delim('\n');
    auto key = value.chop_by_delim('=').trim();

    // * handle empty spaces & comments
    if(key.count == 0 || *key.data == '#')
      continue;

    value = value.trim();

    fputs("Key:\t", stdout);
    fputc('#', stdout);

    // * parse the subkeys
    while(key.count != 0) {
      auto subkey = key.chop_by_delim('.');
      fwrite(subkey.data, 1, subkey.count, stdout);
      fputc('#', stdout);
    }

    // fwrite(key.data, 1, key.count, stdout);
    fputc('#', stdout);
    fputc('\n', stdout);

    // printf("%zu\n", value.count);

    fputs("Value:\t", stdout);
    fputc('#', stdout);
    fwrite(value.data, 1, value.count, stdout);
    fputc('#', stdout);
    fputc('\n', stdout);
  }
  return 0;
}

int main() {
  sec(SDL_Init(SDL_INIT_VIDEO));

  // * Create a SDL window
  SDL_Window *window = sec(SDL_CreateWindow("Something", 0, 0, 800, 600, SDL_WINDOW_RESIZABLE));

  // * Create a SDL renderer
  SDL_Renderer *renderer = sec(SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED));

  // * Get tile texture from png
  SDL_Texture *tileset_texture = load_texture_from_png_file(renderer, "assets/sprites/fantasy_tiles.png");

  Sprite ground_grass_texture = {
    .srcrect = {120, 128, 16, 16},
    .texture = tileset_texture
  };

  Sprite ground_texture = {
    .srcrect = {120, 128 + 16, 16, 16},
    .texture = tileset_texture
  };

  load_spritesheets(renderer);

  auto plasma_pop_animation = load_animation_file("./assets/animats/plasma_pop.txt");
  auto plasma_bolt_animation = load_animation_file("./assets/animats/plasma_bolt.txt");
  auto walking = load_animation_file("./assets/animats/walking.txt");
  auto idle = load_animation_file("./assets/animats/idle.txt");

  init_projectiles(plasma_bolt_animation, plasma_pop_animation);

  const int PLAYER_SPEED = 4;

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

  int PLAYER_ENTITY_IDX = 0;
  entities[PLAYER_ENTITY_IDX].state = Entity_State::Alive;
  entities[PLAYER_ENTITY_IDX].texbox = texbox;
  entities[PLAYER_ENTITY_IDX].hitbox = hitbox;
  entities[PLAYER_ENTITY_IDX].idle = idle;
  entities[PLAYER_ENTITY_IDX].walking = walking;
  entities[PLAYER_ENTITY_IDX].current = &entities[PLAYER_ENTITY_IDX].idle;

  int ENEMY_COUNT = 1;
  int ENEMY_ENTITY_IDX_OFFSET = 1;
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
              entities[PLAYER_ENTITY_IDX].vel.y = -20;
            } break;
            case SDLK_q: {
              debug = !debug;
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

    for (int i = 0; i < ENEMY_COUNT; ++i) {
      entity_shoot(&entities[ENEMY_ENTITY_IDX_OFFSET + i]);
    }
    if(keyboard[SDL_SCANCODE_D]) {
      entity_move(&entities[PLAYER_ENTITY_IDX], PLAYER_SPEED);
    } else if(keyboard[SDL_SCANCODE_A]) {
      entity_move(&entities[PLAYER_ENTITY_IDX], -PLAYER_SPEED);
    } else {
      entity_stop(&entities[PLAYER_ENTITY_IDX]);
    }

    sec(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255));
    sec(SDL_RenderClear(renderer));

    render_level(renderer, ground_grass_texture, ground_texture);
    render_entities(renderer);
    render_projectiles(renderer);

    if(debug) {
      sec(SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255));

      auto dstrect = entity_dstrect(entities[PLAYER_ENTITY_IDX]);
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
      auto hitbox = entity_hitbox(entities[PLAYER_ENTITY_IDX]);
      sec(SDL_RenderDrawRect(renderer, &hitbox));
    }
    
    SDL_RenderPresent(renderer);

    const Uint32 dt = SDL_GetTicks() - begin;
    // printf("%d\n", dt);

    update_entities(gravity, dt);
    update_projectiles(dt);
  }
  SDL_Quit();
  // dump_level(stdout);
  // dump_animation(walking, "sdf", stdout);
  return 0;
}


