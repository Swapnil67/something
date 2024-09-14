
////////////////////////////////////////////////
// * Sprite
////////////////////////////////////////////////

struct Sprite {
  SDL_Rect srcrect;
  SDL_Texture *texture;
};

#define ARRAY_SIZE(xs) (sizeof(xs) / sizeof(xs[0]))

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
    println(stderr, "Could not read file `", image_filename, "`: ", image.message);
    abort();
  }
  image.format = PNG_FORMAT_RGBA;
  // printf("Width: %d, Height %d\n", image.width, image.height);
  // uint32_t *image_pixels = (uint32_t *) std::malloc(sizeof(uint32_t) * image.width * image.height);
  uint32_t *image_pixels = new uint32_t[image.width * image.height];
  if(!png_image_finish_read(&image, nullptr, image_pixels, 0, nullptr)) {
    println(stderr, "libpng pooped itself: ", image.message);
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

struct Spritesheet {
  const char *filename;
  SDL_Texture *texture;
};

Spritesheet spritesheets[] = {
    {"./assets/sprites/destroy-sheet.png", nullptr},
    {"./assets/sprites/fantasy_tiles.png", nullptr},
    {"./assets/sprites/spark-sheet.png", nullptr},
    {"./assets/sprites/walking-12px.png", nullptr},
};

void load_spritesheets(SDL_Renderer *renderer) {
  for (size_t i = 0; i < ARRAY_SIZE(spritesheets); ++i) {
    if(spritesheets[i].texture == nullptr) {
      spritesheets[i].texture = load_texture_from_png_file(renderer, spritesheets[i].filename);
    }
  }
}

SDL_Texture *spritesheet_by_name(String_View filename) {
  for (size_t i = 0; i < ARRAY_SIZE(spritesheets); ++i) {
    if(filename == cstr_as_string_view(spritesheets[i].filename)) {
      return spritesheets[i].texture; 
    }
  }
  println(stderr, "Unknown texture file `", filename, "`. ", "You may want to add it to the spritesheet array.");
  abort();
  return nullptr;
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
  println(output, "sprite = ", sprite_filename);
  println(output, "count = ", animation.frame_count);
  println(output, "duration = ", animation.frame_duration);
  println(output);

  for (size_t i = 0; i < animation.frame_count; ++i) {
    println(output, "frames.", i, ".x = ", animation.frames[i].srcrect.x);
    println(output, "frames.", i, ".y = ", animation.frames[i].srcrect.y);
    println(output, "frames.", i, ".w = ", animation.frames[i].srcrect.w);
    println(output, "frames.", i, ".h = ", animation.frames[i].srcrect.h);
  }
}

void abort_parse_error(FILE *stream,
                       String_View source, String_View rest,
                       const char *prefix, const char *error)
{
    assert(stream);
    assert(source.data < rest.data);

    size_t n = (size_t) (rest.data - source.data);

    for (size_t line_number = 1; source.count; ++line_number) {
        auto line = source.chop_by_delim('\n');

        if (n <= line.count) {
          println(stream, prefix, ":", line_number, ": ", error);
          println(stream, line);
          println(stream, Pad{n, ' '}, '^');
          break;
        }

        n -= line.count + 1;
    }

    for (int i = 0; source.count && i < 3; ++i) {
        auto line = source.chop_by_delim('\n');
        fwrite(line.data, 1, line.count, stream);
        fputc('\n', stream);
    }

    abort();
}

Animation load_animation_file(const char *animation_filepath)
{
  String_View source = file_as_string_view(animation_filepath);
  String_View input = source;
  Animation animation = {};
  SDL_Texture *spritesheet_texture = nullptr;

  while(input.count != 0) {
    auto value = input.chop_by_delim('\n');
    auto key = value.chop_by_delim('=').trim();

    // * handle empty spaces & comments
    if (key.count == 0 || *key.data == '#')
      continue;

    value = value.trim();

    // String_View subkey = trim(chop_by_delim(&key, '.'), isspace);
    auto subkey = key.chop_by_delim('.').trim();

    if(subkey == "count"_sv) {
      if(animation.frames != nullptr) {
        abort_parse_error(stderr, source, input, animation_filepath, "`count` provided twice");
      }

      auto count_result = value.as_integer<size_t>();
      if(!count_result.has_value) {
        abort_parse_error(stderr, source, input, animation_filepath, "`count` is not a number");
      }
      animation.frame_count = count_result.unwrap;
      animation.frames = new Sprite[animation.frame_count];
    }
    else if(subkey == "sprite"_sv) {
      spritesheet_texture = spritesheet_by_name(value);
    }
    else if(subkey == "duration"_sv) {
      auto result = value.as_integer<size_t>();
      if(!result.has_value) {
        abort_parse_error(stderr, source, input, animation_filepath, "duration is not a number");   
      }
      animation.frame_duration = result.unwrap;
    }
    else if(subkey == "frames"_sv) {
      // Result<size_t, void> result = as_number<size_t>(trim(chop_by_delim(&key, '.'), isspace));
      auto result = key.chop_by_delim('.').trim().as_integer<size_t>();
      if (!result.has_value) {  
        abort_parse_error(stderr, source, input, animation_filepath, "frame index is not a number");   
      }
      
      size_t frame_index = result.unwrap;
      if (frame_index >= animation.frame_count) {
        abort_parse_error(stderr, source, input, animation_filepath, "incorrect frame index");   
      }

      animation.frames[frame_index].texture = spritesheet_texture; 

      // * parse the subkeys
      while(key.count) {
        subkey = key.chop_by_delim('.').trim();

        if(key.count != 0) {
          abort_parse_error(stderr, source, input, animation_filepath, "unknown subkey");   
        }

        auto result_value = value.as_integer<int>();
        if(!result_value.has_value) {
          abort_parse_error(stderr, source, input, animation_filepath, "frame parameter is not a number");   
        }

        if(subkey == "x"_sv) {
          animation.frames[frame_index].srcrect.x = result_value.unwrap;
        } else if(subkey == "y"_sv) {
          animation.frames[frame_index].srcrect.y = result_value.unwrap;
        } else if(subkey == "w"_sv) {
          animation.frames[frame_index].srcrect.w = result_value.unwrap;
        } else if(subkey == "h"_sv) {
          animation.frames[frame_index].srcrect.h = result_value.unwrap;
        } else {
          abort_parse_error(stderr, source, input, animation_filepath, "unknown subkey");
        }
      }
    } else {
      abort_parse_error(stderr, source, input, animation_filepath, "unknown subkey");
    }

  }

  delete[] source.data;

  // return ok<Animation, const char *>(animation);
  return animation;
}