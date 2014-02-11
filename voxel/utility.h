//
//  utility.h
//  voxel
//
//  Created by Antony Searle on 2/10/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#ifndef __voxel__utility__
#define __voxel__utility__

#include <string>
#include <exception>

std::string load(std::string filename, std::string ext);

class resource_not_found_error : public std::exception {};


void opengl_health();
class opengl_error : public std::exception {};


#endif /* defined(__voxel__utility__) */
