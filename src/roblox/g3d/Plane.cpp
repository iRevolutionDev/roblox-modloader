/**
 @file Plane.cpp
 
 @maintainer Morgan McGuire, http://graphics.cs.williams.edu
 
 @created 2003-02-06
 @edited  2006-01-29
 */

#include "RobloxModLoader/roblox/g3d/platform.h"
#include "RobloxModLoader/roblox/g3d/Plane.h"
#include "RobloxModLoader/roblox/g3d/stringutils.h"

namespace G3D {

Plane::Plane(
    Vector4      point0,
    Vector4      point1,
    Vector4      point2) {

    debugAssertM(
        point0.w != 0 || 
        point1.w != 0 || 
        point2.w != 0,
        "At least one point must be finite.");

    // Rotate the points around so that the finite points come first.

    while ((point0.w == 0) && 
           ((point1.w == 0) || (point2.w != 0))) {
        Vector4 temp = point0;
        point0 = point1;
        point1 = point2;
        point2 = temp;
    }

    Vector3 dir1;
    Vector3 dir2;

    if (point1.w == 0) {
        // 1 finite, 2 infinite points; the plane must contain
        // the direction of the two direcitons
        dir1 = point1.xyz();
        dir2 = point2.xyz();
    } else if (point2.w != 0) {
        // 3 finite points, the plane must contain the directions
        // betwseen the points.
        dir1 = point1.xyz() - point0.xyz();
        dir2 = point2.xyz() - point0.xyz();
    } else {
        // 2 finite, 1 infinite point; the plane must contain
        // the direction between the first two points and the
        // direction of the third point.
        dir1 = point1.xyz() - point0.xyz();
        dir2 = point2.xyz();
    }

    _normal   = dir1.cross(dir2).direction();
    _distance = _normal.dot(point0.xyz());
}


Plane::Plane(
    const Vector3&      point0,
    const Vector3&      point1,
    const Vector3&      point2) {

    _normal   = (point1 - point0).cross(point2 - point0).direction();
    _distance = _normal.dot(point0);
}


Plane::Plane(
    const Vector3&      __normal,
    const Vector3&      point) {

    _normal    = __normal.direction();
    _distance  = _normal.dot(point);
}


Plane Plane::fromEquation(float a, float b, float c, float d) {
    Vector3 n(a, b, c);
    float magnitude = n.magnitude();
    d /= magnitude;
    n /= magnitude;
    return Plane(n, -d);
}


void Plane::flip() {
    _normal   = -_normal;
    _distance  = -_distance;
}


void Plane::getEquation(Vector3& n, float& d) const {
    double _d;
    getEquation(n, _d);
    d = (float)_d;
}

void Plane::getEquation(Vector3& n, double& d) const {
    n = _normal;
    d = -_distance;
}


void Plane::getEquation(float& a, float& b, float& c, float& d) const {
    double _a, _b, _c, _d;
    getEquation(_a, _b, _c, _d);
    a = (float)_a;
    b = (float)_b;
    c = (float)_c;
    d = (float)_d;
}

void Plane::getEquation(double& a, double& b, double& c, double& d) const {
    a = _normal.x;
    b = _normal.y;
    c = _normal.z;
    d = -_distance;
}


std::string Plane::toString() const {
    return format("Plane(%g, %g, %g, %g)", _normal.x, _normal.y, _normal.z, _distance);
}

}
