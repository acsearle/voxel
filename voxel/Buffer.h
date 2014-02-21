//
//  Buffer.h
//  voxel
//
//  Created by Antony Searle on 2/19/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#ifndef __voxel__Buffer__
#define __voxel__Buffer__

#include <vector>

#include <OpenGL/gl3.h>

#include "Named.h"


class Buffer : public Named {
public:
    Buffer() { glGenBuffers(1, &name_); }
    ~Buffer() { glDeleteBuffers(1, &name_); }
    Buffer& bind(GLenum target) { glBindBuffer(target, name_); return *this;}
    template<typename T> Buffer& data(GLenum target, const std::vector<T>& data, GLenum usage) {
        bind(target);
        glBufferData(target, sizeof(T) * data.size(), data.data(), usage);
        return *this;
    }
};

class ArrayBuffer : public Buffer {
public:
    ArrayBuffer& bind() {
        Buffer::bind(GL_ARRAY_BUFFER);
        return *this;
    }
    template<typename T> ArrayBuffer& data(const std::vector<T>& v) {
        Buffer::data(GL_ARRAY_BUFFER, v, GL_STATIC_DRAW);
        return *this;
    }
};

class ElementArrayBuffer : public Buffer {
public:
    ElementArrayBuffer& bind() {
        Buffer::bind(GL_ELEMENT_ARRAY_BUFFER);
        return *this;
    }
    template<typename T> ElementArrayBuffer& data(const std::vector<T>& v) {
        Buffer::data(GL_ELEMENT_ARRAY_BUFFER, v, GL_STATIC_DRAW);
        return *this;
    }
};


class UniformBufferObject : public Buffer {
    UniformBufferObject& bind() {
        Buffer::bind(GL_UNIFORM_BUFFER);
        return *this;
    }
    template<typename T> UniformBufferObject& data(const T& t) {
        bind();
        glBufferData(GL_UNIFORM_BUFFER, sizeof(T), &t, GL_STATIC_DRAW);
    }
};


class VertexArrayObject : public Named {
public:
    VertexArrayObject() { glGenVertexArrays(1, &name_); }
    ~VertexArrayObject() { glDeleteVertexArrays(1, &name_); }
    VertexArrayObject& bind() { glBindVertexArray(name_); return *this;}
    
};


#endif /* defined(__voxel__Buffer__) */
