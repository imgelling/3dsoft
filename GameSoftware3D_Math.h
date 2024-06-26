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

	inline void my_PerspectiveFOV2(const float_t fov, const float_t aspect, const float_t nearz, const float_t farz, Matrix4x4f& proj)
	{
		float_t D2R = 3.14159f / 180.0f;
		float_t halfFOV = tan((D2R * fov) / 2.0f);
		float_t yScale = 1.0f / halfFOV;
		float_t xScale = 1.0f / (aspect * halfFOV);
		//float_t m[] = {
		//xScale, 0,      0,                           0,
		//0,      yScale, 0,                           0,
		//0,      0,      farz / (farz - nearz),			 1,
		//0,      0,		-(nearz * farz) / (farz - nearz),	 0
		//};
		proj.m[0] = xScale;
		proj.m[5] = yScale;
		proj.m[10] = farz / (farz - nearz);
		proj.m[11] = 1.0f;
		proj.m[14] = -(nearz * farz) / (farz - nearz);
		proj.m[15] = 0;
		//memcpy(proj.m, m, sizeof(float) * 16);
	}

	inline Triangle RotateZ(const Triangle& __restrict tri, const float_t theta) noexcept
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

	inline Vector3f RotateZ(const Vector3f& __restrict in, const float_t theta) noexcept
	{
		float_t ctheta = cos(theta);
		float_t stheta = sin(theta);
		Vector3f ret;// (in);

		ret.x = (in.x * ctheta - in.y * stheta);
		ret.y = (in.x * stheta + in.y * ctheta);
		ret.z = in.z;


		return ret;
	}

	inline Triangle RotateX(const Triangle& __restrict tri, const float_t theta) noexcept
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

	template<typename T>
	inline Vector3<T> RotateX(const Vector3<T>& in, const float_t theta) noexcept
	{
		float_t ctheta = cos(theta);
		float_t stheta = sin(theta);
		Vector3<T> ret(in);

		ret.y = (T)(in.y * ctheta - in.z * stheta);
		ret.z = (T)(in.y * stheta + in.z * ctheta);

		return ret;
	}

	inline Triangle RotateY(const Triangle& __restrict tri, const float_t theta) noexcept
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

	template<typename T>
	inline Vector3<T> RotateY(const Vector3<T>& in, const float_t theta) noexcept
	{
		float_t ctheta = cos(theta);
		float_t stheta = sin(theta);
		Vector3<T> ret(in);

		ret.x = (T)(in.x * ctheta + in.z * stheta);
		ret.z = (T)(in.x * -stheta + in.z * ctheta);

		return ret;
	}

	// Returns +1 if the triangle ABC is CCW, -1 if CW, and 0 if collinear
	inline float_t CheckWinding(Vector3f & __restrict A, Vector3f & __restrict B, Vector3f & __restrict C)
	{
		Vector3f AB = B - A; // Vector from A to B
		Vector3f AC = C - A; // Vector from A to C
		Vector3f N = AB.Cross(AC); // Cross product of AB and AC
		return N.z; // Sign of the z-component of N
	}

	inline Vector3f VectorIntersectPlane(const float_t planeNormalDotPoint, const Vector3f& __restrict plane_n, Vector3f& __restrict lineStart, Vector3f& __restrict lineEnd, float_t& t) noexcept
	{
		float_t plane_d = -planeNormalDotPoint;
		float_t ad = lineStart.Dot(plane_n);
		float_t bd = lineEnd.Dot(plane_n);
		t = (-plane_d - ad) / (bd - ad);
		Vector3f lineStartToEnd = lineEnd - lineStart;
		Vector3f lineToIntersect = lineStartToEnd * t;
		return lineStart + lineToIntersect;
	}

	// Modified code from
	// From OLC PGE, https://github.com/OneLoneCoder/olcPixelGameEngine
	inline uint32_t ClipAgainstNearZ(Triangle& __restrict in_tri, Triangle& out_tri1, Triangle& out_tri2) noexcept
	{
		game::Vector3f planePoint(0.0f, 0.0f, -0.1f); // was changed from 0.0f
		game::Vector3f planeNormal(0.0f, 0.0f, 1.0f);
		float_t planeNormalDotPoint = planeNormal.Dot(planePoint);

		uint32_t insidePointCount = 0;
		uint32_t outsidePointCount = 0;

		uint32_t insidePoints[3] = {};
		uint32_t outsidePoints[3] = {};

		if (in_tri.vertices[0].z - planeNormalDotPoint >= 0.0f)
		{
			insidePoints[insidePointCount++] = 0;
		}
		else
		{
			outsidePoints[outsidePointCount++] = 0;
		}
		if (in_tri.vertices[1].z - planeNormalDotPoint >= 0.0f)
		{
			insidePoints[insidePointCount++] = 1;
		}
		else
		{
			outsidePoints[outsidePointCount++] = 1;
		}
		if (in_tri.vertices[2].z - planeNormalDotPoint >= 0.0f)
		{
			insidePoints[insidePointCount++] = 2;
		}
		else
		{
			outsidePoints[outsidePointCount++] = 2;
		}


		// All outside, no triangle to return
		if (insidePointCount == 0)
		{
			return 0; 
		}

		// All inside, no clipping to be done
		if (insidePointCount == 3)
		{
			out_tri1 = in_tri;
			return 1; 
		}

		if (insidePointCount == 1 && outsidePointCount == 2)
		{
			// Triangle should be clipped. As two points lie outside
			// the plane, the triangle simply becomes a smaller triangle

			// Copy appearance info to new triangle
			//out_tri1.color[0] = in_tri.color[0];   // not correct, need to interpolate color
			//out_tri1.color[1] = in_tri.color[1];
			//out_tri1.color[2] = in_tri.color[2];
			out_tri1.faceNormal = in_tri.faceNormal;


			// The inside point is valid, so keep that...
			out_tri1.vertices[0] = in_tri.vertices[insidePoints[0]];
			out_tri1.normals[0] = in_tri.normals[insidePoints[0]];// in_normals[0];
			out_tri1.uvs[0] = in_tri.uvs[insidePoints[0]];// in_uv[0];
			out_tri1.color[0] = in_tri.color[insidePoints[0]];


			// but the two new points are at the locations where the 
			// original sides of the triangle (lines) intersect with the plane
			float_t t = 0.0f;
			float_t r = 0.0f, g=0.0f, b = 0.0f;

			// First intersection
			out_tri1.vertices[1] = VectorIntersectPlane(planeNormalDotPoint, planeNormal, in_tri.vertices[insidePoints[0]], in_tri.vertices[outsidePoints[0]], t);

			out_tri1.normals[1].x = t * (in_tri.normals[outsidePoints[0]].x - in_tri.normals[insidePoints[0]].x) + in_tri.normals[insidePoints[0]].x;
			out_tri1.normals[1].y = t * (in_tri.normals[outsidePoints[0]].y - in_tri.normals[insidePoints[0]].y) + in_tri.normals[insidePoints[0]].y;
			out_tri1.normals[1].z = t * (in_tri.normals[outsidePoints[0]].z - in_tri.normals[insidePoints[0]].z) + in_tri.normals[insidePoints[0]].z;
			out_tri1.color[1].rf = t * (in_tri.color[outsidePoints[0]].rf - in_tri.color[insidePoints[0]].rf) + in_tri.color[insidePoints[0]].rf;
			out_tri1.color[1].gf = t * (in_tri.color[outsidePoints[0]].gf - in_tri.color[insidePoints[0]].gf) + in_tri.color[insidePoints[0]].gf;
			out_tri1.color[1].bf = t * (in_tri.color[outsidePoints[0]].bf - in_tri.color[insidePoints[0]].bf) + in_tri.color[insidePoints[0]].bf;
			out_tri1.color[1].af = t * (in_tri.color[outsidePoints[0]].af - in_tri.color[insidePoints[0]].af) + in_tri.color[insidePoints[0]].af;
			out_tri1.color[1].Set(out_tri1.color[1].rf, out_tri1.color[1].gf, out_tri1.color[1].bf, out_tri1.color[1].af);
			out_tri1.vertices[1].w = t * (in_tri.vertices[outsidePoints[0]].w - in_tri.vertices[insidePoints[0]].w) + in_tri.vertices[insidePoints[0]].w;
			out_tri1.uvs[1].x = t * (in_tri.uvs[outsidePoints[0]].x - in_tri.uvs[insidePoints[0]].x) + in_tri.uvs[insidePoints[0]].x;
			out_tri1.uvs[1].y = t * (in_tri.uvs[outsidePoints[0]].y - in_tri.uvs[insidePoints[0]].y) + in_tri.uvs[insidePoints[0]].y;


			// Second intersection
			out_tri1.vertices[2] = VectorIntersectPlane(planeNormalDotPoint, planeNormal, in_tri.vertices[insidePoints[0]], in_tri.vertices[outsidePoints[1]], t);

			out_tri1.normals[2].x = t * (in_tri.normals[outsidePoints[1]].x - in_tri.normals[insidePoints[0]].x) + in_tri.normals[insidePoints[0]].x;
			out_tri1.normals[2].y = t * (in_tri.normals[outsidePoints[1]].y - in_tri.normals[insidePoints[0]].y) + in_tri.normals[insidePoints[0]].y;
			out_tri1.normals[2].z = t * (in_tri.normals[outsidePoints[1]].z - in_tri.normals[insidePoints[0]].z) + in_tri.normals[insidePoints[0]].z;
			out_tri1.color[2].rf = t * (in_tri.color[outsidePoints[1]].rf - in_tri.color[insidePoints[0]].rf) + in_tri.color[insidePoints[0]].rf;
			out_tri1.color[2].gf = t * (in_tri.color[outsidePoints[1]].gf - in_tri.color[insidePoints[0]].gf) + in_tri.color[insidePoints[0]].gf;
			out_tri1.color[2].bf = t * (in_tri.color[outsidePoints[1]].bf - in_tri.color[insidePoints[0]].bf) + in_tri.color[insidePoints[0]].bf;
			out_tri1.color[2].af = t * (in_tri.color[outsidePoints[1]].af - in_tri.color[insidePoints[0]].af) + in_tri.color[insidePoints[0]].af;
			out_tri1.color[2].Set(out_tri1.color[2].rf, out_tri1.color[2].gf, out_tri1.color[2].bf, out_tri1.color[2].af);
			out_tri1.vertices[2].w = t * (in_tri.vertices[outsidePoints[1]].w - in_tri.vertices[insidePoints[0]].w) + in_tri.vertices[insidePoints[0]].w;
			out_tri1.uvs[2].x = t * (in_tri.uvs[outsidePoints[1]].x - in_tri.uvs[insidePoints[0]].x) + in_tri.uvs[insidePoints[0]].x;
			out_tri1.uvs[2].y = t * (in_tri.uvs[outsidePoints[1]].y - in_tri.uvs[insidePoints[0]].y) + in_tri.uvs[insidePoints[0]].y;

			return 1; // Return the newly formed single triangle
		}

		if (insidePointCount == 2 && outsidePointCount == 1)
		{
			// Triangle should be clipped. As two points lie inside the plane,
			// the clipped triangle becomes a "quad". Fortunately, we can
			// represent a quad with two new triangles

			// Copy appearance info to new triangles
			//out_tri1.color[0] = in_tri.color[0]; // not correct, need to interpolate color
			//out_tri1.color[1] = in_tri.color[1];
			//out_tri1.color[2] = in_tri.color[2];
			out_tri1.faceNormal = in_tri.faceNormal;
			//out_tri2.color[0] = in_tri.color[0];
			//out_tri2.color[1] = in_tri.color[1];
			//out_tri2.color[2] = in_tri.color[2];
			out_tri2.faceNormal = in_tri.faceNormal;

			// The first triangle consists of the two inside points and a new
			// point determined by the location where one side of the triangle
			// intersects with the plane
			out_tri1.vertices[0] = in_tri.vertices[insidePoints[0]];
			out_tri1.normals[0] = in_tri.normals[insidePoints[0]];
			out_tri1.uvs[0] = in_tri.uvs[insidePoints[0]];
			out_tri1.color[0] = in_tri.color[insidePoints[0]];


			out_tri1.vertices[1] = in_tri.vertices[insidePoints[1]];
			out_tri1.normals[1] = in_tri.normals[insidePoints[1]];
			out_tri1.uvs[1] = in_tri.uvs[insidePoints[1]];
			out_tri1.color[1] = in_tri.color[insidePoints[1]];

			float_t t = 0.0f;
			float_t r = 0.0f, g = 0.0f, b = 0.0f;

			// First intersection
			out_tri1.vertices[2] = VectorIntersectPlane(planeNormalDotPoint, planeNormal, in_tri.vertices[insidePoints[0]], in_tri.vertices[outsidePoints[0]], t);

			out_tri1.normals[2].x = t * (in_tri.normals[outsidePoints[0]].x - in_tri.normals[insidePoints[0]].x) + in_tri.normals[insidePoints[0]].x;
			out_tri1.normals[2].y = t * (in_tri.normals[outsidePoints[0]].y - in_tri.normals[insidePoints[0]].y) + in_tri.normals[insidePoints[0]].y;
			out_tri1.normals[2].z = t * (in_tri.normals[outsidePoints[0]].z - in_tri.normals[insidePoints[0]].z) + in_tri.normals[insidePoints[0]].z;
			out_tri1.color[2].rf = t * (in_tri.color[outsidePoints[0]].rf - in_tri.color[insidePoints[0]].rf) + in_tri.color[insidePoints[0]].rf;
			out_tri1.color[2].gf = t * (in_tri.color[outsidePoints[0]].gf - in_tri.color[insidePoints[0]].gf) + in_tri.color[insidePoints[0]].gf;
			out_tri1.color[2].bf = t * (in_tri.color[outsidePoints[0]].bf - in_tri.color[insidePoints[0]].bf) + in_tri.color[insidePoints[0]].bf;
			out_tri1.color[2].af = t * (in_tri.color[outsidePoints[0]].af - in_tri.color[insidePoints[0]].af) + in_tri.color[insidePoints[0]].af;
			out_tri1.color[2].Set(out_tri1.color[2].rf, out_tri1.color[2].gf, out_tri1.color[2].bf, out_tri1.color[2].af);
			out_tri1.vertices[2].w = t * (in_tri.vertices[outsidePoints[0]].w - in_tri.vertices[insidePoints[0]].w) + in_tri.vertices[insidePoints[0]].w;
			out_tri1.uvs[2].x = t * (in_tri.uvs[outsidePoints[0]].x - in_tri.uvs[insidePoints[0]].x) + in_tri.uvs[insidePoints[0]].x;
			out_tri1.uvs[2].y = t * (in_tri.uvs[outsidePoints[0]].y - in_tri.uvs[insidePoints[0]].y) + in_tri.uvs[insidePoints[0]].y;


			// 2nd intersection
			out_tri2.vertices[0] = in_tri.vertices[insidePoints[1]];
			out_tri2.normals[0] = in_tri.normals[insidePoints[1]];
			out_tri2.uvs[0] = in_tri.uvs[insidePoints[1]];
			out_tri2.color[0] = in_tri.color[insidePoints[1]];

			out_tri2.vertices[1] = out_tri1.vertices[2];
			out_tri2.normals[1] = out_tri1.normals[2];
			out_tri2.uvs[1] = out_tri1.uvs[2];
			out_tri2.color[1] = out_tri1.color[2];

			out_tri2.vertices[2] = VectorIntersectPlane(planeNormalDotPoint, planeNormal, in_tri.vertices[insidePoints[1]], in_tri.vertices[outsidePoints[0]], t);

			out_tri2.normals[2].x = t * (in_tri.normals[outsidePoints[0]].x - in_tri.normals[insidePoints[1]].x) + in_tri.normals[insidePoints[1]].x;
			out_tri2.normals[2].y = t * (in_tri.normals[outsidePoints[0]].y - in_tri.normals[insidePoints[1]].y) + in_tri.normals[insidePoints[1]].y;
			out_tri2.normals[2].z = t * (in_tri.normals[outsidePoints[0]].z - in_tri.normals[insidePoints[1]].z) + in_tri.normals[insidePoints[1]].z;
			out_tri2.color[2].rf = t * (in_tri.color[outsidePoints[0]].rf - in_tri.color[insidePoints[1]].rf) + in_tri.color[insidePoints[1]].rf;
			out_tri2.color[2].gf = t * (in_tri.color[outsidePoints[0]].gf - in_tri.color[insidePoints[1]].gf) + in_tri.color[insidePoints[1]].gf;
			out_tri2.color[2].bf = t * (in_tri.color[outsidePoints[0]].bf - in_tri.color[insidePoints[1]].bf) + in_tri.color[insidePoints[1]].bf;
			out_tri2.color[2].af = t * (in_tri.color[outsidePoints[0]].af - in_tri.color[insidePoints[1]].af) + in_tri.color[insidePoints[1]].af;
			out_tri2.color[2].Set(out_tri2.color[2].rf, out_tri2.color[2].gf, out_tri2.color[2].bf, out_tri2.color[2].af);
			out_tri2.vertices[2].w = t * (in_tri.vertices[outsidePoints[0]].w - in_tri.vertices[insidePoints[1]].w) + in_tri.vertices[insidePoints[1]].w;
			out_tri2.uvs[2].x = t * (in_tri.uvs[outsidePoints[0]].x - in_tri.uvs[insidePoints[1]].x) + in_tri.uvs[insidePoints[1]].x;
			out_tri2.uvs[2].y = t * (in_tri.uvs[outsidePoints[0]].y - in_tri.uvs[insidePoints[1]].y) + in_tri.uvs[insidePoints[1]].y;

			return 2; // Return two newly formed triangles which form a quad
		}
		return -1; // I added for all return paths warning
	}

	inline void ScaleToScreen(Triangle& triangle, const Pointi& bufferSize) noexcept
	{
		triangle.vertices[0].x += 1.0f;
		triangle.vertices[1].x += 1.0f;
		triangle.vertices[2].x += 1.0f;

		triangle.vertices[0].y += 1.0f;
		triangle.vertices[1].y += 1.0f;
		triangle.vertices[2].y += 1.0f;

		triangle.vertices[0].x *= bufferSize.x;
		triangle.vertices[1].x *= bufferSize.x;
		triangle.vertices[2].x *= bufferSize.x;

		triangle.vertices[0].y *= bufferSize.y;
		triangle.vertices[1].y *= bufferSize.y;
		triangle.vertices[2].y *= bufferSize.y;
	}
}

#endif
