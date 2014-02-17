//
//  program.cpp
//  voxel
//
//  Created by Antony Searle on 2/10/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <OpenGL/gl3.h>

#include "program.h"
#include "shader.h"
#include "utility.h"

using namespace std;




program::program() {
    name_ = glCreateProgram();
}

program::~program() {
    glDeleteProgram(name_);
}

program& program::attach(shader& s) {
    glAttachShader(name_, s);
    return *this;
}

program& program::detach(shader& s) {
    glDetachShader(name_, s);
    return *this;
}

string program::log()
{
    GLint n;
    glGetProgramiv(name_, GL_INFO_LOG_LENGTH, &n);
    vector<GLchar> s(n + 1);
    if (n)
        glGetProgramInfoLog(name_, n, &n, s.data());
    return string{s.data()};
}

program& program::link() {
    glLinkProgram(name_);
    cerr << log();
    GLint status;
    glGetProgramiv(name_, GL_LINK_STATUS, &status);
    if (!status)
        throw link_error{};
    return *this;
}

program& program::bind_attribute(GLuint index, string name) {
    glBindAttribLocation(name_, index, name.c_str());
    return *this;
}

program& program::bind_fragdata(GLuint color, string name) {
    glBindFragDataLocation(name_, color, name.c_str());
    return *this;
}

program& program::validate() {
    glValidateProgram(name_);
    cerr << log();
    GLint status;
    glGetProgramiv(name_, GL_VALIDATE_STATUS, &status);
    if (!status)
        throw validate_error();
    return *this;
}

program& program::use() {
    glUseProgram(name_);
    return *this;
}

program::uniform program::operator[](string s) {
    return operator[](s.c_str());
}

program::uniform program::operator[](const char* c) {
    GLint location = glGetUniformLocation(name_, c);
    //if (location == -1)
    //    throw exception{};
    return program::uniform(name_, location);
}

template<> program::uniform& program::uniform::operator=(mat<float, 4, 4> a) {
    glUniformMatrix4fv(location_, 1, GL_FALSE, a.data());
    return *this;
}

template<> program::uniform& program::uniform::operator=(GLint a) {
    glUniform1i(location_, a);
    return *this;
}





std::unique_ptr<program> make_program(std::string resource) {
    unique_ptr<program> p(new program);
    shader a(GL_VERTEX_SHADER);
    shader b(GL_FRAGMENT_SHADER);
    p->attach(a.source(load(resource, "vsh")).compile());
    p->attach(b.source(load(resource, "fsh")).compile());
    p->link();
    return p;
}




/*
program& program::bind_active_attributes(map<string, GLint> binding) {
    // Find the active attributes
    
    GLint n;
    glGetProgramiv(name_, GL_ACTIVE_ATTRIBUTES, &n);
    GLint len;
    glGetProgramiv(name_, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &len);
    
    vector<string> active_attributes;
    for (GLint i = 0; i != n; ++i) {
        vector<GLchar> buffer{len + 1};
        GLsizei length; GLint size; GLenum type;
        glGetActiveAttrib(name_, i, len, &length, &size, &type, buffer.data());
        active_attributes.push_back(string{buffer.data()});
    }
    
    for (string s : active_attributes) {
        auto i = binding.find(s);
        if (i != binding.end())
            glBindAttribLocation(name_, i->second, i->first.c_str());
        else
            throw bind_error{};
    }
    
    
*/
