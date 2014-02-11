//
//  application.h
//  voxel
//
//  Created by Antony Searle on 2/10/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#ifndef __voxel__application__
#define __voxel__application__

#include <memory>

class application {

public:
    
    static std::unique_ptr<application> make();

    virtual ~application() = 0;
    
    virtual void render(std::size_t width, std::size_t height, std::int64_t time) = 0;

};

#endif /* defined(__voxel__application__) */
