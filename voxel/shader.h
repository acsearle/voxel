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
    
    void source(std::vector<std::string> src);
    
    void compile();
    
    class compile_error : public std::exception {
    };
    
};


#endif /* defined(__voxel__shader__) */
