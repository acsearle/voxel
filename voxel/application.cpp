//
//  application.cpp
//  voxel
//
//  Created by Antony Searle on 2/10/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#include <cstdlib>
#include <fstream>

#include <OpenGL/gl3.h>

#include "application.h"
#include "shader.h"
#include "program.h"

using namespace std;

application::~application() {
}



class my_application : public application {
    
public:
    
    my_application();
    
    virtual ~my_application();
    
    virtual void render(size_t width, size_t height, int64_t time);
    
};


unique_ptr<application> application::make() {
    return unique_ptr<application>{new my_application{}};
}






my_application::my_application() {
    
    shader a(GL_VERTEX_SHADER);
    shader b(GL_FRAGMENT_SHADER);
    
    a.compile();
    
    
    program p;
    
    //p.attach(a);
    p.attach(b);
    
    
   
    
    
    
    
    
}

my_application::~my_application() {
}

void my_application::render(size_t width, size_t height, int64_t time) {
    
    glViewport(0, 0, (int) width, (int) height);
    glClearColor(rand()&1,rand()&1,rand()&1,1);
    glClear(GL_COLOR_BUFFER_BIT);
    
    
}