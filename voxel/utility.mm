//
//  utility.cpp
//  voxel
//
//  Created by Antony Searle on 2/10/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#include <string>
#include <fstream>
#include <iostream>

#include <OpenGL/gl3.h>

#include "utility.h"


using namespace std;

string path_for_resource(string filename, string ext)
{
    NSString* path = [[NSBundle mainBundle] pathForResource:[NSString stringWithUTF8String:filename.c_str()]
                                                     ofType:[NSString stringWithUTF8String:ext.c_str()]];
    if (!path)
        throw resource_not_found_error{};
    return [path UTF8String];
}

string load(string name) {
    ifstream f{name};
    if (!f.good())
        throw std::exception();
    return string{istreambuf_iterator<char>{f}, istreambuf_iterator<char>{}};
}

string load(string filename, string ext) {
    return load(path_for_resource(filename, ext));
}



const char * string_from_error_code(GLenum error)
{
	const char *str;
	switch( error )
	{
		case GL_NO_ERROR:
			str = "GL_NO_ERROR";
			break;
		case GL_INVALID_ENUM:
			str = "GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			str = "GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			str = "GL_INVALID_OPERATION";
			break;
		case GL_OUT_OF_MEMORY:
			str = "GL_OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			str = "GL_INVALID_FRAMEBUFFER_OPERATION";
			break;
		default:
			str = "(ERROR: Unknown Error Enum)";
			break;
	}
	return str;
}


void opengl_health() {
    GLenum e = glGetError();
    if (e != GL_NO_ERROR) {
        cerr << string_from_error_code(e) << endl;
        throw opengl_error{};
    }
}

