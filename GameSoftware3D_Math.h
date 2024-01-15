#if !defined(GAMESOFTWARE3D_MATH_H)
#define GAMESOFTWARE3D_MATH_H

#include "GameSoftware3D_Data.h"


namespace game
{
	inline Triangle RotateZ(const Triangle& tri, const float_t theta) noexcept
	{
		Triangle ret(tri);
		float_t ctheta = cos(theta);
		float_t stheta = sin(theta);

		ret.vertices[0].x = (tri.vertices[0].x) * ctheta - (tri.vertices[0].y) * stheta;
		ret.vertices[0].y = (tri.vertices[0].x) * stheta + (tri.vertices[0].y) * ctheta;
		ret.vertices[1].x = (tri.vertices[1].x) * ctheta - (tri.vertices[1].y) * stheta;
		ret.vertices[1].y = (tri.vertices[1].x) * stheta + (tri.vertices[1].y) * ctheta;
		ret.vertices[2].x = (tri.vertices[2].x) * ctheta - (tri.vertices[2].y) * stheta;
		ret.vertices[2].y = (tri.vertices[2].x) * stheta + (tri.vertices[2].y) * ctheta;

		return ret;
	}

	inline Triangle RotateX(const Triangle& tri, const float_t theta) noexcept
	{
		Triangle ret(tri);
		float_t ctheta = cos(theta);
		float_t stheta = sin(theta);

		ret.vertices[0].y = (tri.vertices[0].y) * ctheta - (tri.vertices[0].z) * stheta;
		ret.vertices[0].z = (tri.vertices[0].y) * stheta + (tri.vertices[0].z) * ctheta;
		ret.vertices[1].y = (tri.vertices[1].y) * ctheta - (tri.vertices[1].z) * stheta;
		ret.vertices[1].z = (tri.vertices[1].y) * stheta + (tri.vertices[1].z) * ctheta;
		ret.vertices[2].y = (tri.vertices[2].y) * ctheta - (tri.vertices[2].z) * stheta;
		ret.vertices[2].z = (tri.vertices[2].y) * stheta + (tri.vertices[2].z) * ctheta;

		return ret;
	}

	inline Triangle RotateY(const Triangle& tri, const float_t theta) noexcept
	{
		Triangle ret(tri);
		float_t ctheta = cos(theta);
		float_t stheta = sin(theta);

		ret.vertices[0].x = (tri.vertices[0].x) * ctheta + (tri.vertices[0].z) * stheta;
		ret.vertices[0].z = (tri.vertices[0].x) * -stheta + (tri.vertices[0].z) * ctheta;
		ret.vertices[1].x = (tri.vertices[1].x) * ctheta + (tri.vertices[1].z) * stheta;
		ret.vertices[1].z = (tri.vertices[1].x) * -stheta + (tri.vertices[1].z) * ctheta;
		ret.vertices[2].x = (tri.vertices[2].x) * ctheta + (tri.vertices[2].z) * stheta;
		ret.vertices[2].z = (tri.vertices[2].x) * -stheta + (tri.vertices[2].z) * ctheta;

		return ret;
	}

	inline Triangle RotateXYZ(const Triangle& tri, const float_t thetaX, const float_t thetaY, const float_t thetaZ) noexcept
	{
		Triangle ret(tri);

		ret = RotateX(ret, thetaX);
		ret = RotateY(ret, thetaY);
		ret = RotateZ(ret, thetaZ);

		return ret;
	}

	inline Triangle Translate(const Triangle& tri, const float_t _x, const float_t _y, const float_t _z) noexcept
	{
		Triangle ret(tri);

		ret.vertices[0].x += _x;
		ret.vertices[0].y += _y;
		ret.vertices[0].z += _z;
		ret.vertices[1].x += _x;
		ret.vertices[1].y += _y;
		ret.vertices[1].z += _z;
		ret.vertices[2].x += _x;
		ret.vertices[2].y += _y;
		ret.vertices[2].z += _z;

		return ret;
	}

	inline Triangle Translate(const Triangle& tri, Vector3f& translate) noexcept
	{
		Triangle ret(tri);

		ret.vertices[0] += translate;
		ret.vertices[1] += translate;
		ret.vertices[2] += translate;

		return ret;
	}
}

#endif
