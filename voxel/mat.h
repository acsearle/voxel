//
//  mat.h
//  voxel
//
//  Created by Antony Searle on 2/11/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#ifndef voxel_mat_h
#define voxel_mat_h


#include <algorithm>
#include <memory>

#include "vec.h"

template<typename T, size_t M, size_t N> class mat {
    T data_[M * N];
public:
    mat() { std::fill_n(data_, M * N, 0); }
    explicit mat(const T* p) { std::copy_n(p, M * N, data_); }
    T* data() { return data_; }
    
    T& operator[](size_t k) {
        assert(k < M * N);
        return data_[k];
    }
    
    T& operator()(size_t i, size_t j) {
        assert(i < M);
        assert(j < N);
        return data_[i + M * j];
    }
    
    vec<T, M>& col(size_t j) {
        assert(j < N);
        return reinterpret_cast<vec<T, M>*>(this)[j];
    }
    
};

template<typename T, size_t M, size_t N> mat<T, N, M> transpose(mat<T, M, N> a) {
    mat<T, N, M> b;
    for (size_t j = 0; j != N; ++j)
        for (size_t i = 0; i != M; ++i)
            b(j, i) = a(i, j);
    return b;
}

template<typename T, size_t M, size_t N> mat<T, M, N>& operator*=(mat<T, M, N>& a, T b) {
    for (size_t i = 0; i != M * N; ++i)
        a[i] *= b;
    return a;
}

// C = A . B

template<typename T, size_t L, size_t M, size_t N> mat<T, L, N> operator*(mat<T, L, M> a, mat<T, M, N> b) {
    mat<T, L, N> c;
    for (size_t h = 0; h != L; ++h)
        for (size_t j = 0; j != N; ++j)
            for (size_t i = 0; i != M; ++i)
                c(h, j) += a(h, i) * b(i, j);
    return c;
}

// c = A . b

template<typename T, size_t M, size_t N> vec<T, M> operator*(mat<T, M, N> a, vec<T, N> b) {
    vec<T, M> c;
    for (size_t j = 0; j != N; ++j)
        c += (a.col(j) * b[j]);
    return c;
}

// C = k A

template<typename T, size_t M, size_t N> mat<T, M, N> operator*(mat<T, M, N> a, T b) {
    for (size_t k = 0; k != M * N; ++k)
        a[k] *= b;
    return a;
}

// C = I

template<typename T, size_t N> mat<T, N, N> identity() {
    mat<T, N, N> m;
    for (size_t j = 0; j != N; ++j)
        m(j, j) = 1;
    return m;
}

template<typename T, size_t N> mat<T, N, N> inverse(mat<T, N, N> a);

template<typename T> mat<T, 4, 4> translate(vec<T, 3> a) {
    mat<T, 4, 4> m;
    m = identity<T, 4>();
    m.col(3).xyz() = a;
    return m;
}




template<typename T> mat<T, 4, 4> lookat(vec<T, 3> eye,
                                         vec<T, 3> center,
                                         vec<T, 3> up);

template<typename T> mat<T, 4, 4> frustum(T left, T right, T bottom, T top, T nearZ, T farZ);

template<typename T> mat<T, 4, 4> perspective(T fovyRadians, T aspect, T nearZ, T farZ);

template<typename T> mat<T, 4, 4> rotateX(T radians) {
    mat<T, 4, 4> m;
    m = identity<T, 4>();
    T c = cos(radians);
    T s = sin(radians);
    m(1,1) = c;
    m(2,2) = c;
    m(2,1) = s;
    m(1,2) = -s;
    return m;
}

template<typename T> mat<T, 4, 4> rotateY(T radians) {
    mat<T, 4, 4> m;
    m = identity<T, 4>();
    T c = cos(radians);
    T s = sin(radians);
    m(2,2) = c;
    m(0,0) = c;
    m(0,2) = s;
    m(2,0) = -s;
    return m;
}



#endif
