#if !defined(GAMESOFTWARE3D_MATH_H)
#define GAMESOFTWARE3D_MATH_H

#include "GameSoftware3D_Data.h"


namespace game
{
	// Left handed (GL) -1 to +1
	inline void my_PerspectiveFOV(float_t fov, float_t aspect, float_t nearz, float_t farz, Projection& proj)
	{
		float_t D2R = 3.14159f / 180.0f;
		float_t yScale = 1.0f / tan((D2R * fov) / 2);
		float_t xScale = yScale / aspect;
		//float_t nearmfar = farz - nearz;
		//float_t m[] = {
		//	xScale, 0,      0,                           0,
		//	0,      yScale, 0,                           0,
		//	0,      0,      (farz + nearz) / nearmfar,   1,
		//	0,      0,      -(2 * farz * nearz) / nearmfar, 0
		//};
		proj.a = xScale;
		proj.b = yScale;
		proj.c = (farz + nearz) / (farz - nearz);
		proj.d = 1.0f;
		proj.e = -(2.0f * farz * nearz) / (farz - nearz);
	}

	// Left handed (DX) 0 to +1
	inline void my_PerspectiveFOV2(const float_t fov, const float_t aspect, const float_t nearz, const float_t farz, Projection& proj)
	{
		float_t D2R = 3.14159f / 180.0f;
		float_t halfFOV = tan((D2R * fov) / 2.0f);
		float_t yScale = 1.0f / halfFOV;
		float_t xScale = 1.0f / (aspect * halfFOV);
		//  float_t m[] = {
		//  xScale, 0,      0,                           0,
		//  0,      yScale, 0,                           0,
		//  0,      0,      farz / (farz - nearz),			 1,
		//  0,      0,		-(nearz * farz) / (farz - nearz),	 0
		//  	};
		proj.a = xScale;
		proj.b = yScale;
		proj.c = farz / (farz - nearz);
		proj.d = 1.0f;
		proj.e = -(nearz * farz) / (farz - nearz);
	}

	inline Triangle RotateZ(const Triangle& tri, const float_t theta) noexcept
	{
		Triangle ret(tri);
		float_t ctheta = cos(theta);
		float_t stheta = sin(theta);

		// Vertices
		ret.vertices[0].x = (tri.vertices[0].x) * ctheta - (tri.vertices[0].y) * stheta;
		ret.vertices[0].y = (tri.vertices[0].x) * stheta + (tri.vertices[0].y) * ctheta;
		ret.vertices[1].x = (tri.vertices[1].x) * ctheta - (tri.vertices[1].y) * stheta;
		ret.vertices[1].y = (tri.vertices[1].x) * stheta + (tri.vertices[1].y) * ctheta;
		ret.vertices[2].x = (tri.vertices[2].x) * ctheta - (tri.vertices[2].y) * stheta;
		ret.vertices[2].y = (tri.vertices[2].x) * stheta + (tri.vertices[2].y) * ctheta;

		// Vertex normals
		ret.normals[0].x = (tri.normals[0].x) * ctheta - (tri.normals[0].y) * stheta;
		ret.normals[0].y = (tri.normals[0].x) * stheta + (tri.normals[0].y) * ctheta;
		ret.normals[1].x = (tri.normals[1].x) * ctheta - (tri.normals[1].y) * stheta;
		ret.normals[1].y = (tri.normals[1].x) * stheta + (tri.normals[1].y) * ctheta;
		ret.normals[2].x = (tri.normals[2].x) * ctheta - (tri.normals[2].y) * stheta;
		ret.normals[2].y = (tri.normals[2].x) * stheta + (tri.normals[2].y) * ctheta;

		// Face normals
		ret.faceNormal.x = (tri.faceNormal.x) * ctheta - (tri.faceNormal.y) * stheta;
		ret.faceNormal.y = (tri.faceNormal.x) * stheta + (tri.faceNormal.y) * ctheta;

		return ret;
	}

	inline Vector3f RotateZ(const Vector3f& in, const float_t theta)
	{
		float_t ctheta = cos(theta);
		float_t stheta = sin(theta);
		Vector3f ret(in);

		ret.x = in.x * ctheta - in.y * stheta;
		ret.y = in.x * stheta + in.y * ctheta;

		return ret;
	}

