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
        3, GL_UNSIGNED_BYTE, GL_FALSE, offsetof(VoxelVertex, inPosition)});
    a.attribs[8].reset(new VertexAttributeDescription{
        2, GL_UNSIGNED_BYTE, GL_FALSE, offsetof(VoxelVertex, inTexcoord)});
    a.attribs[4].reset(new VertexAttributeDescription{
        1, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(VoxelVertex, inAmbientOcclusion)});
    return a;
}

unique_ptr<mesh<VoxelVertex, GLuint>> Voxel::makeMesh() const {
    unique_ptr<mesh<VoxelVertex, GLuint>> m{new mesh<VoxelVertex, GLuint>{}};
    for (size_t k = 0; k != size_[2]; ++k)
        for (size_t j = 0; j != size_[1]; ++j)
            for (size_t i = 0; i != size_[0]; ++i)
                if (type r = operator()(i, j, k))
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
                    emit(0,0,0,0,0);
                    emit(0,0,1,1,0);
                    emit(0,1,1,1,1);
                    emit(0,1,0,0,1);
                    // y == 0
                    emit(0,0,0,1,0);
                    emit(1,0,0,2,0);
                    emit(1,0,1,2,1);
                    emit(0,0,1,1,1);
                    // z == 0
                    emit(0,0,0,2,0);
                    emit(0,1,0,2,1);
                    emit(1,1,0,3,1);
                    emit(1,0,0,3,0);
                    // x == 1
                    emit(1,0,0,3,0);
                    emit(1,1,0,3,1);
                    emit(1,1,1,4,1);
                    emit(1,0,1,4,0);
                    // y == 1
                    emit(0,1,0,4,0);
                    emit(0,1,1,4,1);
                    emit(1,1,1,5,1);
                    emit(1,1,0,5,0);
                    // z == 1
                    emit(0,0,1,5,0);
                    emit(1,0,1,6,0);
                    emit(1,1,1,6,1);
                    emit(0,1,1,5,1);
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

void randomize(Voxel& v) {
    for (size_t k = 0; k != v.size()[2]; ++k)
        for (size_t j = 0; j != v.size()[1]; ++j)
            for (size_t i = 0; i != v.size()[0]; ++i)
                v(i, j, k) = !(rand() & 0x3) ? rand() % 8 : 0;
}



vec<float, 3> Voxel::raycast(vec<float, 3> a,
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
    } while ((i < size_[0]) && (j < size_[1]) && (k < size_[2]) && !operator()(i, j, k));
    
    
    return a;
}

void Voxel::paint() const {

    // tag each voxel.  a level of indirection allows us to merge regions as
    // we discover their conections, without repainting
    Voxel p(size_);
    vector<size_t> mapping(1, 0);
    
    
    auto merge = [&](type a, type b) {
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
    
    for (size_t k = 0; k != size_[2]; ++k)
        for (size_t j = 0; j != size_[1]; ++j)
            for (size_t i = 0; i != size_[0]; ++i)
                if (operator()(i,j,k))
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
                    
                    if (i > 0)
                        append(mapping[p(i-1,j,k)]);
                    if (j > 0)
                        append(mapping[p(i,j-1,k)]);
                    if (k > 0)
                        append(mapping[p(i,j,k-1)]);

                    if (h == 0) {
                        // create a new color, mapped to itself
                        p(i,j,k) = mapping.size();
                        mapping.push_back(mapping.size());
                    } else if (h == 1) {
                        // one neighbour, adopt its mapped color
                        p(i, j, k) = r[0];
                    } else if (h == 2) {
                        merge(r[0], r[1]);
                        p(i, j, k) = mapping[r[0]]; // may have changed
                    } else { // h == 3
                        merge(r[0], r[1]);
                        merge(mapping[r[0]], r[2]);
                        p(i, j, k) = mapping[r[0]];
                        
                    }
                }
    vector<size_t> unique;
    for (size_t i = 1; i != mapping.size(); ++i) {
        if (mapping[i] == i)
            unique.push_back(i);
    }
    
    /*
    for (int i = 0; i != p.data_.size(); ++i) {
        if (mapping[p.data_[i]] != unique[3])
            data_[i] = 0;
    }
     */
    
}
