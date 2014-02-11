//
//  program.h
//  voxel
//
//  Created by Antony Searle on 2/10/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#ifndef __voxel__program__
#define __voxel__program__

#include <OpenGL/gl3.h>

#include "mat.h"
#include "shader.h"
#include "vec.h"

class program : public named {

public:

    program();
    ~program();
    
    program& attach(shader& s);
    program& detach(shader& s);
    
    program& compile();
    
    program& link();
    
    program& validate();
    
    program& use();
    
    program& bind_attribute(GLuint index, std::string name);
    program& bind_fragdata(GLuint color, std::string name);
    
    std::string log();
    
    class link_error : public std::exception {};
    
    class validate_error : public std::exception {};
    
    class bind_error : public std::exception {};
    
    class uniform {
        GLuint program_;
        GLint location_;
    public:
        
    };
    
};

std::unique_ptr<program> make_program(std::string resource);

#endif /* defined(__voxel__program__) */
