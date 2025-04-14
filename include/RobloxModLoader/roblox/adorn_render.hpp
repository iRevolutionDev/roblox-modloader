#pragma once
#include "util/G3DCore.h"

using namespace G3D;

class AdornRender {
private:
	virtual void Function0();

	virtual void Function1();

	virtual void Function2();

	virtual void Function3();

	virtual void Function4();

	virtual void Function5();

	virtual void Function6();

	virtual void Function7();

	virtual void Function8();

	virtual void Function9();

public:
	virtual void prepareRenderPass();

	virtual void finishRenderPass();

	virtual void preSubmitPass();

	virtual void postSubmitPass();

private:
	virtual void Function11();

	virtual void Function12();

	virtual void Function13();

	virtual void Function14();

	virtual void Function15();

	virtual void Function16();

public:
	virtual void line2d(
		const Vector2 &p0,
		const Vector2 &p1,
		const Color4 &color);

private:
	virtual void idk(

	);

	virtual void unk2(
		const Vector4 &a1,
		const Vector4 &a2,
		const Vector2 &a3,
		const Vector2 &a4,
		const Color4 &a5,
		const Color4 &a6);

public:
	virtual void line3d(
		const Vector3 &p0,
		const Vector3 &p1,
		const Color4 &color,
		int zIndex,
		bool alwaysOnTop);

	virtual void line3d(
		const Vector3 &p0,
		const Vector3 &p1,
		const Color4 &color,
		float thickness,
		int zIndex,
		bool alwaysOnTop);

	virtual void line3dWithBlue(
		const CoordinateFrame &cFrame,
		const Color4 &color,
		int countv
	);

	virtual void setObjectToWorldMatrix(const CoordinateFrame &c);

	virtual void box(const CoordinateFrame &cFrame, const Vector3 &size, const Color4 &color, int zIndex,
	                 bool alwaysOnTop);

	virtual void box(
		const AABox &box,
		const Color4 &solidColor = Color4(1, .2f, .2f, .5f));

	virtual void sphere(
		const CoordinateFrame &cFrame,
		float radius,
		const Color4 &color,
		int zIndex,
		bool alwaysOnTop);

	virtual void sphere(
		const Sphere &sphere,
		const Color4 &solidColor = Color4(1, 1, 0, .5f));

	virtual void wheel(const CoordinateFrame &cframe, float size, const Color4 &color, int zIndex, bool alwaysOnTop);

	virtual void Function22();

	virtual void cylinder(
		const CoordinateFrame &cFrame,
		float radius,
		float length,
		const Color4 &color,
		int zIndex,
		bool alwaysOnTop);

private:
	virtual void unk(
		const Vector2 &pos,
		float size,
		float a4,
		float *a5,
		const Color4 &color,
		int zIndex,
		bool alwaysOnTop);

public:
	virtual void cylinderAlongX(
		float radius,
		float length,
		const Color4 &solidColor,
		bool cap = true);

	virtual void cone(
		const CoordinateFrame &cFrame,
		const float radius,
		float size,
		bool someBool,
		const Color4 &color,
		int zIndex,
		bool alwaysOnTop);

	virtual void arrow(
		const CoordinateFrame &cFrame,
		const Color4 &color);

private:
	virtual void Function28();

public:
	virtual void quad(
		bool isMesh,
		const CoordinateFrame &cFrame,
		const Vector2 &size,
		const Color4 &color,
		int zIndex,
		bool alwaysOnTop);

	virtual void convexPolygon2d(const Vector2 *v, int countv, const Color4 &color);

	virtual void convexPolygon(const Vector3 *v, int countv, const Color4 &color);
};
