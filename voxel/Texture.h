//
//  Texture.h
//  voxel
//
//  Created by Antony Searle on 2/12/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#ifndef __voxel__Texture__
#define __voxel__Texture__

#include <memory>

#include "Image.h"
#include "named.h"

class Texture : public Named {
public:
    Texture() {
        glGenTextures(1, &name_);
    }
    ~Texture() {
        glDeleteTextures(1, &name_);
    }
};

class Texture2D : public Texture {
public:
    Texture2D(GLsizei width, GLsizei height, GLenum format, GLenum type) {
        glBindTexture(GL_TEXTURE_2D, name_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, nullptr);
    }
    Texture2D& bind() {
        glBindTexture(GL_TEXTURE_2D, name_);
        return *this;
    }
};

std::unique_ptr<Texture> makeTexture(Image&);

std::unique_ptr<Image> makeImageNormals();
std::unique_ptr<Image> makeImageTextures();



#endif /* defined(__voxel__Texture__) */
