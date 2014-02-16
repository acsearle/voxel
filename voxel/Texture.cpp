//
//  Texture.cpp
//  voxel
//
//  Created by Antony Searle on 2/12/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#include "Texture.h"

#include <vector>

#include "Image.h"
#include "vec.h"

using namespace std;


template<typename T> T clamp(T x, T a, T b) {
    return a < x ? x < b ? x : b : a;
}

Image::Image(size_t width, size_t height)
: vector_(width * height)
, width_(width)
, height_(height) {
}


unique_ptr<Image> makeImageNormals() {
    auto shrink = [](float x, float y, float z) {
        return vec<GLubyte, 4>{
            (GLubyte) 255*clamp(x * 0.5f + 0.5f, 0.0f, 1.0f),
            (GLubyte) 255*clamp(y * 0.5f + 0.5f, 0.0f, 1.0f),
            (GLubyte) 255*clamp(z * 0.5f + 0.5f, 0.0f, 1.0f),
            (GLubyte) 255*clamp(0 * 0.5f + 0.5f, 0.0f, 1.0f)
        };
    };
    
    auto img = unique_ptr<Image>{new Image{256, 256}};
    for (int v = 16; v != 128; v += 16)
    for (int i = 0; i != 16; ++i)
        for (int j = 0; j != 16; ++j)
        {
            (*img)(i   , j+v) = shrink(-1,0,0);
            (*img)(i+16, j+v) = shrink(0,-1,0);
            (*img)(i+32, j+v) = shrink(0,0,-1);
            (*img)(i+48, j+v) = shrink(+1,0,0);
            (*img)(i+64, j+v) = shrink(0,+1,0);
            (*img)(i+80, j+v) = shrink(0,0,+1);
        }
    return img;
}

unique_ptr<Image> makeImageTextures() {
    
    auto img = unique_ptr<Image>{new Image{256, 256}};
    GLubyte r, g, b;
    for (int j = 0; j != 256; ++j)
        for (int i = 0; i != 256; ++i)
        {
            if (!(j & 0xF)) {
                r = 128 + rand() % 64;
                g = 128 + rand() % 64;
                b = 128 + rand() % 64;
                
                rand();
                
            }
            //if (((i & 0xF) && (j & 0xF)) && (((i+1) & 0xF) && ((j+1) & 0xF)))
                (*img)(i, j) = vec<GLubyte, 4>(r + rand() % 64,
                                               g + rand() % 64,
                                               b + rand() % 64, 0);
        }
    return img;
}




unique_ptr<Texture> makeTexture(Image& img) {
    auto t = unique_ptr<Texture>{new Texture{}};
    glBindTexture(GL_TEXTURE_2D, *t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
    
    glTexImage2D(GL_TEXTURE_2D, 0, img.format(), (GLsizei) img.width(), (GLsizei) img.height(), 0, img.format(), img.type(), &img(0,0));
    return t;
}

/*

 ss texture2d : public texture {
 public:
 GLsizei format_, type_;
 texture2d(GLsizei width, GLsizei height, GLenum format, GLenum type) : format_(format), type_(type) {
 bind();
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
 glTexImage2D(GL_TEXTURE_2D, 0, format_, width, height, 0, format_, type_, NULL);
 }
 texture2d& bind() {
 glBindTexture(GL_TEXTURE_2D, name_);
 return *this;
 }
 texture2d& generateMipmap() {
 glGenerateMipmap(GL_TEXTURE_2D);
 return *this;
 }
 texture2d& resize(GLsizei width, GLsizei height) {
 glTexImage2D(GL_TEXTURE_2D, 0, format_, width, height, 0, format_, type_, NULL);
 return *this;
 }
 };
 

*/