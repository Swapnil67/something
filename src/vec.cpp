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

static inline
int sqr_dist(Vec2i p0, Vec2i p1) {
  auto d = p0 - p1;
  return (d.x * d.x) + (d.y * d.y);
}

////////////////////////////
// * Vector X Scaler
////////////////////////////

template <typename T>
Vec2<T> operator+(Vec2<T> a, T b) { return {a.x + b, a.y + b}; }

template <typename T>
Vec2<T> operator-(Vec2<T> a, T b) { return {a.x - b, a.y - b}; }

template <typename T>
Vec2<T> operator*(Vec2<T> a, T b) { return {a.x * b, a.y * b}; }

template <typename T>
Vec2<T> operator/(Vec2<T> a, T b) { return {a.x / b, a.y / b}; }

template <typename T>
Vec2<T> &operator+=(Vec2<T> &a, T b) { a = a + b; return a; }
template <typename T>
Vec2<T> &operator-=(Vec2<T> &a, T b) { a = a - b; return a; }
template <typename T>
Vec2<T> &operator*=(Vec2<T> &a, T b) { a = a * b; return a; }
template <typename T>
Vec2<T> &operator/=(Vec2<T> &a, T b) { a = a / b; return a; }


////////////////////////////
// * Scaler X Vector
////////////////////////////

template <typename T>
Vec2<T> operator+(T a, Vec2<T> b) { return {a + b.x, a + b.y}; }

template <typename T>
Vec2<T> operator-(T a, Vec2<T> b) { return {a - b.x, a - b.y}; }

template <typename T>
Vec2<T> operator*(T a, Vec2<T> b) { return {a * b.x, a * b.y}; }

template <typename T>
Vec2<T> operator/(T a, Vec2<T> b) { return {a / b.x, a / b.y}; }


template <typename T>
Vec2<T> &operator+=(T &a, Vec2<T> b) { a = a + b; return a; }
template <typename T>
Vec2<T> &operator-=(T &a, Vec2<T> b) { a = a - b; return a; }
template <typename T>
Vec2<T> &operator*=(T &a, Vec2<T> b) { a = a * b; return a; }
template <typename T>
Vec2<T> &operator/=(T &a, Vec2<T> b) { a = a / b; return a; }


#endif // * VEC_HPP_
