//
//  mat.h
//  voxel
//
//  Created by Antony Searle on 2/11/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#ifndef voxel_mat_h
#define voxel_mat_h

#include <memory>

template<typename T, size_t M, size_t N> class mat {
    T data_[M * N];
public:
    mat() {
        std::fill_n(data_, M * N, T{});
    }
    explicit mat(T* p) {
        std::copy(p, M * N, data_);
    }
    T* data() { return data_; }
    T& operator()(size_t i, size_t j) {
        return data_[i + M * j];
    }
};

template<typename T, size_t N> mat<T, N, N> identity() {
    mat<T, N, N> m;
    for (size_t j = 0; j != N; ++j)
        m(j, j) = 1;
    return m;
}

#endif
