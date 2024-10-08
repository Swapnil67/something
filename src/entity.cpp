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
  
  Rectf texbox_local;
  Rectf hitbox_local;
  Vec2f pos;
  Vec2f vel;

  Animation idle;
  Animation walking;
  Animation *current;

  Entity_Dir dir;
  int cooldown_weapon;
};

const int ENTITIES_COUNT = 69;
Entity entities[ENTITIES_COUNT];


void resolve_point_collision(Vec2f *p) {
  assert(p);

  // * Calculates which tile player is currently standing
  const auto tile = vec_cast<int>(*p / TILE_SIZE);
  // printf("Position : %d\t%d\n", p->x, p->y);
  // printf("Tile x : %d\t Tile y : %d\n", tile.x, tile.y);

  // * Out of bound [No Tile To Walk]
  if (is_tile_empty(tile)) {
    return;
  }

  // * Calculates the tile texbox points
  const auto p0 = vec_cast<float>(tile) * TILE_SIZE;
  const auto p1 = vec_cast<float>(tile + 1) * TILE_SIZE;
  // printf("%d\t%d\t%d\t%d\n", p0.x, p1.x, p0.y, p1.y);

  struct Side {
    float d;
    Vec2f np;     // * neighbor position
    Vec2i nd;     // * neighbor direction
    float dd;
  };

  // * distance x and y
  Side sides[] = {
      {sqr_dist<float>({p0.x, 0}, {p->x, 0}), {p0.x, p->y}, {-1, 0}, TILE_SIZE_SQR},            // * Left side
      {sqr_dist<float>({p1.x, 0}, {p->x, 0}), {p1.x, p->y}, {1, 0}, TILE_SIZE_SQR},             // * Right side
      {sqr_dist<float>({0, p0.y}, {0, p->y}), {p->x, p0.y}, {0, -1}, TILE_SIZE_SQR},            // * Top side
      {sqr_dist<float>({0, p1.y}, {0, p->y}), {p->x, p1.y}, {0, 1}, TILE_SIZE_SQR},             // * Bottom side
      {sqr_dist<float>({p0.x, p0.y}, {p->x, p->y}), {p0.x, p0.y}, {-1, -1}, TILE_SIZE_SQR * 2}, // * Top left
      {sqr_dist<float>({p1.x, p0.y}, {p->x, p->y}), {p1.x, p0.y}, {1, -1}, TILE_SIZE_SQR * 2},  // * Top right
      {sqr_dist<float>({p0.x, p1.y}, {p->x, p->y}), {p0.x, p1.y}, {-1, 1}, TILE_SIZE_SQR * 2},  // * Bottom left
      {sqr_dist<float>({p1.x, p1.y}, {p->x, p->y}), {p1.x, p1.y}, {1, 1}, TILE_SIZE_SQR * 2},   // * Bottom right
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
  Vec2f p0 = vec2(entity->hitbox_local.x, entity->hitbox_local.y) + entity->pos;
  Vec2f p1 = p0 + vec2(entity->hitbox_local.w, entity->hitbox_local.h);

  // printf("%d\t%d\t%d\t%d\n", p0.x, p1.x, p0.y, p1.y);
  // * 0       48       211      259
  Vec2f mesh[] = {
    p0,
    {p1.x, p0.y},
    {p0.x, p1.y},
    p1
  };

  const int MESH_COUNT = sizeof(mesh) / sizeof(mesh[0]);
  for (int i = 0; i < MESH_COUNT; ++i) {
    Vec2f t = mesh[i];
    resolve_point_collision(&t);

    // * Snaps the entity to proper position & resolves the collision
    Vec2f d = t - mesh[i];

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

Rectf entity_texbox_world(const Entity entity) {
  return Rectf{
      entity.texbox_local.x + entity.pos.x,
      entity.texbox_local.y + entity.pos.y,
      entity.texbox_local.w,
      entity.texbox_local.h};
}

Rectf entity_hitbox_world(const Entity entity) {
  return Rectf{
      entity.hitbox_local.x + entity.pos.x,
      entity.hitbox_local.y + entity.pos.y,
      entity.hitbox_local.w,
      entity.hitbox_local.h};
}

void render_entity(SDL_Renderer *renderer, const Entity entity, Camera camera) {
  assert(renderer);

  if (entity.state == Entity_State::Ded)
    return;
    
  Rectf dstrect = entity_texbox_world(entity);
  dstrect.x -= camera.pos.x;
  dstrect.y -= camera.pos.y;

  const SDL_RendererFlip flip = entity.dir == Entity_Dir::Right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
  render_animation(renderer, *entity.current, dstrect, flip);
}

void update_entity(Entity *entity, Vec2f gravity, float dt) {
  assert(entity);

  if (entity->state == Entity_State::Ded)
    return;

  // * Add Gravity
  entity->vel += gravity * dt;

  // * Add Velocity
  entity->pos += entity->vel * dt;

  // * Resolve Collision
  resolve_entity_collision(entity);

  entity->cooldown_weapon -= 1;

  update_animation(entity->current, dt);
}


void entity_move(Entity *entity, float speed) {
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

void entity_shoot(int entity_index) {
  assert(0 <= entity_index);
  assert(entity_index < ENTITIES_COUNT);
  Entity *entity = &entities[entity_index];

  if (entity->state == Entity_State::Ded)
    return;
  if (entity->cooldown_weapon > 0)
    return;
  if (entity->dir == Entity_Dir::Right) {
    spwan_projectiles(entity->pos, vec2(10.0f, 0.0f), entity_index);
  } else {
    spwan_projectiles(entity->pos, vec2(-10.0f, 0.0f), entity_index);
  }
  entity->cooldown_weapon = ENTITY_WEAPON_COOLDOWN;
}

// * update all entities
void update_entities(Vec2f gravity, float dt) {
  for (int i = 0; i < ENTITIES_COUNT; ++i) {
    update_entity(&entities[i], gravity, dt);
  }
}

// * Render all entities
void render_entities(Camera camera, SDL_Renderer *renderer) {
  for (int i = 0; i < ENTITIES_COUNT; ++i) {
    render_entity(renderer, entities[i], camera);
  } 
}