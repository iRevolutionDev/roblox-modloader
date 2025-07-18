/**
 @file CoordinateFrame.cpp

 Coordinate frame class

 @maintainer Morgan McGuire, http://graphics.cs.williams.edu

 @created 2001-06-02
 @edited  2010-03-13

 Copyright 2000-2010, Morgan McGuire.
 All rights reserved.
*/

#include "RobloxModLoader/roblox/g3d/platform.h"
#include "RobloxModLoader/roblox/g3d/CoordinateFrame.h"
#include "RobloxModLoader/roblox/g3d/Quat.h"
#include "RobloxModLoader/roblox/g3d/Matrix4.h"
#include "RobloxModLoader/roblox/g3d/Box.h"
#include "RobloxModLoader/roblox/g3d/AABox.h"
#include "RobloxModLoader/roblox/g3d/Sphere.h"
#include "RobloxModLoader/roblox/g3d/Triangle.h"
#include "RobloxModLoader/roblox/g3d/Ray.h"
#include "RobloxModLoader/roblox/g3d/Capsule.h"
#include "RobloxModLoader/roblox/g3d/Cylinder.h"
#include "RobloxModLoader/roblox/g3d/UprightFrame.h"
#include "RobloxModLoader/roblox/g3d/stringutils.h"
#include "RobloxModLoader/roblox/g3d/PhysicsFrame.h"
#include "RobloxModLoader/roblox/g3d/UprightFrame.h"


