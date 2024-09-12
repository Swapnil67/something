// template <typename T>
template <typename T>
struct Maybe {
  bool has_value;
  T unwrap;
};

struct String_View {
  size_t count;
  const char *data;

  String_View trim_begin(void) const {
    String_View view = *this;
    while (view.count != 0 && isspace(*view.data)) {
      view.data += 1;
      view.count -= 1;
    }
    return view;
  }

  String_View trim_end(void) const {
    String_View view = *this;
    while (view.count != 0 && isspace(*(view.data + view.count - 1))) {
      view.count -= 1;
    }
    return view;
  }

  String_View trim(void) const {
    return trim_begin().trim_end();
  }

  void chop(size_t n) {
    if(n > count) {
      data += count;
      count = 0;
    }
    else {
      data += n;
      count -= n;
    }
  }

  String_View chop_by_delim(char delim) {
    assert(data);

    size_t i = 0;
    while(i < count && data[i] != delim) i++;
    String_View result = {i, data};
    chop(i + 1);
    return result;
  }

  template <typename Integer>
  Maybe<Integer> as_integer() const {
    Integer sign = 1;
    Integer number = {};
    String_View view = *this;

    if(view.count == 0) {
      return {};
    }

    if(*view.data == '-') {
      sign = -1;
      view.chop(1);
    }

    while(view.count) {
      if(!(isdigit(*view.data))) {
        return {};
      }
      number = number * 10 + (*view.data - '0');
      view.chop(1);
    }

    return {true, number * sign};
  }
};

String_View cstr_as_string_view(const char* cstr) {
  return String_View{strlen(cstr), cstr};
}

String_View operator""_sv(const char *data, size_t count) {
  String_View result;
  result.count = count;
  result.data = data;
  return result;
}

// * check if two String_View are equal
bool operator==(String_View view1, String_View view2) {
  // printf("%zu == %zu\n", view1.count, view2.count);
  if (view1.count != view2.count)
    return false;
  return memcmp(view1.data, view2.data, view1.count) == 0;
}

String_View file_as_string_view(const char* filepath) {
  assert(filepath);
  size_t n = 0;
  String_View result = {};
  FILE *f = fopen(filepath, "rb");
  if(!f) {
    fprintf(stderr, "Could not open file %s: %s\n", filepath, strerror(errno));
    abort();
  }

  int code = fseek(f, 0, SEEK_END);
  if(code < 0) {
    fprintf(stderr, "Could not find the end of file %s: %s\n", filepath, strerror(errno));
    abort(); 
  }

  long m = ftell(f);
  if(m < 0) {
    fprintf(stderr, "Could not get the end of file %s: %s\n", filepath, strerror(errno));
    abort(); 
  }

  result.count = (size_t)m;

  code = fseek(f, 0, SEEK_SET);
  if(code < 0) {
    fprintf(stderr, "Could not find the beginning of file %s: %s\n", filepath, strerror(errno));
    abort(); 
  } 

  char *buffer = new char[result.count];
  n = fread(buffer, 1, result.count, f);
  assert(n == result.count);
  result.data = buffer;

  return result;
}

// * convert string to String_View
// template <typename T>
// String_View<T> string_view_of_cstr(const char *cstr) {
//   String_View<T> result = {};
//   result.count = strlen(cstr);
//   result.data = cstr;
//   return result;
// }

// template <typename T, typename Is_Space>
// String_View<T> trim_begin(String_View<T> view, Is_Space is_space) {
//   while (view.count != 0 && is_space(*view.data)) {
//     view.data += 1;
//     view.count -= 1;
//   }
//   return view;
// }

// template <typename T, typename Is_Space>
// String_View<T> trim_end(String_View<T> view, Is_Space is_space) {
//   while (view.count != 0 && is_space(*(view.data + view.count - 1))) {
//     view.count -= 1;
//   }
//   return view;
// }

// template <typename T, typename Is_Space>
// String_View<T> trim(String_View<T> view, Is_Space is_space) {
//   return trim_end(trim_begin(view, is_space), is_space);
// }

// template <typename T>
// String_View<T> chop_by_delim(String_View<T> *view, T delim) {
//   assert(view);
//   assert(view->data);

//   size_t i = 0;
//   while(i < view->count && view->data[i] != delim)
//     i++;
  
//   String_View<T> result;
//   result.count = i;
//   result.data = view->data;

//   if(i < view->count) {
//     view->count -= (i + 1);
//     view->data += (i + 1);
//   }
//   else {
//     view->count -= i;
//     view->data += i;
//   }

//   return result;
// }

// * operator overloading
// template <typename T>
// bool operator==(String_View<T> view, const T *cstr) {
//   return view == string_view_of_cstr<T>(cstr);
// }

// template <typename T>
// Result<T, void> as_number(String_View<char> view) {

//   T result = {};
//   for (size_t i = 0; i < view.count; ++i) {
//     if(!isdigit(view.data[i])) {
//       return fail<T>();
//     }
//     result = result * 10 + (view.data[i] - '0');
//   }

//   return ok<T, void>(result);
// }

// String_View<char> file_as_string_view(const char *filepath) {
//   assert(filepath);
//   String_View<char> result = {};

//   size_t n = 0;
//   FILE *f = fopen(filepath, "rb");
//   assert(f);

//   fseek(f, 0, SEEK_END);
//   long m = ftell(f);
//   fseek(f, 0, SEEK_SET);
//   result.count = (size_t)m;
//   char *buffer = new char[result.count];
//   n = fread(buffer, 1, result.count, f);
//   assert(n == result.count);
//   result.data = buffer;

//   return result;
// }