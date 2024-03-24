#if !defined(GAMESOFTWARE3D_POINTSPRITE_H)
#define GAMESOFTWARE3D_POINTSPRITE_H

#include "GameMath.h"
#include "GameColor.h"
#include "GameSoftware3D_Math.h"
#include "GameSoftware3D_Camera3D.h"

namespace game
{
	class PointSprite
	{
	public:
		Vector3f position;
		Matrix4x4f pointSprite;
		float_t rotation;
		Vector2f size;
		Color color;
		PointSprite()
		{
			rotation = 0;
			color = Colors::White;
		}
		inline void GeneratePointSpriteMatrix(const Camera3D& camera) noexcept
		{
			pointSprite.m[0] = camera.view.m[0];
			pointSprite.m[4] = camera.view.m[1];
			pointSprite.m[8] = camera.view.m[2];

			pointSprite.m[1] = camera.view.m[4];
			pointSprite.m[5] = camera.view.m[5];
			pointSprite.m[9] = camera.view.m[6];

			pointSprite.m[2] = camera.view.m[8];
			pointSprite.m[6] = camera.view.m[9];
			pointSprite.m[10] = camera.view.m[10];

			pointSprite.m[3] = 0;
			pointSprite.m[7] = 0;
			pointSprite.m[11] = 0;

			pointSprite.m[15] = 1;
		}
		inline void GenerateQuad(Triangle& topLeftTri, Triangle& bottomRightTri) noexcept
		{
			// tl
			topLeftTri.vertices[0].x = -size.width; 
			topLeftTri.vertices[0].y = -size.height;
			topLeftTri.vertices[0].z = 0;
			topLeftTri.color[0] = color;
			topLeftTri.uvs[0].u = 0.0f;
			topLeftTri.uvs[0].v = 0.0f;
			topLeftTri.faceNormal.x = 0.0f;
			topLeftTri.faceNormal.y = 0.0f;
			topLeftTri.faceNormal.z = -1.0f;


			// tr
			topLeftTri.vertices[1].x = size.width;  
			topLeftTri.vertices[1].y = -size.height;
			topLeftTri.vertices[1].z = 0;
			topLeftTri.uvs[1].u = 1.0f;
			topLeftTri.uvs[1].v = 0.0f;
			topLeftTri.color[1] = color;

			// bl
			topLeftTri.vertices[2].x = -size.width;
			topLeftTri.vertices[2].y = size.height;
			topLeftTri.vertices[2].z = 0;
			topLeftTri.uvs[2].u = 0.0f;
			topLeftTri.uvs[2].v = 1.0f;
			topLeftTri.color[2] = color;

			// tr
			bottomRightTri.vertices[0].x = size.width;  
			bottomRightTri.vertices[0].y = -size.height;
			bottomRightTri.vertices[0].z = 0;
			bottomRightTri.color[0] = color;
			bottomRightTri.uvs[0].u = 1.0f;
			bottomRightTri.uvs[0].v = 0.0f;
			bottomRightTri.faceNormal.x = 0.0f;
			bottomRightTri.faceNormal.y = 0.0f;
			bottomRightTri.faceNormal.z = -1.0f;

			// br
			bottomRightTri.vertices[1].x = size.width; 
			bottomRightTri.vertices[1].y = size.height;
			bottomRightTri.vertices[1].z = 0;
			bottomRightTri.uvs[1].u = 1.0f;
			bottomRightTri.uvs[1].v = 1.0f;
			bottomRightTri.color[1] = color;

			// bl
			bottomRightTri.vertices[2].x = -size.width;
			bottomRightTri.vertices[2].y = size.height;
			bottomRightTri.vertices[2].z = 0;
			bottomRightTri.uvs[2].u = 0.0f;
			bottomRightTri.uvs[2].v = 1.0f;
			bottomRightTri.color[2] = color;

			for (uint32_t i = 0; i < 3; i++)
			{
				topLeftTri.normals[i] = { 0.0f,0.0f,-1.0f };
				bottomRightTri.normals[i] = { 0.0f,0.0f,-1.0f };

				topLeftTri.vertices[i] = RotateZ(topLeftTri.vertices[i], rotation);
				topLeftTri.vertices[i] = topLeftTri.vertices[i] * pointSprite;

				bottomRightTri.vertices[i] = RotateZ(bottomRightTri.vertices[i], rotation);
				bottomRightTri.vertices[i] = bottomRightTri.vertices[i] * pointSprite;
			}
		}

