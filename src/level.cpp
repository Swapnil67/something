const float TILE_SIZE = 64.0f;
const float TILE_SIZE_SQR = TILE_SIZE * TILE_SIZE;

enum class Tile {
  Empty = 0,
  Wall
};


const int LEVEL_WIDTH = 10;
const int LEVEL_HEIGHT = 10;

// * Level Boundary
const Rectf LEVEL_BOUNDARY = {0.0f, 0.0f, LEVEL_WIDTH *TILE_SIZE, LEVEL_HEIGHT *TILE_SIZE};

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
bool is_tile_inbounds(Vec2<int> p) {
  return 0 <= p.x && p.x < LEVEL_WIDTH && 0 <= p.y && p.y < LEVEL_HEIGHT;
}

bool is_tile_empty(Vec2<int> p) {
  return !is_tile_inbounds(p) || level[p.y][p.x] == Tile::Empty;
}

void render_level(Camera camera, SDL_Renderer *renderer, Sprite top_ground_texture, Sprite bottom_ground_texture) {
    for (int y = 0; y < LEVEL_HEIGHT; ++y) {
    for (int x = 0; x < LEVEL_WIDTH; ++x) {
      switch (level[y][x]) {
        case Tile::Empty: {
          // * Do nothing
        } break;
        case Tile::Wall: {
          const auto dstrect = rect(
              vec_cast<float>(vec2(x, y)) * TILE_SIZE - camera.pos,
              TILE_SIZE, TILE_SIZE);
          if (is_tile_empty(vec2(x, y - 1))) {
            render_sprite(renderer, top_ground_texture, dstrect);
          }
          else {
            render_sprite(renderer, bottom_ground_texture, dstrect);
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
