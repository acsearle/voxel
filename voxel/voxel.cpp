//
//  voxel.cpp
//  voxel
//
//  Created by Antony Searle on 2/11/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#include <vector>
#include <iostream>

#include "voxel.h"
#include "vec.h"
#include "vertex.h"
#include "mesh.h"

using namespace std;

VertexDescription VoxelVertex::describe() {
    VertexDescription a;
    a.stride = sizeof(VoxelVertex);
    a.attribs[0].reset(new VertexAttributeDescription{
        3, GL_BYTE, GL_FALSE, offsetof(VoxelVertex, inPosition)});
    a.attribs[8].reset(new VertexAttributeDescription{
        2, GL_UNSIGNED_BYTE, GL_FALSE, offsetof(VoxelVertex, inTexcoord)});
    a.attribs[4].reset(new VertexAttributeDescription{
        1, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(VoxelVertex, inAmbientOcclusion)});
    return a;
}

template<> unique_ptr<mesh<VoxelVertex, GLuint>> Voxel<char>::makeMesh() const {
    typedef char type;
    unique_ptr<mesh<VoxelVertex, GLuint>> m{new mesh<VoxelVertex, GLuint>{}};
    for (ptrdiff_t k = min_[2]; k != max_[2]; ++k)
    for (ptrdiff_t j = min_[1]; j != max_[1]; ++j)
    for (ptrdiff_t i = min_[0]; i != max_[0]; ++i)
    if (type r = get(i, j, k))
    {
        GLubyte u, v;
        u = 0;
        v = r;
        
        auto emit = [&](GLubyte x,
                        GLubyte y,
                        GLubyte z,
                        GLubyte s,
                        GLubyte t) {
            m->vertices.push_back(VoxelVertex(x+i,y+j,z+k,s+u,t+v, rand()));
        };
        // x == 0
        if (!get(i-1,j,k)) {
            emit(0,0,0,0,0);
            emit(0,0,1,1,0);
            emit(0,1,1,1,1);
            emit(0,1,0,0,1);
        }
        // y == 0
        if (!get(i,j-1,k)) {
            emit(0,0,0,1,0);
            emit(1,0,0,2,0);
            emit(1,0,1,2,1);
            emit(0,0,1,1,1);
        }
        // z == 0
        if (!get(i,j,k-1)) {
            emit(0,0,0,2,0);
            emit(0,1,0,2,1);
            emit(1,1,0,3,1);
            emit(1,0,0,3,0);
        }
        // x == 1
        if (!get(i+1,j,k)) {
            emit(1,0,0,3,0);
            emit(1,1,0,3,1);
            emit(1,1,1,4,1);
            emit(1,0,1,4,0);
        }
        // y == 1
        if (!get(i,j+1,k)) {
            emit(0,1,0,4,0);
            emit(0,1,1,4,1);
            emit(1,1,1,5,1);
            emit(1,1,0,5,0);
        }
        // z == 1
        if (!get(i,j,k+1)) {
            emit(0,0,1,5,0);
            emit(1,0,1,6,0);
            emit(1,1,1,6,1);
            emit(0,1,1,5,1);
        }
    }
    for (GLuint i = 0; i != m->vertices.size(); i += 4) {
        m->elements.push_back(i);
        m->elements.push_back(i+1);
        m->elements.push_back(i+2);
        m->elements.push_back(i);
        m->elements.push_back(i+2);
        m->elements.push_back(i+3);
    }
    return m;
}

void randomize(Voxel<char>& v) {
    for (ptrdiff_t k = v.lower()[2]; k != v.upper()[2]; ++k)
    for (ptrdiff_t j = v.lower()[1]; j != v.upper()[1]; ++j)
    for (ptrdiff_t i = v.lower()[0]; i != v.upper()[0]; ++i)
    v.set(vec<ptrdiff_t, 3>(i, j, k), !(rand() % 3) ? rand() % 8 : 0);
}