		inline void UpdateQuad(Triangle& topLeftTri, Triangle& bottomRightTri, const Matrix4x4f & __restrict bill) 
		{
			// tl
			topLeftTri.vertices[0].x = -size.width; 
			topLeftTri.vertices[0].y = -size.height;
			topLeftTri.vertices[0].z = 0;
			topLeftTri.color[0] = color;

			// tr
			topLeftTri.vertices[1].x = size.width;  
			topLeftTri.vertices[1].y = -size.height;
			topLeftTri.vertices[1].z = 0;
			topLeftTri.color[1] = color;

			// bl
			topLeftTri.vertices[2].x = -size.width;
			topLeftTri.vertices[2].y = size.height;
			topLeftTri.vertices[2].z = 0;
			topLeftTri.color[2] = color;

			// tr
			bottomRightTri.vertices[0].x = size.width;  
			bottomRightTri.vertices[0].y = -size.height;
			bottomRightTri.vertices[0].z = 0;
			bottomRightTri.color[0] = color;

			// br
			bottomRightTri.vertices[1].x = size.width;
			bottomRightTri.vertices[1].y = size.height;
			bottomRightTri.vertices[1].z = 0;
			bottomRightTri.color[1] = color;

			// bl
			bottomRightTri.vertices[2].x = -size.width;
			bottomRightTri.vertices[2].y = size.height;
			bottomRightTri.vertices[2].z = 0;
			bottomRightTri.color[2] = color;

			float_t ctheta = cos(rotation);
			float_t stheta = sin(rotation);
			Vector3f ret;// (topLeftTri.vertices[0]);

			ret = topLeftTri.vertices[0];
			topLeftTri.vertices[0].x = ret.x * ctheta - ret.y * stheta;
			topLeftTri.vertices[0].y = ret.x * stheta + ret.y * ctheta;
			topLeftTri.vertices[0] = topLeftTri.vertices[0] * bill;

			ret = bottomRightTri.vertices[0];
			bottomRightTri.vertices[0].x = ret.x * ctheta - ret.y * stheta;
			bottomRightTri.vertices[0].y = ret.x * stheta + ret.y * ctheta;
			bottomRightTri.vertices[0] = bottomRightTri.vertices[0] * bill;

			ret = topLeftTri.vertices[1];
			topLeftTri.vertices[1].x = ret.x * ctheta - ret.y * stheta;
			topLeftTri.vertices[1].y = ret.x * stheta + ret.y * ctheta;
			topLeftTri.vertices[1] = topLeftTri.vertices[1] * bill;

			ret = bottomRightTri.vertices[1];
			bottomRightTri.vertices[1].x = ret.x * ctheta - ret.y * stheta;
			bottomRightTri.vertices[1].y = ret.x * stheta + ret.y * ctheta;
			bottomRightTri.vertices[1] = bottomRightTri.vertices[1] * bill;

			ret = topLeftTri.vertices[2];
			topLeftTri.vertices[2].x = ret.x * ctheta - ret.y * stheta;
			topLeftTri.vertices[2].y = ret.x * stheta + ret.y * ctheta;
			topLeftTri.vertices[2] = topLeftTri.vertices[2] * bill;

			ret = bottomRightTri.vertices[2];
			bottomRightTri.vertices[2].x = ret.x * ctheta - ret.y * stheta;
			bottomRightTri.vertices[2].y = ret.x * stheta + ret.y * ctheta;
			bottomRightTri.vertices[2] = bottomRightTri.vertices[2] * bill;
		}


		//inline void GenerateBillboardMatrix(Camera3D& camera) noexcept
		//{
		//	Vector3f look;// = camera.position - position;
		//	look.x = -(camera.position.x - position.x);
		//	look.y = (camera.position.y - position.y);
		//	look.z = (camera.position.z - position.z);

		//	look.Normalize();

		//	Vector3f right = camera.up.Cross(look);
		//	right.Normalize();
		//	Vector3f up = look.Cross(right);
		//	up.Normalize();

		//	billboard.SetIdentity();
		//	billboard.m[0] = right.x;
		//	billboard.m[4] = right.y;
		//	billboard.m[8] = right.z;

		//	billboard.m[1] = 0.0f;// up.x; // locks y rotation
		//	billboard.m[5] = 1.0f;// up.y;
		//	billboard.m[9] = 0.0f;// up.z;

		//	billboard.m[2] = look.x;
		//	billboard.m[6] = look.y;
		//	billboard.m[10] = look.z;

		//	billboard.m[12] = (position.x);
		//	billboard.m[13] = (position.y);
		//	billboard.m[14] = (position.z);


		//	////billboard.m[15] = 1.0f;
		//	//Matrix4x4f rotY;
		//	//rotY.SetRotationY(3.14159f/2.0f);
		//	//billboard = billboard * translate;
		//	//SetTranslation(0, 0, 0);
		//	//billboard = billboard *rotY;
		//}

		//inline void GenerateBillboardMatrix2(const Camera3D& camera) noexcept
		//{
		//	//Matrix4x4f x;
		//	//Matrix4x4f y;
		//	//Matrix4x4f z;
		//	//Matrix4x4f t;
		//	billboard.SetIdentity();
		//	//x.SetRotationX(-camera.rotation.x);
		//	//y.SetRotationY(camera.rotation.y -3.14159f / 2.0f);
		//	//z.SetRotationZ(camera.rotation.z);
		//	//SetTranslation(position.x, position.y, position.z);
		//	billboard.m[0] = camera.view.m[0];//right.x;
		//	billboard.m[4] = camera.view.m[1];//right.y;
		//	billboard.m[8] = camera.view.m[2];//right.z;

