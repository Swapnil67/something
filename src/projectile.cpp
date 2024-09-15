
enum class Projectile_State {
  Ded = 0,
  Active,
  Poof
};

const char* projectile_state_as_cstr(Projectile_State state) {
  switch (state) {
  case Projectile_State::Ded:
    return "Ded";
  case Projectile_State::Active:
    return "Active";
  case Projectile_State::Poof:
    return "Poof ";
  }

  assert(!"Incorrect Projectile State");
}

struct Projectile {
  Vec2i pos;
  Vec2i vel;
  size_t shooter_entity;
  Projectile_State state;
  Animation active_animation;
  Animation poof_animation;
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

void spwan_projectiles(Vec2i pos, Vec2i vel, int shooter_entity) {
   for (size_t i = 0; i < projectiles_count; ++i) {
    // * Find the first one which is in ded state & activate it & return.
    if(projectiles[i].state == Projectile_State::Ded) {
      projectiles[i].state = Projectile_State::Active;
      projectiles[i].pos = pos;
      projectiles[i].vel = vel;
      projectiles[i].shooter_entity = shooter_entity;
      return; 
    }
  } 
}

void render_projectiles(SDL_Renderer *renderer) {
  for (size_t i = 0; i < projectiles_count; ++i) {
    switch (projectiles[i].state) {
      case Projectile_State::Active: {
        render_animation(
            renderer,
            projectiles[i].active_animation,
            projectiles[i].pos);
      } break;
      case Projectile_State::Poof: {
        render_animation(
            renderer,
            projectiles[i].poof_animation,
            projectiles[i].pos);
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
        auto projectile_tile = projectiles[i].pos / TILE_SIZE;
        if (!is_tile_empty(projectile_tile) ||
            !rect_contains_vec2i(LEVEL_BOUNDARY, projectiles[i].pos))
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

const int PROJECTILE_TRACKING_PADDING = 50;

SDL_Rect hitbox_of_projectile (size_t index) {
  // * x = 188, y = 237
  // * x = 163, y = 212
  assert(index < projectiles_count);
  return SDL_Rect{
      projectiles[index].pos.x - PROJECTILE_TRACKING_PADDING / 2,
      projectiles[index].pos.y - PROJECTILE_TRACKING_PADDING / 2,
      PROJECTILE_TRACKING_PADDING,
      PROJECTILE_TRACKING_PADDING};
}

int projectiles_at_position(Vec2i position) {
  // * projectile hitbox
  for (int i = 0; i < (int)projectiles_count; ++i) {
    if (projectiles[i].state == Projectile_State::Ded)
      continue;
    SDL_Rect hitbox = hitbox_of_projectile(i);
    if(rect_contains_vec2i(hitbox, position)) {
      return i;
    }
  }

  return -1;
}