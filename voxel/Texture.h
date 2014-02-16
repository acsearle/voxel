//
//  Texture.h
//  voxel
//
//  Created by Antony Searle on 2/12/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#ifndef __voxel__Texture__
#define __voxel__Texture__

#include <memory>

#include "Image.h"
#include "named.h"

class Texture : public named {
public:
    Texture() {
        glGenTextures(1, &name_);
    }
    ~Texture() {
        glDeleteTextures(1, &name_);
    }
};

std::unique_ptr<Texture> makeTexture(Image&);

std::unique_ptr<Image> makeImageNormals();
std::unique_ptr<Image> makeImageTextures();



#endif /* defined(__voxel__Texture__) */
