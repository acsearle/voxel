//
//  vertex.cpp
//  voxel
//
//  Created by Antony Searle on 2/12/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#include "vertex.h"

using namespace std;


VertexDescription::VertexDescription() {
    GLint size;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &size);
    // Can't resize an array of noncopyable objects
    attribs.reserve(size);
    for (GLint index = 0; index != size; ++index)
        attribs.push_back(nullptr);
}

void VertexDescription::use() {
    for (GLuint index = 0; index != attribs.size(); ++index)
        if (attribs[index]) {
            glEnableVertexAttribArray(index);
            glVertexAttribPointer(index,
                                  attribs[index]->size,
                                  attribs[index]->type,
                                  attribs[index]->normalized,
                                  stride,
                                  (void*) attribs[index]->offset);
        } else {
            glDisableVertexAttribArray(index);
        }
    
}

