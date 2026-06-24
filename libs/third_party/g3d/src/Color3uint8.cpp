/**
 @file Color3uint8.cpp
 
 @author Morgan McGuire, http://graphics.cs.williams.edu
  
 @created 2003-04-07
 @edited  2006-01-07
 */

#include "g3d/platform.h"
#include "g3d/g3dmath.h"
#include "g3d/Color3uint8.h"
#include "g3d/Color3.h"
namespace G3D {

Color3uint8::Color3uint8(const class Color3& c) {
    r = iMin(255, iFloor(c.r * 256));
    g = iMin(255, iFloor(c.g * 256));
    b = iMin(255, iFloor(c.b * 256));
}



}
