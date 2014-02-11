//
//  utility.cpp
//  voxel
//
//  Created by Antony Searle on 2/10/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#include <string>
#include <fstream>

#include "utility.h"

using namespace std;

string path_for_resource(string filename, string ext)
{
    return [[[NSBundle mainBundle] pathForResource:[NSString stringWithUTF8String:filename.c_str()]
                                            ofType:[NSString stringWithUTF8String:ext.c_str()]] UTF8String];
    
}

string load(string name) {
    ifstream f{name};
    return string{istreambuf_iterator<char>{f}, istreambuf_iterator<char>{}};
}

string load(string filename, string ext) {
    return load(path_for_resource(filename, ext));
}

