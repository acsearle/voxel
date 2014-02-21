//
//  voxel.h
//  voxel
//
//  Created by Antony Searle on 2/11/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#ifndef __voxel__voxel__
#define __voxel__voxel__

#include <cmath>
#include <algorithm>

#include "vec.h"
#include "vertex.h"
#include "mesh.h"

using namespace std;

class VoxelVertex {
public:
    vec<GLbyte, 3> inPosition;
    vec<GLubyte, 2> inTexcoord;
    GLubyte inAmbientOcclusion;
    static VertexDescription describe();
    VoxelVertex(GLbyte x,
                GLbyte y,
                GLbyte z,
                GLubyte s,
                GLubyte t,
                GLubyte a)
    : inPosition(x, y, z), inTexcoord(s, t), inAmbientOcclusion(a) {
    }
};

class VoxelPainting;


template<typename T> class Voxel {

public:
    
    Voxel() {}
    
    Voxel(vec<ptrdiff_t, 3> lower, vec<ptrdiff_t, 3> upper)
    : data_(prod(upper - lower), 0)
    , min_(lower)
    , max_(upper) {
        stride_[0] = 1;
        stride_[1] = (max_[0] - min_[0]);
        stride_[2] = (max_[1] - min_[1]) * stride_[1];
        offset_ = - dot(min_, stride_);
    }

    T get(vec<ptrdiff_t, 3> x) const {
        for (size_t i = 0; i != 3; ++i)
            if ((x[i] < min_[i]) || (max_[i] <= x[i]))
                return T{};
        return data_[offset_ + dot(x, stride_)];
    }
    
    T get(ptrdiff_t i, ptrdiff_t j, ptrdiff_t k) const {
        return get(vec<ptrdiff_t, 3>(i,j,k));
    }

    void set(vec<ptrdiff_t, 3> x, T t) {
        for (size_t i = 0; i != 3; ++i)
            assert((min_[i] <= x[i]) && (x[i] < max_[i]));
        data_[offset_ + dot(x, stride_)] = t;
    }
    
    void set(ptrdiff_t i, ptrdiff_t j, ptrdiff_t k, T t) {
        return set(vec<ptrdiff_t, 3>(i,j,k), t);
    }

    void push(vec<ptrdiff_t, 3> x, T t) {
        // do we need to grow?
        for (size_t q = 0; q != 3; ++q)
        if ((x[q] < min_[q]) || (max_[q] <= x[q]))
        {
            vec<ptrdiff_t, 3> a {
                min(min_[0], x[0]-1),
                min(min_[1], x[1]-1),
                min(min_[2], x[2]-1)
            };
            vec<ptrdiff_t, 3> b {
                max(max_[0], x[0]+1),
                max(max_[1], x[1]+1),
                max(max_[2], x[2]+1)
            };
            Voxel<T> v {a, b};
            
            for (ptrdiff_t k = min_[2]; k != max_[2]; ++k)
            for (ptrdiff_t j = min_[1]; j != max_[1]; ++j)
            for (ptrdiff_t i = min_[0]; i != max_[0]; ++i)
            v.set(i,j,k,get(i,j,k));
            v.set(x, t);
            swap(*this, v);
            return;
        }
        // we don't need to grow
        set(x, t);
    }
    
    std::unique_ptr<mesh<VoxelVertex, GLuint>> makeMesh() const;
    
    vec<ptrdiff_t, 3> lower() const { return min_; }
    vec<ptrdiff_t, 3> upper() const { return max_; }
    
    vec<float, 3> raycast(vec<float, 3> a,
                        vec<float, 3> b) const;
    
    VoxelPainting paint() const;
    
private:
    
    std::vector<T> data_;
    vec<ptrdiff_t, 3> max_;
    vec<ptrdiff_t, 3> min_;
    vec<ptrdiff_t, 3> stride_;
    ptrdiff_t offset_;
    
};


struct VoxelPainting {
    Voxel<short> p;
    vector<short> mapping;
    vector<short> unique;
};

void randomize(Voxel<char>& v);
void surface(Voxel<char>& v);


#endif /* defined(__voxel__voxel__) */
