template <typename T>
T *stec(T *ptr) {
  if(ptr == nullptr) {
    println(stderr, "SDL_ttf popped itself: ", TTF_GetError());
    abort();
  }
  return ptr;
}

void stec(int code) {
  if(code < 0) {
    println(stderr, "SDL_ttf popped itself: ", TTF_GetError());
    abort();
  }
}

int sec(int code) {
  if(code < 0) {
    println(stderr, "SDL popped itself: ", SDL_GetError());
    abort();
  }
  return code;
}

template <typename T>
T *sec(T *ptr) {
  if(ptr == nullptr) {
    println(stderr, "SDL popped itself: ", SDL_GetError());
    abort(); 
  }
  return ptr;
} 