		//	billboard.m[1] = camera.view.m[4];//up.x;
		//	billboard.m[5] = camera.view.m[5];//up.y;
		//	billboard.m[9] = camera.view.m[6];//up.z;

		//	billboard.m[2] = camera.view.m[8];//look.x;
		//	billboard.m[6] = camera.view.m[9];//look.y;
		//	billboard.m[10] = camera.view.m[10];//look.z;
		//	//billboard.m[12] = position.x;// camera.view.m[3];
		//	//billboard.m[13] = position.y;// camera.view.m[7];
		//	//billboard.m[14] = position.z;// camera.view.m[11];
		//	//position = { 0 ,0,0};
		//	//GenerateModelMatrix();
		//	//billboard = billboard * translate;
		//	//SetTranslation(0, 0, 0);
		//	//billboard = billboard * z * y * x;// *translate;
		//}

		//inline void GenerateQuad(const Color& color, const Pointf size) noexcept
		//{
		//	Triangle topLeftTri;
		//	Triangle bottomRightTri;
		//	// ---------- ALL OF THE FOLLOWING CAN BE PRECALC -------------------------------
		//				//float_t size = 0.5f;  // actually half size
		//	float_t z = 0;// position.z;
		//	//position = { 0,0,position.z };
		//	// tl
		//	topLeftTri.vertices[0].x = -size.width;// +position.x;
		//	topLeftTri.vertices[0].y = -size.height;// +position.y;
		//	topLeftTri.vertices[0].z = z;
		//	topLeftTri.color[0] = color;
		//	topLeftTri.uvs[0].u = 0.0f;
		//	topLeftTri.uvs[0].v = 0.0f;
		//	topLeftTri.faceNormal.x = 0.0f;
		//	topLeftTri.faceNormal.y = 0.0f;
		//	topLeftTri.faceNormal.z = -1.0f;


		//	// tr
		//	topLeftTri.vertices[1].x = size.width;// +position.x;
		//	topLeftTri.vertices[1].y = -size.height;// +position.y;
		//	topLeftTri.vertices[1].z = z;
		//	topLeftTri.uvs[1].u = 1.0f;
		//	topLeftTri.uvs[1].v = 0.0f;
		//	topLeftTri.color[1] = color;

		//	// bl
		//	topLeftTri.vertices[2].x = -size.width;// +position.x;
		//	topLeftTri.vertices[2].y = size.height;// +position.y;
		//	topLeftTri.vertices[2].z = z;
		//	topLeftTri.uvs[2].u = 0.0f;
		//	topLeftTri.uvs[2].v = 1.0f;
		//	topLeftTri.color[2] = color;

		//	// tr
		//	bottomRightTri.vertices[0].x = size.width;// +position.x;
		//	bottomRightTri.vertices[0].y = -size.height;// +position.y;
		//	bottomRightTri.vertices[0].z = z;
		//	bottomRightTri.color[0] = color;
		//	bottomRightTri.uvs[0].u = 1.0f;
		//	bottomRightTri.uvs[0].v = 0.0f;
		//	bottomRightTri.faceNormal.x = 0.0f;
		//	bottomRightTri.faceNormal.y = 0.0f;
		//	bottomRightTri.faceNormal.z = -1.0f;

		//	// br
		//	bottomRightTri.vertices[1].x = size.width;// +position.x;
		//	bottomRightTri.vertices[1].y = size.height;// +position.y;
		//	bottomRightTri.vertices[1].z = z;
		//	bottomRightTri.uvs[1].u = 1.0f;
		//	bottomRightTri.uvs[1].v = 1.0f;
		//	bottomRightTri.color[1] = color;

		//	// bl
		//	bottomRightTri.vertices[2].x = -size.width;// +position.x;
		//	bottomRightTri.vertices[2].y = size.height;// +position.y;
		//	bottomRightTri.vertices[2].z = z;
		//	bottomRightTri.uvs[2].u = 0.0f;
		//	bottomRightTri.uvs[2].v = 1.0f;
		//	bottomRightTri.color[2] = color;

		//	Matrix4x4f mat = billboard * rotation;
		//	for (uint32_t i = 0; i < 3; i++)
		//	{
		//		topLeftTri.normals[i] = { 0.0f,0.0f,-1.0f };
		//		bottomRightTri.normals[i] = { 0.0f,0.0f,-1.0f };
		//		// ---------------------------------------------------------------------------
		//		topLeftTri.vertices[i] = topLeftTri.vertices[i] * mat;
		//		bottomRightTri.vertices[i] = bottomRightTri.vertices[i] * mat;
		//	}
		//	rotation.SetIdentity();
		//	tris.emplace_back(topLeftTri);
		//	tris.emplace_back(bottomRightTri);

		//}

	};
}

#endif