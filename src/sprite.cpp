
////////////////////////////////////////////////
// * Sprite
////////////////////////////////////////////////


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

////////////////////////////////////////////////
// * Animation
////////////////////////////////////////////////

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

Animation load_spritesheet_animation(
    SDL_Renderer *renderer,
    size_t frame_count,
    uint32_t frame_duration,
    const char *spritesheet_filepath)
{
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
