//
//  shader.cpp
//  voxel
//
//  Created by Antony Searle on 2/10/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#include <iostream>
#include <OpenGL/gl3.h>

#include "shader.h"

using namespace std;

shader::shader(GLenum type) {
    name_ = glCreateShader(type);
}

shader::~shader() {
    glDeleteShader(name_);
}

shader& shader::source(string src) {
    return source(vector<string>{1, src});
}

shader& shader::source(vector<string> src) {
    vector<GLchar*> pointers;
    vector<GLint> sizes;
    for (auto& s : src) {
        pointers.push_back((GLchar*) s.data());
        sizes.push_back((GLint) s.size());
    }
    glShaderSource(name_, (GLsizei) src.size(), pointers.data(), sizes.data());
    return *this;
}

shader& shader::compile() {
    glCompileShader(name_);
    GLsizei n;
    glGetShaderiv(name_, GL_INFO_LOG_LENGTH, &n);
    if (n > 0)
    {
        string s;
        s.resize(n);
        glGetShaderInfoLog(name_, n, &n, (GLchar*) s.data());
        cerr << s;
    }
    GLint status;
    glGetShaderiv(name_, GL_COMPILE_STATUS, &status);
    if (!status)
        throw compile_error{};
    return *this;
}
