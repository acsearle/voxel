//
//  Image.h
//  voxel
//
//  Created by Antony Searle on 2/12/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#ifndef __voxel__Image__
#define __voxel__Image__

#include <OpenGL/gl3.h>

#include <vector>

#include "vec.h"

class Image {
    
    std::vector<vec<GLubyte, 4> > vector_;
    size_t width_;
    size_t height_;

public:

    Image(size_t width, size_t height);
    vec<GLubyte, 4>& operator()(int u, int v) {
        return vector_[u + v * width_];
    }
    
    size_t width() const { return width_; }
    size_t height() const { return height_; }
    GLint format() const { return GL_RGBA; }
    GLenum type() const { return GL_UNSIGNED_BYTE; }
    
};


#endif /* defined(__voxel__Image__) */
