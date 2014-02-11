//
//  application.cpp
//  voxel
//
//  Created by Antony Searle on 2/10/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#include <cstdlib>
#include <fstream>
#include <iostream>

#include <OpenGL/gl3.h>


#include "application.h"
#include "shader.h"
#include "program.h"
#include "utility.h"
#include "mat.h"
#include "vec.h"
#include "mesh.h"

using namespace std;

application::~application() {
}





class my_vertex {
    
public:
    
    typedef vec<float, 3> position_type;
    
    position_type position;
    
};


template<typename T> class buffer : public named {
public:
    buffer() { glGenBuffers(1, &name_); }
    ~buffer() { glDeleteBuffers(1, &name_); }
    buffer& bind(GLenum target) { glBindBuffer(target, name_); return *this;}
    buffer& data(GLenum target, vector<T> data, GLenum usage) {
        glBufferData(target, sizeof(T) * data.size(), data.data(), usage);
        return *this;
    }
};

#define OFFSET(t, d) (char*) nullptr + offsetof(t, d)

class vertex_array : public named {
public:
    vertex_array() { glGenVertexArrays(1, &name_); }
    ~vertex_array() { glDeleteVertexArrays(1, &name_); }
    vertex_array& bind() { glBindVertexArray(name_); return *this;}
    
    vertex_array& point(buffer<my_vertex>& buf) {
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,        // from choice of bindings
                              3,        // from my_vertex ... vec<float, 3>
                              GL_FLOAT, // from my_vertex ... vec<float, 3>
                              GL_FALSE, // normalize?
                              sizeof(my_vertex), // stride
                              OFFSET(my_vertex, position));
        return *this;
    }
};




class my_application : public application {
    
public:
    
    my_application();
    
    virtual ~my_application();
    
    virtual void render(size_t width, size_t height, int64_t time);
    
private:
    
    unique_ptr<program> p;
    
    buffer<my_vertex> buf;
    buffer<GLuint> elm;
    vertex_array vao;
    GLsizei count;
    
};


unique_ptr<application> application::make() {
    return unique_ptr<application>{new my_application{}};
}




my_application::my_application() {
    
    cout << glGetString(GL_RENDERER) << endl;
    cout << glGetString(GL_VERSION) << endl;
    
    
    p = make_program("basic");
    
    
    auto m = mesh<my_vertex, GLuint>::make_cube();
    
    buf.bind(GL_ARRAY_BUFFER).data(GL_ARRAY_BUFFER, m->vertices, GL_STATIC_DRAW);

    vao.bind().point(buf);
    
    p->validate();
    
    p->use();
    
    elm.bind(GL_ELEMENT_ARRAY_BUFFER).data(GL_ELEMENT_ARRAY_BUFFER, m->elements, GL_STATIC_DRAW);
    
    count = (GLsizei) m->elements.size();
    
    float identity4[] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1 };
    
    opengl_health();

    glUniformMatrix4fv(glGetUniformLocation(*p,
                                            "cameraProjectionMatrix"),
                       1,
                       GL_FALSE,
                       identity<float, 4>().data());
    glUniformMatrix4fv(glGetUniformLocation(*p,
                                            "cameraViewMatrix"),
                       1,
                       GL_FALSE,
                       identity<float, 4>().data());
    glUniformMatrix4fv(glGetUniformLocation(*p,
                                            "modelMatrix"),
                       1,
                       GL_FALSE,
                       identity<float, 4>().data());
    
    opengl_health();
    
    
    
   
    
    
    
    
    
}

my_application::~my_application() {
}

void my_application::render(size_t width, size_t height, int64_t time) {
    
    glViewport(0, 0, (int) width, (int) height);
    glClearColor(rand()&1,rand()&1,rand()&1,1);
    glClear(GL_COLOR_BUFFER_BIT);

    opengl_health();

    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
    
    opengl_health();

    
    
}