//
//  shader.h
//  voxel
//
//  Created by Antony Searle on 2/10/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#ifndef __voxel__shader__
#define __voxel__shader__

#include <string>
#include <vector>

#include "named.h"


class shader : public named
{

public:
    
    explicit shader(GLenum type);
    ~shader();
    
    shader& source(std::string src);
    shader& source(std::vector<std::string> src);
    
    shader& compile();
    
    class compile_error : public std::exception {
    };
    
    const GLenum vertex = GL_VERTEX_SHADER;
    const GLenum fragment = GL_FRAGMENT_SHADER;
    
};


#endif /* defined(__voxel__shader__) */
