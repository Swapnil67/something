
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
  assert(animation);
  if (dt < animation->frame_cooldown) {
    animation->frame_cooldown -= dt;
  } else {
    // * To keep the animation frames bounded to frame count [%]
    animation->frame_current = (animation->frame_current + 1) % animation->frame_count;
    animation->frame_cooldown = animation->frame_duration;
  }
}

// * Creates SDL_Surface from png image
SDL_Surface *load_png_file_as_surface(const char *image_filename) {
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
  return image_surface;
}

// * Create a image texture from png image
SDL_Texture *load_texture_from_png_file(SDL_Renderer *renderer, const char *image_filename) {
  SDL_Surface *image_surface = load_png_file_as_surface(image_filename);
  SDL_Texture *image_texture = sec(SDL_CreateTextureFromSurface(renderer, image_surface));
  SDL_FreeSurface(image_surface);
  return image_texture;
}

SDL_Texture *load_texture_from_png_file(SDL_Renderer *renderer, String_View<char> image_filename) {
  char buffer[256] = {};
  strncpy(buffer, image_filename.data, min(sizeof(buffer)-1, image_filename.count));
  return load_texture_from_png_file(renderer, buffer);
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


void dump_animation(Animation animation, const char *sprite_filename, FILE *output) {
  fprintf(output, "sprite = %s\n", sprite_filename);
  fprintf(output, "count = %lu\n", animation.frame_count);
  fprintf(output, "duration = %u\n", animation.frame_duration);
  fprintf(output, "\n");

  for (size_t i = 0; i < animation.frame_count; ++i) {
    fprintf(output, "frames.%lu.x = %d\n", i, animation.frames[i].srcrect.x);
    fprintf(output, "frames.%lu.y = %d\n", i, animation.frames[i].srcrect.y);
    fprintf(output, "frames.%lu.w = %d\n", i, animation.frames[i].srcrect.w);
    fprintf(output, "frames.%lu.h = %d\n", i, animation.frames[i].srcrect.h);
  }
}

Result<Animation, const char *> parse_animation(SDL_Renderer *renderer, String_View<char> input) {
  Animation animation = {};
  SDL_Texture *spritesheet_texture = nullptr;

  while(input.count != 0) {
    String_View<char> value = chop_by_delim(&input, '\n');
    String_View<char> key = trim(chop_by_delim(&value, '='), isspace);

    // * handle empty spaces & comments
    if (key.count == 0 || *key.data == '#')
      continue;

    value = trim(value, isspace);

    String_View<char> subkey = trim(chop_by_delim(&key, '.'), isspace);

    if(subkey == "count") {

      if(animation.frames != nullptr) {
        return fail<Animation>("count provided twice.");
      }

      Result<size_t, void> count_result = as_number<size_t>(value);
      if(count_result.is_error) {
        return fail<Animation>("count is not a number.");
      }
      animation.frame_count = count_result.unwrap;
      animation.frames = new Sprite[animation.frame_count];
    }
    else if(subkey == "sprite") {
      spritesheet_texture = load_texture_from_png_file(renderer, value);
    }
    else if(subkey == "duration") {
      Result<size_t, void> result = as_number<size_t>(value);
      if(result.is_error) {
        return fail<Animation>("duration is not a number");
      }
      animation.frame_duration = result.unwrap;
    }
    else if(subkey == "frames") {
      Result<size_t, void> result = as_number<size_t>(trim(chop_by_delim<char>(&key, '.'), isspace));
      if (result.is_error) {  
        return fail<Animation>("incorrect frame index");
      }
      
      size_t frame_index = result.unwrap;
      if (frame_index >= animation.frame_count) {
        return fail<Animation>("incorrect frame index");
      }

      animation.frames[frame_index].texture = spritesheet_texture; 

      // * parse the subkeys
      while(key.count) {
        subkey = trim(chop_by_delim<char>(&key, '.'), isspace); 

        if(key.count != 0) {
          return fail<Animation>("unknown subkeys");
        }

        Result<int, void> result_value = as_number<int>(value);
        if(result_value.is_error) {
          return fail<Animation>("frame parameter is not a number");
        }

        if(subkey == "x") {
          animation.frames[frame_index].srcrect.x = result_value.unwrap;
        } else if(subkey == "y") {
          animation.frames[frame_index].srcrect.y = result_value.unwrap;
        } else if(subkey == "w") {
          animation.frames[frame_index].srcrect.w = result_value.unwrap;
        } else if(subkey == "h") {
          animation.frames[frame_index].srcrect.h = result_value.unwrap;
        } else {
          return fail<Animation>("unknown subkeys"); 
        }

      }
    }

  }

  return ok<Animation, const char *>(animation);
}