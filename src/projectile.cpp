
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

void spwan_projectiles(Vec2i pos, Vec2i vel) {
   for (size_t i = 0; i < projectiles_count; ++i) {
    // * Find the first one which is in ded state & activate it & return.
    if(projectiles[i].state == Projectile_State::Ded) {
      projectiles[i].state = Projectile_State::Active;
      projectiles[i].pos = pos;
      projectiles[i].vel = vel;
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
