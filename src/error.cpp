template <typename T>
T *stec(T *ptr) {
  if(ptr == nullptr) {
    fprintf(stderr, "SDL_ttf popped itself: %s\n", TTF_GetError());
    abort();
  }
  return ptr;
}

void stec(int code) {
  if(code < 0) {
    fprintf(stderr, "SDL_ttf popped itself: %s\n", TTF_GetError());
    abort();
  }
}

int sec(int code) {
  if(code < 0) {
    fprintf(stderr, "SDL popped itself: %s\n", SDL_GetError());
    abort();
  }
  return code;
}

template <typename T>
T *sec(T *ptr) {
  if(ptr == nullptr) {
    fprintf(stderr, "SDL popped itself: %s\n", SDL_GetError());
    abort(); 
  }
  return ptr;
} 