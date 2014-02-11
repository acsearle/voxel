//
//  vec.h
//  voxel
//
//  Created by Antony Searle on 2/11/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#ifndef voxel_vec_h
#define voxel_vec_h

template<typename T, size_t N> class vec;

template<typename T> class vec<T, 3> {
    union {
        T data_[3];
        struct { T x, y, z; };
    };
public:
    vec() : x(0), y(0), z(0) {}
    vec(T x, T y, T z) : x(x), y(y), z(z) {}
    T& operator[](size_t i) { return data_[i]; }
};



#endif