	inline Triangle RotateX(const Triangle& tri, const float_t theta) noexcept
	{
		Triangle ret(tri);
		float_t ctheta = cos(theta);
		float_t stheta = sin(theta);

		// Vertices
		ret.vertices[0].y = (tri.vertices[0].y) * ctheta - (tri.vertices[0].z) * stheta;
		ret.vertices[0].z = (tri.vertices[0].y) * stheta + (tri.vertices[0].z) * ctheta;
		ret.vertices[1].y = (tri.vertices[1].y) * ctheta - (tri.vertices[1].z) * stheta;
		ret.vertices[1].z = (tri.vertices[1].y) * stheta + (tri.vertices[1].z) * ctheta;
		ret.vertices[2].y = (tri.vertices[2].y) * ctheta - (tri.vertices[2].z) * stheta;
		ret.vertices[2].z = (tri.vertices[2].y) * stheta + (tri.vertices[2].z) * ctheta;

		// Vertex normals
		ret.normals[0].y = (tri.normals[0].y) * ctheta - (tri.normals[0].z) * stheta;
		ret.normals[0].z = (tri.normals[0].y) * stheta + (tri.normals[0].z) * ctheta;
		ret.normals[1].y = (tri.normals[1].y) * ctheta - (tri.normals[1].z) * stheta;
		ret.normals[1].z = (tri.normals[1].y) * stheta + (tri.normals[1].z) * ctheta;
		ret.normals[2].y = (tri.normals[2].y) * ctheta - (tri.normals[2].z) * stheta;
		ret.normals[2].z = (tri.normals[2].y) * stheta + (tri.normals[2].z) * ctheta;

		// Face normals
		ret.faceNormal.y = (tri.faceNormal.y) * ctheta - (tri.faceNormal.z) * stheta;
		ret.faceNormal.z = (tri.faceNormal.y) * stheta + (tri.faceNormal.z) * ctheta;

		return ret;
	}

	inline Triangle RotateY(const Triangle& tri, const float_t theta) noexcept
	{
		Triangle ret(tri);
		float_t ctheta = cos(theta);
		float_t stheta = sin(theta);

		// Vertices
		ret.vertices[0].x = (tri.vertices[0].x) * ctheta + (tri.vertices[0].z) * stheta;
		ret.vertices[0].z = (tri.vertices[0].x) * -stheta + (tri.vertices[0].z) * ctheta;
		ret.vertices[1].x = (tri.vertices[1].x) * ctheta + (tri.vertices[1].z) * stheta;
		ret.vertices[1].z = (tri.vertices[1].x) * -stheta + (tri.vertices[1].z) * ctheta;
		ret.vertices[2].x = (tri.vertices[2].x) * ctheta + (tri.vertices[2].z) * stheta;
		ret.vertices[2].z = (tri.vertices[2].x) * -stheta + (tri.vertices[2].z) * ctheta;

		// Vertex normals
		ret.normals[0].x = (tri.normals[0].x) * ctheta + (tri.normals[0].z) * stheta;
		ret.normals[0].z = (tri.normals[0].x) * -stheta + (tri.normals[0].z) * ctheta;
		ret.normals[1].x = (tri.normals[1].x) * ctheta + (tri.normals[1].z) * stheta;
		ret.normals[1].z = (tri.normals[1].x) * -stheta + (tri.normals[1].z) * ctheta;
		ret.normals[2].x = (tri.normals[2].x) * ctheta + (tri.normals[2].z) * stheta;
		ret.normals[2].z = (tri.normals[2].x) * -stheta + (tri.normals[2].z) * ctheta;

		// Face normals
		ret.faceNormal.x = (tri.faceNormal.x) * ctheta + (tri.faceNormal.z) * stheta;
		ret.faceNormal.z = (tri.faceNormal.x) * -stheta + (tri.faceNormal.z) * ctheta;

		return ret;
	}

	inline Vector3f RotateY(const Vector3f& in, const float_t theta)
	{
		float_t ctheta = cos(theta);
		float_t stheta = sin(theta);
		Vector3f ret(in);

		ret.x = in.x * ctheta + in.z * stheta;
		ret.z = in.x * -stheta + in.z * ctheta;

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
