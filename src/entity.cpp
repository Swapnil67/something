enum class Entity_Dir {
  Right = 0,
  Left
};

enum class Entity_State {
  Ded = 0,
  Alive
};

struct Entity {

  Entity_State state;

  SDL_Rect texbox;
  SDL_Rect hitbox;
  Vec2i pos;
  Vec2i vel;

  Animation idle;
  Animation walking;
  Animation *current;

  Entity_Dir dir;
  int cooldown_weapon;
};

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
  assert(renderer);

  if (entity.state == Entity_State::Ded)
    return;
    
  const SDL_Rect dstrect = entity_dstrect(entity);
  const SDL_RendererFlip flip = entity.dir == Entity_Dir::Right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
  render_animation(renderer, *entity.current, dstrect, flip);
}

void update_entity(Entity *entity, Vec2i gravity, uint32_t dt) {
  assert(entity);

  if (entity->state == Entity_State::Ded)
    return;

  // * Add Gravity
  entity->vel += gravity;

  // * Add Velocity
  entity->pos += entity->vel;

  // * Resolve Collision
  resolve_entity_collision(entity);

  entity->cooldown_weapon -= 1;

  update_animation(entity->current, dt);
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

const int ENTITY_WEAPON_COOLDOWN = 30;

void entity_shoot(Entity *entity) {
  assert(entity);
  if (entity->cooldown_weapon > 0)
    return;
  if (entity->dir == Entity_Dir::Right) {
    spwan_projectiles(entity->pos, vec2(10, 0));
  } else {
    spwan_projectiles(entity->pos, vec2(-10, 0));
  }
  entity->cooldown_weapon = ENTITY_WEAPON_COOLDOWN;
}

const int entities_count = 69;
Entity entities[entities_count];

// * update all entities
void update_entities(Vec2i gravity, uint32_t dt) {
  for (int i = 0; i < entities_count; ++i) {
    update_entity(&entities[i], gravity, dt);
  }
}

// * Render all entities
void render_entities(SDL_Renderer *renderer) {
  for (int i = 0; i < entities_count; ++i) {
    render_entity(renderer, entities[i]);
  } 
}