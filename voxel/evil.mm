//
//  evil.cpp
//  voxel
//
//  Created by Antony Searle on 2/11/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#include "vec.h"
#include "mat.h"

// Try and reduce dependencies on GLKit

#import <GLKit/GLKit.h>

template<> mat<float, 4, 4> perspective(float fovyRadians, float aspect, float nearZ, float farZ) {
    return mat<float, 4, 4>(GLKMatrix4MakePerspective(fovyRadians, aspect, nearZ, farZ).m);
}

template<> mat<float, 4, 4> lookat(vec<float, 3> eye, vec<float, 3> center, vec<float, 3> up) {
    return mat<float, 4, 4>(GLKMatrix4MakeLookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, up.x, up.y, up.z).m);
}


template<> mat<float, 4, 4> inverse(mat<float, 4, 4> a) {
    bool b;
    return mat<float, 4, 4>(GLKMatrix4Invert(reinterpret_cast<GLKMatrix4&>(a), &b).m);
}

