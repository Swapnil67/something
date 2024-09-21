#ifndef VEC_HPP_
#define VEC_HPP_

template <typename T>
struct Vec2 {
  T x, y;
};

template <typename T>
Vec2<T> vec2(T x, T y) { return {x, y}; };

// * alias
using Vec2i = Vec2<int>;
using Vec2f = Vec2<float>;

////////////////////////////
// * Vector X Vector
////////////////////////////

template <typename T>
Vec2<T> operator+(Vec2<T> a, Vec2<T> b) { return {a.x + b.x, a.y + b.y}; }

template <typename T>
Vec2<T> operator-(Vec2<T> a, Vec2<T> b) { return {a.x - b.x, a.y - b.y}; }

template <typename T>
Vec2<T> operator*(Vec2<T> a, Vec2<T> b) { return {a.x * b.x, a.y * b.y}; }

template <typename T>
Vec2<T> operator/(Vec2<T> a, Vec2<T> b) { return {a.x / b.x, a.y / b.y}; }

template <typename T>
Vec2<T> &operator+=(Vec2<T> &a, Vec2<T> b) { a = a + b; return a; }
template <typename T>
Vec2<T> &operator-=(Vec2<T> &a, Vec2<T> b) { a = a - b; return a; }
template <typename T>
Vec2<T> &operator*=(Vec2<T> &a, Vec2<T> b) { a = a * b; return a; }
template <typename T>
Vec2<T> &operator/=(Vec2<T> &a, Vec2<T> b) { a = a / b; return a; }

template <typename T>
static inline
T sqr_dist(Vec2<T> p0, Vec2<T> p1) {
  auto d = p0 - p1;
  return (d.x * d.x) + (d.y * d.y);
}

template <typename T>
struct Rect {
  T x, y, w, h;
};
using Rectf = Rect<float>;

template <typename T>
bool rect_contains_vec2(Rect<T> rect, Vec2<T> point) {
  return rect.x <= point.x && point.x < (rect.x + rect.w) && rect.y <= point.y && point.y < (rect.y + rect.h);
}

////////////////////////////
// * Vector X Scaler
////////////////////////////

template <typename T>
Vec2<T> constexpr operator+(Vec2<T> a, T b) { return {a.x + b, a.y + b}; }
template <typename T>
Vec2<T> constexpr operator-(Vec2<T> a, T b) { return {a.x - b, a.y - b}; }
template <typename T>
Vec2<T> constexpr operator*(Vec2<T> a, T b) { return {a.x * b, a.y * b}; }
template <typename T>
Vec2<T> constexpr operator/(Vec2<T> a, T b) { return {a.x / b, a.y / b}; }

template <typename T>
Vec2<T> constexpr &operator+=(Vec2<T> &a, T b) { a = a + b; return a; }
template <typename T>
Vec2<T> constexpr &operator-=(Vec2<T> &a, T b) { a = a - b; return a; }
template <typename T>
Vec2<T> constexpr &operator*=(Vec2<T> &a, T b) { a = a * b; return a; }
template <typename T>
Vec2<T> constexpr &operator/=(Vec2<T> &a, T b) { a = a / b; return a; }


////////////////////////////
// * Scaler X Vector
////////////////////////////

template <typename T>
Vec2<T> constexpr operator+(T a, Vec2<T> b) { return {a + b.x, a + b.y}; }
template <typename T>
Vec2<T> constexpr operator-(T a, Vec2<T> b) { return {a - b.x, a - b.y}; }
template <typename T>
Vec2<T> constexpr operator*(T a, Vec2<T> b) { return {a * b.x, a * b.y}; }
template <typename T>
Vec2<T> constexpr operator/(T a, Vec2<T> b) { return {a / b.x, a / b.y}; }

template <typename T>
Vec2<T> constexpr &operator+=(T &a, Vec2<T> b) { a = a + b; return a; }
template <typename T>
Vec2<T> constexpr &operator-=(T &a, Vec2<T> b) { a = a - b; return a; }
template <typename T>
Vec2<T> constexpr &operator*=(T &a, Vec2<T> b) { a = a * b; return a; }
template <typename T>
Vec2<T> constexpr &operator/=(T &a, Vec2<T> b) { a = a / b; return a; }


////////////////////////////
// * Just Vector
////////////////////////////

template <typename T>
Vec2<T> constexpr operator-(Vec2<T> a) { return {-a.x, -a.y}; }


////////////////////////////
// * Rect x Vector
////////////////////////////
template <typename T>
Rect<T> constexpr operator-(Rect<T> a, Vec2<T> b) { return {a.x - b.x, a.y - b.y, a.w, a.h}; }

////////////////////////////
// * Algorithms
////////////////////////////

template <typename T>
T min(T a, T b) {
  return a < b ? a : b;
}

template <typename T>
T abs(T x) {
  return x < 0 ? -x : x;
}

template <typename T>
Rect<T> rect(Vec2<T> pos, float w, float h) {
  return {pos.x, pos.y, w, h};
}

// * convert my rect to SDL_Rect
SDL_Rect rectf_for_sdl(Rectf rect) {
  return {(int)floorf(rect.x),
          (int)floorf(rect.y),
          (int)floorf(rect.w),
          (int)floorf(rect.h)};
}

template <typename U, typename T>
Vec2<U> vec_cast(Vec2<T> v) {
  return {(U)v.x, (U)v.y};
}

// * Template specialisation
template<>
Vec2i vec_cast(Vec2<float> v) {
  return {(int)floorf(v.x), (int)floorf(v.y)};
}

#endif // * VEC_HPP_


