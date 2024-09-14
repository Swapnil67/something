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
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall},
    {Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Wall, Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
};
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
      switch (level[y][x]) {
        case Tile::Empty: {
          // * Do nothing
        } break;
        case Tile::Wall: {
          if (is_tile_empty(vec2(x, y - 1))) {
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

void dump_level(FILE *stream) {
  println(stream, "{");
  for (int y = 0; y < LEVEL_HEIGHT; ++y) {
    print(stream, "{");
    for (int x = 0; x < LEVEL_WIDTH; ++x) {
      switch(level[y][x]) {
        case Tile::Empty: {
          print(stream, "Tile::Empty, ");
        } break;
        case Tile::Wall: {
          print(stream, "Tile::Wall, ");
        } break;
      }
    }
    print(stream, "},");
    println(stream, "\n");
  }
  println(stream, "};\n");
}
