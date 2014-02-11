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

#include "shader.h"

class program {

    GLuint name_;

public:

    program();
    ~program();
    
    program& attach(shader& s);
    program& detach(shader& s);
    
    program& compile();
    
    program& link();
    
    program& validate();
    
    program& use();
    
    program& bind();
    
    std::string log();
    
    class link_error : public std::exception {
    };
    
    class validate_error : public std::exception {
    };
    
    class bind_error : public std::exception {
    };
    
};


#endif /* defined(__voxel__program__) */
