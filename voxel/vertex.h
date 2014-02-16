//
//  vertex.h
//  voxel
//
//  Created by Antony Searle on 2/12/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#ifndef __voxel__vertex__
#define __voxel__vertex__

#include <memory>
#include <vector>

#include <OpenGL/gl3.h>

struct VertexAttributeDescription {
    GLint size;
    GLenum type;
    GLboolean normalized;
    size_t offset;
};

struct VertexDescription {
    GLsizei stride;
    std::vector<std::unique_ptr<VertexAttributeDescription>> attribs;
    VertexDescription();
    void use();
};




#endif /* defined(__voxel__vertex__) */
