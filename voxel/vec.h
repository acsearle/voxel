//
//  vec.h
//  voxel
//
//  Created by Antony Searle on 2/11/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#ifndef voxel_vec_h
#define voxel_vec_h

#include <cstddef>
#include <cassert>
#include <cmath>

using std::size_t;

template<typename T, size_t N> class vec;

template<typename T> class vec<T, 2> {
public:
    union {
        T data_[2];
        struct { T x, y; };
        struct { T r, g; };
        struct { T s, t; };
    };
    vec() : x(0), y(0) {}
    vec(T x, T y) : x(x), y(y) {}
    T& operator[](size_t i) { assert(i < 2); return data_[i]; }
    const T& operator[](size_t i) const { assert(i < 2); return data_[i]; }
};

template<typename T> class vec<T, 3> {
public:
    union {
        T data_[3];
        struct { T x, y, z; };
        struct { T r, g, b; };
        struct { T s, t, p; };
        
    };
    vec() : x(0), y(0), z(0) {}
    vec(T x, T y, T z) : x(x), y(y), z(z) {}
    T& operator[](size_t i) { assert(i < 3); return data_[i]; }
    const T& operator[](size_t i) const { assert(i < 3); return data_[i]; }
};

template<typename T> class vec<T, 4> {
public:
    union {
        T data_[4];
        struct { T x, y, z, w; };
        struct { T r, g, b, a; };
        struct { T s, t, p, q; };
    };
    vec() : x(0), y(0), z(0), w(0) {}
    vec(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
    T& operator[](size_t i) { assert(i < 4); return data_[i]; }
    const T& operator[](size_t i) const { assert(i < 4); return data_[i]; }
    vec(vec<T, 3> a, T w) : x(a.x), y(a.y), z(a.z), w(w) {}
    vec<T, 3>& xyz() { return reinterpret_cast<vec<T,3>&>(*this); }
};

#define VTN vec<T, N>

#define BINARY_INLINE(F, AT, BT, AI, BI) \
template<typename T, size_t N> VTN operator F (AT a, BT b) { \
    VTN c; \
    for (size_t i = 0; i != N; ++i) \
        c[i] = a AI F b BI; \
    return c; \
}

#define COMPUTED_ASSIGNMENT(F, BT, BI) \
template<typename T, size_t N> VTN operator F ## = (VTN& a, BT b) { \
    for (size_t i = 0; i != N; ++i) \
        a[i] F ## = b BI; \
    return a; \
}

#define MATH_OPERATORS(F) \
BINARY_INLINE(F, VTN, VTN, [i], [i]) \
BINARY_INLINE(F, VTN, T, [i], ) \
BINARY_INLINE(F, T, VTN, , [i]) \
COMPUTED_ASSIGNMENT(F, VTN, [i]) \
COMPUTED_ASSIGNMENT(F, T, )

MATH_OPERATORS(+)
MATH_OPERATORS(-)
MATH_OPERATORS(*)
MATH_OPERATORS(/)

#undef MATH_OPERATORS
#undef COMPUTED_ASSIGNMENT
#undef BINARY_INLINE
#undef VTN

template<typename T, size_t N> vec<T, N> operator-(vec<T, N> a) {
    for (size_t i = 0; i != N; ++i)
        a[i] = -a[i];
    return a;
}

template<typename T, size_t N> T dot(vec<T, N> a, vec<T, N> b) {
    T c = 0;
    for (size_t i = 0; i != N; ++i)
        c += a[i] * b[i];
    return c;
}

template<typename T> vec<T, 3> cross(vec<T, 3> a, vec<T, 3> b) {
    return vec<T, 3>{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

// length, distance, normalize (not in place)

template<typename T, size_t N> T lengthSquared(vec<T, N> a) {
    T b = T{};
    for (size_t i = 0; i != N; ++i)
        b += a[i] * a[i];
    return b;
}

template<typename T, size_t N> T length(vec<T, N> a) {
    T b = T{};
    for (size_t i = 0; i != N; ++i)
        b += a[i] * a[i];
    return sqrt(b);
}

template<typename T, size_t N> T distance(vec<T, N> a, vec<T, N> b) {
    return length(b - a);
}

template<typename T, size_t N> vec<T, N> normalize(vec<T, N> a) {
    return a / length(a);
}

template<typename T, size_t N> T prod(vec<T, N> a) {
    T b = a[0];
    for (size_t i = 1; i != N; ++i)
        b *= a[i];
    return b;
}



#endif
