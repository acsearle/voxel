//
//  voxel.h
//  voxel
//
//  Created by Antony Searle on 2/11/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#ifndef __voxel__voxel__
#define __voxel__voxel__


#include "vec.h"
#include "vertex.h"
#include "mesh.h"

class VoxelVertex {
public:
    vec<GLubyte, 3> inPosition;
    vec<GLubyte, 2> inTexcoord;
    GLubyte inAmbientOcclusion;
    static VertexDescription describe();
    VoxelVertex(GLubyte x,
                GLubyte y,
                GLubyte z,
                GLubyte s,
                GLubyte t,
                GLubyte a)
    : inPosition(x, y, z), inTexcoord(s, t), inAmbientOcclusion(a) {
    }
};


class Voxel {

public:

    typedef unsigned short type;
    
    Voxel(vec<size_t, 3> size) : data_(prod(size), 0), size_(size) {}

    type& operator()(size_t i, size_t j, size_t k) {
        return data_[i + size_[0] * (j + size_[1] * k)];
    }
    const type& operator()(size_t i, size_t j, size_t k) const {
        return data_[i + size_[0] * (j + size_[1] * k)];
    }
    
    std::unique_ptr<mesh<VoxelVertex, GLuint>> makeMesh() const;
    
    vec<size_t, 3> size() const { return size_; }
    
    vec<float, 3> raycast(vec<float, 3> a,
                        vec<float, 3> b) const;
    
    void paint() const;
    
private:
    
    std::vector<type> data_;
    vec<size_t, 3> size_;
    
};

void randomize(Voxel& v);

#endif /* defined(__voxel__voxel__) */