namespace G3D {


std::string CoordinateFrame::toXYZYPRDegreesString() const {
    UprightFrame uframe(*this);
    
    return format("CFrame::fromXYZYPRDegrees(% 5.1ff, % 5.1ff, % 5.1ff, % 5.1ff, % 5.1ff, % 5.1ff)", 
                  uframe.translation.x, uframe.translation.y, uframe.translation.z, 
                  toDegrees(uframe.yaw), toDegrees(uframe.pitch), 0.0f);
}

CoordinateFrame CoordinateFrame::fromXYZYPRRadians(float x, float y, float z, float yaw, 
                                                   float pitch, float roll) {
    Matrix3 rotation = Matrix3::fromAxisAngleFast(Vector3::unitY(), yaw);
    
    rotation = Matrix3::fromAxisAngleFast(rotation.column(0), pitch) * rotation;
    rotation = Matrix3::fromAxisAngleFast(rotation.column(2), roll) * rotation;

    const Vector3 translation(x, y, z);
    
    return CoordinateFrame(rotation, translation);
}


void CoordinateFrame::getXYZYPRRadians(float& x, float& y, float& z, 
                                       float& yaw, float& pitch, float& roll) const {
    x = translation.x;
    y = translation.y;
    z = translation.z;
    
    const Vector3& look = lookVector();

    if (abs(look.y) > 0.99f) {
        // Looking nearly straight up or down

        yaw   = G3D::pi() + atan2(look.x, look.z);
        pitch = asin(look.y);
        roll  = 0.0f;
        
    } else {

        // Yaw cannot be affected by others, so pull it first
        yaw = G3D::pi() + atan2(look.x, look.z);
        
        // Pitch is the elevation of the yaw vector
        pitch = asin(look.y);
        
        Vector3 actualRight = rightVector();
        Vector3 expectedRight = look.cross(Vector3::unitY());

        roll = 0;//acos(actualRight.dot(expectedRight));  TODO
    }
}


void CoordinateFrame::getXYZYPRDegrees(float& x, float& y, float& z, 
                                       float& yaw, float& pitch, float& roll) const {
    getXYZYPRRadians(x, y, z, yaw, pitch, roll);
    yaw   = toDegrees(yaw);
    pitch = toDegrees(pitch);
    roll  = toDegrees(roll);
}
    

CoordinateFrame CoordinateFrame::fromXYZYPRDegrees(float x, float y, float z, 
                                                   float yaw, float pitch, float roll) {
    return fromXYZYPRRadians(x, y, z, toRadians(yaw), toRadians(pitch), toRadians(roll));
}


Ray CoordinateFrame::lookRay() const {
    return Ray::fromOriginAndDirection(translation, lookVector());
}


bool CoordinateFrame::fuzzyEq(const CoordinateFrame& other) const {

    for (int c = 0; c < 3; ++c) {
        for (int r = 0; r < 3; ++r) {
            if (! G3D::fuzzyEq(other.rotation[r][c], rotation[r][c])) {
                return false;
            }
        }
        if (! G3D::fuzzyEq(translation[c], other.translation[c])) {
            return false;
        }
    }

    return true;
}

// ROBLOX
bool CoordinateFrame::fuzzyEq(const CoordinateFrame& other, double absepsilon) const {

    for (int c = 0; c < 3; ++c) {
        for (int r = 0; r < 3; ++r) {
            if (! G3D::fuzzyEq(other.rotation[r][c], rotation[r][c], absepsilon)) {
                return false;
            }
        }
        if (! G3D::fuzzyEq(translation[c], other.translation[c], absepsilon)) {
            return false;
        }
    }

    return true;
}
// =============

bool CoordinateFrame::fuzzyIsIdentity() const {
    const Matrix3& I = Matrix3::identity();

    for (int c = 0; c < 3; ++c) {
        for (int r = 0; r < 3; ++r) {
            if (fuzzyNe(I[r][c], rotation[r][c])) {
                return false;
            }
        }
        if (fuzzyNe(translation[c], 0)) {
            return false;
        }
    }

    return true;
}


bool CoordinateFrame::isIdentity() const {
    return 
        (translation == Vector3::zero()) &&
        (rotation == Matrix3::identity());
}


Matrix4 CoordinateFrame::toMatrix4() const {
    return Matrix4(*this);
}


std::string CoordinateFrame::toXML() const {
    return G3D::format(
        "<COORDINATEFRAME>\n  %lf,%lf,%lf,%lf,\n  %lf,%lf,%lf,%lf,\n  %lf,%lf,%lf,%lf,\n  %lf,%lf,%lf,%lf\n</COORDINATEFRAME>\n",
        rotation[0][0], rotation[0][1], rotation[0][2], translation.x,
        rotation[1][0], rotation[1][1], rotation[1][2], translation.y,
        rotation[2][0], rotation[2][1], rotation[2][2], translation.z,
        0.0, 0.0, 0.0, 1.0);
}

//ROBLOX
#if 0
Plane CoordinateFrame::toObjectSpace(const Plane& p) const {
    Vector3 N, P;
    double d;
    p.getEquation(N, d);
    P = N * (float)d;
    P = pointToObjectSpace(P);
    N = normalToObjectSpace(N);
    return Plane(N, P);
}


Plane CoordinateFrame::toWorldSpace(const Plane& p) const {
    Vector3 N, P;
    double d;
    p.getEquation(N, d);
    P = N * (float)d;
    P = pointToWorldSpace(P);
    N = normalToWorldSpace(N);
    return Plane(N, P);
}
#else
Plane CoordinateFrame::toObjectSpace(const Plane& p) const 
{
	return Plane(	normalToObjectSpace(p.normal()),
					pointToObjectSpace(p.normal() * p.distance())	);
}


Plane CoordinateFrame::toWorldSpace(const Plane& p) const {
	return Plane(	normalToWorldSpace(p.normal()), 
					pointToWorldSpace(p.normal() * p.distance())	);
}
#endif
// =============

Triangle CoordinateFrame::toObjectSpace(const Triangle& t) const {
    return Triangle(pointToObjectSpace(t.vertex(0)),
        pointToObjectSpace(t.vertex(1)),
        pointToObjectSpace(t.vertex(2)));
}

//ROBLOX
Line CoordinateFrame::toWorldSpace(const Line& l) const {
    return Line::fromPointAndUnitDirection(pointToWorldSpace(l.point()), vectorToWorldSpace(l.direction()));
}
//========

Triangle CoordinateFrame::toWorldSpace(const Triangle& t) const {
    return Triangle(pointToWorldSpace(t.vertex(0)),
        pointToWorldSpace(t.vertex(1)),
        pointToWorldSpace(t.vertex(2)));
}


Cylinder CoordinateFrame::toWorldSpace(const Cylinder& c) const {
    return Cylinder(
        pointToWorldSpace(c.point(0)), 
        pointToWorldSpace(c.point(1)), 
        c.radius());
}


Capsule CoordinateFrame::toWorldSpace(const Capsule& c) const {
    return Capsule(
        pointToWorldSpace(c.point(0)), 
        pointToWorldSpace(c.point(1)), 
        c.radius());
}


Box CoordinateFrame::toWorldSpace(const AABox& b) const {
    Box b2(b);
    return toWorldSpace(b2);
}


Box CoordinateFrame::toWorldSpace(const Box& b) const {
    Box out(b);

    for (int i = 0; i < 8; ++i) {
        out._corner[i] = pointToWorldSpace(b._corner[i]);
        debugAssert(! isNaN(out._corner[i].x));
    }

    for (int i = 0; i < 3; ++i) {
        out._axis[i] = vectorToWorldSpace(b._axis[i]);
    }

    out._center = pointToWorldSpace(b._center);

    return out;
}


Box CoordinateFrame::toObjectSpace(const Box &b) const {
    return inverse().toWorldSpace(b);
}


Box CoordinateFrame::toObjectSpace(const AABox& b) const {
    return toObjectSpace(Box(b));
}

AABox CoordinateFrame::AABBtoWorldSpace(const AABox& b) const {
	Vector3 center = b.center();
	Vector3 halfSize = b.extent()*0.5f;

	Vector3 newCenter = pointToWorldSpace(center);
	Vector3 newHalfSize = Vector3(
		fabs(rotation[0][0]) * halfSize[0] + fabs(rotation[0][1]) * halfSize[1] + fabs(rotation[0][2]) * halfSize[2],
		fabs(rotation[1][0]) * halfSize[0] + fabs(rotation[1][1]) * halfSize[1] + fabs(rotation[1][2]) * halfSize[2],
		fabs(rotation[2][0]) * halfSize[0] + fabs(rotation[2][1]) * halfSize[1] + fabs(rotation[2][2]) * halfSize[2]);

	return AABox(newCenter - newHalfSize, newCenter + newHalfSize);
}

AABox CoordinateFrame::AABBtoObjectSpace(const AABox& b) const {
	Vector3 center = b.center();
	Vector3 halfSize = b.extent()*0.5f;

	Vector3 newCenter = pointToObjectSpace(center);
	Vector3 newHalfSize = Vector3(
		fabs(rotation[0][0]) * halfSize[0] + fabs(rotation[1][0]) * halfSize[1] + fabs(rotation[2][0]) * halfSize[2],
		fabs(rotation[0][1]) * halfSize[0] + fabs(rotation[1][1]) * halfSize[1] + fabs(rotation[2][1]) * halfSize[2],
		fabs(rotation[0][2]) * halfSize[0] + fabs(rotation[1][2]) * halfSize[1] + fabs(rotation[2][2]) * halfSize[2]);

	return AABox(newCenter - newHalfSize, newCenter + newHalfSize);
}


Sphere CoordinateFrame::toWorldSpace(const Sphere &b) const {
    return Sphere(pointToWorldSpace(b.center), b.radius);
}


Sphere CoordinateFrame::toObjectSpace(const Sphere &b) const {
    return Sphere(pointToObjectSpace(b.center), b.radius);
}


Ray CoordinateFrame::toWorldSpace(const Ray& r) const {
    return Ray::fromOriginAndDirection(pointToWorldSpace(r.origin()), vectorToWorldSpace(r.direction()));
}


Ray CoordinateFrame::toObjectSpace(const Ray& r) const {
    return Ray::fromOriginAndDirection(pointToObjectSpace(r.origin()), vectorToObjectSpace(r.direction()));
}

RBX::RbxRay CoordinateFrame::toObjectSpace(const RBX::RbxRay& r) const {
	return RBX::RbxRay::fromOriginAndDirection(pointToObjectSpace(r.origin()), vectorToObjectSpace(r.direction()));
}


void CoordinateFrame::lookAt(const Vector3 &target) {
    lookAt(target, Vector3::unitY());
}


void CoordinateFrame::lookAt(
    const Vector3&  target,
    Vector3         up) {

    up = up.direction();

    Vector3 look = (target - translation).direction();
    if (fabs(look.dot(up)) > .99f) {
        up = Vector3::unitX();
        if (fabs(look.dot(up)) > .99f) {
            up = Vector3::unitY();
        }
    }

    up -= look * look.dot(up);
    up.unitize();

    Vector3 z = -look;
    Vector3 x = -z.cross(up);
    x.unitize();

    Vector3 y = z.cross(x);

    rotation.setColumn(0, x);
    rotation.setColumn(1, y);
    rotation.setColumn(2, z);
}


CoordinateFrame CoordinateFrame::lerp(
    const CoordinateFrame&  other,
    float                   alpha) const {

    if (alpha == 1.0f) {
        return other;
    } else if (alpha == 0.0f) {
        return *this;
    } else {
        const Quat q1(this->rotation);
        const Quat q2(other.rotation);

        return CoordinateFrame(
            q1.slerp(q2, alpha).toRotationMatrix(),
            translation * (1 - alpha) + other.translation * alpha);
    }
} 

CoordinateFrame CoordinateFrame::nlerp(
	const CoordinateFrame&  other,
	float                   alpha) const
{
	if (alpha == 1.0f) {
		return other;
	} else if (alpha == 0.0f) {
		return *this;
	} else {
		const Quat q1(this->rotation);
		const Quat q2(other.rotation);

		return CoordinateFrame(
			q1.nlerp(q2, alpha).toRotationMatrix(),
			translation * (1 - alpha) + other.translation * alpha);
	}
}

void CoordinateFrame::pointToWorldSpace(const Array<Vector3>& v, Array<Vector3>& vout) const {
    vout.resize(v.size());

    for (int i = 0; i < v.size(); ++i) {
        vout[i] = pointToWorldSpace(v[i]);
    }
}


void CoordinateFrame::normalToWorldSpace(const Array<Vector3>& v, Array<Vector3>& vout) const  {
    vout.resize(v.size());

    for (int i = 0; i < v.size(); ++i) {
        vout[i] = normalToWorldSpace(v[i]);
    }
}


void CoordinateFrame::vectorToWorldSpace(const Array<Vector3>& v, Array<Vector3>& vout) const {
    vout.resize(v.size());

    for (int i = v.size() - 1; i >= 0; --i) {
        vout[i] = vectorToWorldSpace(v[i]);
    }
}


void CoordinateFrame::pointToObjectSpace(const Array<Vector3>& v, Array<Vector3>& vout) const {
    vout.resize(v.size());

    for (int i = v.size() - 1; i >= 0; --i) {
        vout[i] = pointToObjectSpace(v[i]);
    }
}


void CoordinateFrame::normalToObjectSpace(const Array<Vector3>& v, Array<Vector3>& vout) const {
    vout.resize(v.size());

    for (int i = v.size() - 1; i >= 0; --i) {
        vout[i] = normalToObjectSpace(v[i]);
    }
}


void CoordinateFrame::vectorToObjectSpace(const Array<Vector3>& v, Array<Vector3>& vout) const {
    vout.resize(v.size());

    for (int i = v.size() - 1; i >= 0; --i) {
        vout[i] = vectorToObjectSpace(v[i]);
    }
}

btTransform CoordinateFrame::transformFromCFrame() const
{
    btMatrix3x3 rot(
        rotation[0][0], rotation[0][1], rotation[0][2],
        rotation[1][0], rotation[1][1], rotation[1][2],
        rotation[2][0], rotation[2][1], rotation[2][2]);

    return btTransform(rot, btVector3(translation.x, translation.y, translation.z));
}

} // namespace
