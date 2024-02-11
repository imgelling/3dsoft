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

	template<typename T>
	inline Vector3<T> RotateZ(const Vector3<T>& in, const float_t theta) noexcept
	{
		float_t ctheta = cos(theta);
		float_t stheta = sin(theta);
		Vector3<T> ret(in);

		ret.x = (T)(in.x * ctheta - in.y * stheta);
		ret.y = (T)(in.x * stheta + in.y * ctheta);

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

	inline Triangle RotateXYZ(const Triangle& tri, const float_t thetaX, const float_t thetaY, const float_t thetaZ) noexcept
	{
		Triangle ret(tri);

		ret = RotateX(ret, thetaX);
		ret = RotateY(ret, thetaY);
		ret = RotateZ(ret, thetaZ);

		return ret;
	}

	template<typename T>
	inline Vector3<T> RotateXYZ(const Vector3<T>& in, const float_t thetaX, const float_t thetaY, const float_t thetaZ) noexcept
	{
		Vector3<T> ret(in);

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

	inline Triangle Project(const Triangle& triangle, const Projection& proj) noexcept
	{
		Triangle ret(triangle);

		ret.vertices[0].x = triangle.vertices[0].x * proj.a;
		ret.vertices[1].x = triangle.vertices[1].x * proj.a;
		ret.vertices[2].x = triangle.vertices[2].x * proj.a;

		ret.vertices[0].y = triangle.vertices[0].y * proj.b;
		ret.vertices[1].y = triangle.vertices[1].y * proj.b;
		ret.vertices[2].y = triangle.vertices[2].y * proj.b;

		ret.vertices[0].z = (triangle.vertices[0].z * proj.c) + (triangle.vertices[0].w * proj.e);
		ret.vertices[1].z = (triangle.vertices[1].z * proj.c) + (triangle.vertices[1].w * proj.e);
		ret.vertices[2].z = (triangle.vertices[2].z * proj.c) + (triangle.vertices[2].w * proj.e);

		ret.vertices[0].w = triangle.vertices[0].z;
		ret.vertices[1].w = triangle.vertices[1].z;
		ret.vertices[2].w = triangle.vertices[2].z;

		return ret;
	}


	// Returns +1 if the triangle ABC is CCW, -1 if CW, and 0 if collinear
	inline float_t CheckWinding(Vector3f A, Vector3f B, Vector3f C)
	{
		Vector3f AB = B - A; // Vector from A to B
		Vector3f AC = C - A; // Vector from A to C
		Vector3f N = AB.Cross(AC); // Cross product of AB and AC
		return (N.z); // Sign of the z-component of N
	}

	inline Vector3f VectorIntersectPlane(const Vector3f& plane_p, const Vector3f& plane_n, Vector3f& lineStart, Vector3f& lineEnd, float_t& t) noexcept
	{
		float_t plane_d = -plane_n.Dot(plane_p);
		float_t ad = lineStart.Dot(plane_n);
		float_t bd = lineEnd.Dot(plane_n);
		t = (-plane_d - ad) / (bd - ad);
		Vector3f lineStartToEnd = lineEnd - lineStart;
		Vector3f lineToIntersect = lineStartToEnd * t;
		return lineStart + lineToIntersect;
	}

	inline uint32_t ClipAgainstPlane(Vector3f plane_p, Vector3f plane_n, Triangle& in_tri, Triangle& out_tri1, Triangle& out_tri2)
	{
		//// Make sure plane normal is indeed normal
		//plane_n.Normalize();// plane_n = Vector_Normalise(plane_n);
		float_t d = plane_n.Dot(plane_p);

		// Return signed shortest distance from point to plane, plane normal must be normalised
		//auto dist = [&](Vector3f& p)
		//	{
		//		//Vector3f n = p;// Vector_Normalise(p);
		//		//n.Normalize();
		//		//return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - d);// Vector_DotProduct(plane_n, plane_p));
		//		return (p.z - d);
		//	};

		// Create two temporary storage arrays to classify points either side of plane
		// If distance sign is positive, point lies on "inside" of plane
		Vector3f inside_points[3] = {};  uint32_t nInsidePointCount = 0;
		Vector3f outside_points[3] = {}; uint32_t nOutsidePointCount = 0;

		// Get signed distance of each point in triangle to plane
		//float_t d0 = in_tri.vertices[0].z - d;// dist(in_tri.vertices[0]);
		//float_t d1 = in_tri.vertices[1].z - d;// dist(in_tri.vertices[1]);
		//float_t d2 = in_tri.vertices[2].z - d;// dist(in_tri.vertices[2]);

		Vector3f in_normals[3];
		Vector3f out_normals[3];

		Vector2f in_uv[3];
		Vector2f out_uv[3];

		float_t in_d[3]{};
		float_t out_d[3]{};


		if (in_tri.vertices[0].z - d >= 0.0f)
		{
			inside_points[nInsidePointCount++] = in_tri.vertices[0];
			in_normals[nInsidePointCount - 1] = in_tri.normals[0];
			in_uv[nInsidePointCount - 1] = in_tri.uvs[0];
			in_d[nInsidePointCount - 1] = in_tri.vertices[0].w;
		}
		else
		{
			outside_points[nOutsidePointCount++] = in_tri.vertices[0];
			out_normals[nOutsidePointCount - 1] = in_tri.normals[0];
			out_uv[nOutsidePointCount - 1] = in_tri.uvs[0];
			out_d[nOutsidePointCount - 1] = in_tri.vertices[0].w;
		}
		if (in_tri.vertices[1].z - d >= 0.0f)
		{
			inside_points[nInsidePointCount++] = in_tri.vertices[1];
			in_normals[nInsidePointCount - 1] = in_tri.normals[1];
			in_uv[nInsidePointCount - 1] = in_tri.uvs[1];
			in_d[nInsidePointCount - 1] = in_tri.vertices[1].w;
		}
		else
		{
			outside_points[nOutsidePointCount++] = in_tri.vertices[1];
			out_normals[nOutsidePointCount - 1] = in_tri.normals[1];
			out_uv[nOutsidePointCount - 1] = in_tri.uvs[1];
			out_d[nOutsidePointCount - 1] = in_tri.vertices[1].w;
		}
		if (in_tri.vertices[2].z - d >= 0.0f)
		{
			inside_points[nInsidePointCount++] = in_tri.vertices[2];
			in_normals[nInsidePointCount - 1] = in_tri.normals[2];
			in_uv[nInsidePointCount - 1] = in_tri.uvs[2];
			in_d[nInsidePointCount - 1] = in_tri.vertices[2].w;
		}
		else
		{
			outside_points[nOutsidePointCount++] = in_tri.vertices[2];
			out_normals[nOutsidePointCount - 1] = in_tri.normals[2];
			out_uv[nOutsidePointCount - 1] = in_tri.uvs[2];
			out_d[nOutsidePointCount - 1] = in_tri.vertices[2].w;
		}



		if (nInsidePointCount == 0)
		{
			return 0; // No returned triangles are valid
		}

		if (nInsidePointCount == 3)
		{
			out_tri1 = in_tri;
			return 1; // Same triangle returned as didn't get clipped
		}

		if (nInsidePointCount == 1 && nOutsidePointCount == 2)
		{
			// Triangle should be clipped. As two points lie outside
			// the plane, the triangle simply becomes a smaller triangle

			// Copy appearance info to new triangle
			out_tri1.color[0] = in_tri.color[0];
			out_tri1.color[1] = in_tri.color[1];
			out_tri1.color[2] = in_tri.color[2];
			out_tri1.faceNormal = in_tri.faceNormal;


			// The inside point is valid, so keep that...
			out_tri1.vertices[0] = inside_points[0];
			out_tri1.normals[0] = in_normals[0];
			out_tri1.uvs[0] = in_uv[0];


			// but the two new points are at the locations where the 
			// original sides of the triangle (lines) intersect with the plane
			float t = 0.0;

			// First intersection
			out_tri1.vertices[1] = VectorIntersectPlane(plane_p, plane_n, inside_points[0], outside_points[0], t);

			// Correct vertex normal due to clipping
			out_tri1.normals[1].x = t * (out_normals[0].x - in_normals[0].x) + in_normals[0].x;
			out_tri1.normals[1].y = t * (out_normals[0].y - in_normals[0].y) + in_normals[0].y;
			out_tri1.normals[1].z = t * (out_normals[0].z - in_normals[0].z) + in_normals[0].z;
			out_tri1.vertices[1].w = t * (out_d[0] - in_d[0]) + in_d[0];
			out_tri1.uvs[1].x = t * (out_uv[0].x - in_uv[0].x) + in_uv[0].x;
			out_tri1.uvs[1].y = t * (out_uv[0].y - in_uv[0].y) + in_uv[0].y;


			// Second intersection
			out_tri1.vertices[2] = VectorIntersectPlane(plane_p, plane_n, inside_points[0], outside_points[1], t);

			out_tri1.normals[2].x = t * (out_normals[1].x - in_normals[0].x) + in_normals[0].x;
			out_tri1.normals[2].y = t * (out_normals[1].y - in_normals[0].y) + in_normals[0].y;
			out_tri1.normals[2].z = t * (out_normals[1].z - in_normals[0].z) + in_normals[0].z;
			out_tri1.vertices[2].w = t * (out_d[1] - in_d[0]) + in_d[0];
			out_tri1.uvs[2].x = t * (out_uv[1].x - in_uv[0].x) + in_uv[0].x;
			out_tri1.uvs[2].y = t * (out_uv[1].y - in_uv[0].y) + in_uv[0].y;

			return 1; // Return the newly formed single triangle
		}

		if (nInsidePointCount == 2 && nOutsidePointCount == 1)
		{
			// Triangle should be clipped. As two points lie inside the plane,
			// the clipped triangle becomes a "quad". Fortunately, we can
			// represent a quad with two new triangles

			// Copy appearance info to new triangles
			out_tri1.color[0] = in_tri.color[0];
			out_tri1.color[1] = in_tri.color[1];
			out_tri1.color[2] = in_tri.color[2];
			out_tri1.faceNormal = in_tri.faceNormal;
			out_tri2.color[0] = in_tri.color[0];
			out_tri2.color[1] = in_tri.color[1];
			out_tri2.color[2] = in_tri.color[2];
			out_tri2.faceNormal = in_tri.faceNormal;

			// The first triangle consists of the two inside points and a new
			// point determined by the location where one side of the triangle
			// intersects with the plane
			out_tri1.vertices[0] = inside_points[0];
			out_tri1.normals[0] = in_normals[0];
			out_tri1.uvs[0] = in_uv[0];

			out_tri1.vertices[1] = inside_points[1];
			out_tri1.normals[1] = in_normals[1];
			out_tri1.uvs[1] = in_uv[1];

			float t = 0.0;

			// First intersection
			out_tri1.vertices[2] = VectorIntersectPlane(plane_p, plane_n, inside_points[0], outside_points[0], t);

			// Correct the vertex normal due to clipping
			out_tri1.normals[2].x = t * (out_normals[0].x - in_normals[0].x) + in_normals[0].x;
			out_tri1.normals[2].y = t * (out_normals[0].y - in_normals[0].y) + in_normals[0].y;
			out_tri1.normals[2].z = t * (out_normals[0].z - in_normals[0].z) + in_normals[0].z;
			out_tri1.vertices[2].w = t * (out_d[0] - in_d[0]) + in_d[0];
			out_tri1.uvs[2].x = t * (out_uv[0].x - in_uv[0].x) + in_uv[0].x;
			out_tri1.uvs[2].y = t * (out_uv[0].y - in_uv[0].y) + in_uv[0].y;


			// 2nd intersection
			out_tri2.vertices[0] = inside_points[1];
			out_tri2.normals[0] = in_normals[1];
			out_tri2.uvs[0] = in_uv[1];

			out_tri2.vertices[1] = out_tri1.vertices[2];
			out_tri2.normals[1] = out_tri1.normals[2];
			out_tri2.uvs[1] = out_tri1.uvs[2];

			out_tri2.vertices[2] = VectorIntersectPlane(plane_p, plane_n, inside_points[1], outside_points[0], t);


			// Correct the vertex normal due to clipping
			out_tri2.normals[2].x = t * (out_normals[0].x - in_normals[1].x) + in_normals[1].x;
			out_tri2.normals[2].y = t * (out_normals[0].y - in_normals[1].y) + in_normals[1].y;
			out_tri2.normals[2].z = t * (out_normals[0].z - in_normals[1].z) + in_normals[1].z;
			out_tri2.vertices[2].w = t * (out_d[0] - in_d[1]) + in_d[1];
			out_tri2.uvs[2].x = t * (out_uv[0].x - in_uv[1].x) + in_uv[1].x;
			out_tri2.uvs[2].y = t * (out_uv[0].y - in_uv[1].y) + in_uv[1].y;

			return 2; // Return two newly formed triangles which form a quad
		}
		return -1; // I added for all return paths warning
	}

	inline void PerspectiveDivide(Triangle& triangle)
	{
		triangle.vertices[0] /= triangle.vertices[0].w;
		triangle.vertices[1] /= triangle.vertices[1].w;
		triangle.vertices[2] /= triangle.vertices[2].w;
	}

	inline void ScaleToScreen(Triangle& triangle, const Pointi& bufferSize) noexcept
	{
		triangle.vertices[0].x += 1.0f;
		triangle.vertices[1].x += 1.0f;
		triangle.vertices[2].x += 1.0f;

		triangle.vertices[0].y += 1.0f;
		triangle.vertices[1].y += 1.0f;
		triangle.vertices[2].y += 1.0f;

		triangle.vertices[0].x *= 0.5f * (float_t)bufferSize.x;
		triangle.vertices[1].x *= 0.5f * (float_t)bufferSize.x;
		triangle.vertices[2].x *= 0.5f * (float_t)bufferSize.x;

		triangle.vertices[0].y *= 0.5f * (float_t)bufferSize.y;
		triangle.vertices[1].y *= 0.5f * (float_t)bufferSize.y;
		triangle.vertices[2].y *= 0.5f * (float_t)bufferSize.y;
	}
}

#endif
