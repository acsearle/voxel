//
//  framebuffer.h
//  voxel
//
//  Created by Antony Searle on 2/16/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#ifndef __voxel__framebuffer__
#define __voxel__framebuffer__

#include "Named.h"
#include "Texture.h"

class Framebuffer : public Named {
public:
    Framebuffer() {
        glGenFramebuffers(1, &name_);
    }
    ~Framebuffer() {
        glDeleteFramebuffers(1, &name_);
    }
    Framebuffer& bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, name_);
        return *this;
    }
    
    class Error : public std::exception {};
    
    Framebuffer& validate() {
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            throw Error{};
        }
        return *this;
    }
};


class ShadowBuffer : public Framebuffer {
public:
    Texture2D depth;
    GLsizei width, height;
    explicit ShadowBuffer(GLsizei width, GLsizei height)
    : depth(width, height, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT)
    , width(width)
    , height(height) {
        bind();
        GLfloat color[] = { 1.f, 1.f, 1.f, 1.f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
        validate();
    }
};

#endif /* defined(__voxel__framebuffer__) */
