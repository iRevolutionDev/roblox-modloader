/**
 @file Vector2int16.cpp
 
 @author Morgan McGuire, http://graphics.cs.williams.edu
  
 @created 2003-08-09
 @edited  2006-01-29
 */

#include "RobloxModLoader/roblox/g3d/platform.h"
#include "RobloxModLoader/roblox/g3d/g3dmath.h"
#include "RobloxModLoader/roblox/g3d/Vector2int16.h"
#include "RobloxModLoader/roblox/g3d/Vector2.h"

namespace G3D {

Vector2int16::Vector2int16(const class Vector2& v) {
    x = (int16)iFloor(v.x + 0.5);
    y = (int16)iFloor(v.y + 0.5);
}

Vector2int16::Vector2int16(int coordinate[2]) {
    x = (int16)coordinate[0];
    y = (int16)coordinate[1];
}



Vector2int16 Vector2int16::clamp(const Vector2int16& lo, const Vector2int16& hi) {
    return Vector2int16(iClamp(x, lo.x, hi.x), iClamp(y, lo.y, hi.y));
}

Vector2int16 Vector2int16::xx() const {
	return Vector2int16(x,x);
}

Vector2int16 Vector2int16::xy() const {
	return Vector2int16(x,y);
}

Vector2int16 Vector2int16::yx() const {
	return Vector2int16(y,x);
}

Vector2int16 Vector2int16::yy() const {
	return Vector2int16(y,y);
}

}