void surface(Voxel<char>& v) {
    for (ptrdiff_t k = v.lower()[2]; k != v.upper()[2]; ++k)
    for (ptrdiff_t j = v.lower()[1]; j != v.upper()[1]; ++j)
    for (ptrdiff_t i = v.lower()[0]; i != v.upper()[0]; ++i)
    {
        if (j < 8 + (sin(i * 0.5) + cos(k * 0.5)))
        v.set(vec<ptrdiff_t, 3>(i, j, k), 2);
    }
}

/*
 template<> vec<float, 3> Voxel<char>::raycast(vec<float, 3> a,
 vec<float, 3> b) const {
 size_t i, j, k;
 do {
 float maxInvT = 0;
 int iBest = 0;
 for (int i = 0; i != 3; ++i) {
 if (b[i]) { // can't intersect otherwise
 float c;
 if (b[i] > 0) {
 c = ceil(a[i]);
 if (c == a[i])
 c += 1;
 } else {
 c = floor(a[i]);
 if (c == a[i])
 c -= 1;
 }
 float invT = b[i] / (c - a[i]);
 if (invT > maxInvT) {
 iBest = i;
 maxInvT = invT;
 }
 }
 }
 for (int i = 0; i != 3; ++i) {
 a[i] += b[i] / maxInvT;
 }
 
 
 a[iBest] = round(a[iBest]);
 vec<float, 3> c = a;
 for (size_t i = 0; i != 3; ++i)
 {
 if ((b[i] < 0) && (a[i] == round(a[i]))) c[i] -= 1;
 }
 
 i = c[0];
 j = c[1];
 k = c[2];
 } while ((i < size_[0]) && (j < size_[1]) && (k < size_[2]) && !get(i, j, k));
 
 
 return a;
 }
 */




template<> VoxelPainting Voxel<char>::paint() const {
    
    // tag each voxel.  a level of indirection allows us to merge regions as
    // we discover their conections, without repainting
    Voxel<short> p(min_, max_);
    vector<short> mapping(1, 0);
    
    
    auto merge = [&](short a, short b) {
        // scan through the mappings redirecting things that point to b to
        // point to a
        if (b < a)
        swap(a, b);
        for (auto& c : mapping)
        if (c == b)
        c = a;
        // we could track the reductions performed here and if the colors
        // become too sparse, pause to recolor the voxels and compact the
        // mapping
    };
    
    for (ptrdiff_t k = min_[2]; k != max_[2]; ++k)
    for (ptrdiff_t j = min_[1]; j != max_[1]; ++j)
    for (ptrdiff_t i = min_[0]; i != max_[0]; ++i)
    if (get(i,j,k))
    {
        size_t h = 0;
        size_t r[3];
        
        auto append = [&](size_t s) {
            if (s) {
                for (size_t i = 0; i != h; ++i)
                if (s == r[i])
                return;
                r[h++] = s;
            }
        };
        
        append(mapping[p.get(i-1,j,k)]);
        append(mapping[p.get(i,j-1,k)]);
        append(mapping[p.get(i,j,k-1)]);
        
        if (h == 0) {
            // create a new color, mapped to itself
            p.set(i,j,k, mapping.size());
            mapping.push_back(mapping.size());
        } else if (h == 1) {
            // one neighbour, adopt its mapped color
            p.set(i, j, k, r[0]);
        } else if (h == 2) {
            merge(r[0], r[1]);
            p.set(i, j, k, mapping[r[0]]); // may have changed
        } else { // h == 3
            merge(r[0], r[1]);
            merge(mapping[r[0]], r[2]);
            p.set(i, j, k, mapping[r[0]]);
            
        }
    }
    vector<short> unique;
    for (size_t i = 1; i != mapping.size(); ++i) {
        if (mapping[i] == i)
        unique.push_back(i);
    }
    
    VoxelPainting q;
    swap(q.p, p);
    q.mapping.swap(mapping);
    q.unique.swap(unique);
    return q;
    
}

