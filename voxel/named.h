//
//  named.h
//  voxel
//
//  Created by Antony Searle on 2/10/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#ifndef __voxel__named__
#define __voxel__named__

#include <OpenGL/gl3.h>

class named {
public:
    named() : name_(0) {}
    operator GLuint() { return name_; }
protected:
    GLuint name_;
private:
    named(const named&);
    named& operator=(const named&);
};

#endif /* defined(__voxel__named__) */
