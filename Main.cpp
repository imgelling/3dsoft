//#define GAME_SUPPORT_OPENGL
//#define GAME_SUPPORT_DIRECTX9
//#define GAME_SUPPORT_DIRECTX10
#define GAME_SUPPORT_DIRECTX11
//#define GAME_SUPPORT_DIRECTX12

#include "game.h"
#include "GameSoftware3D.h"


class Camera
{
public:
	game::Vector3f position;
	game::Vector3f rotation;
	Camera();
	Camera(const game::Vector3f& position);

private:
};

Camera::Camera()
{
}

Camera::Camera(const game::Vector3f& position)
{
	this->position = position;
}


class Game : public game::Engine
{

public:
	// Pixel renderers
	game::PixelMode pixelMode;
	game::Software3D software3D;


	// 3D stuff
	game::Triangle topLeftTri;
	game::Triangle bottomRightTri;
	game::Recti clip[16];  // in renderer
	std::vector<game::Triangle> clippedTris[16];
	game::Projection projection;
	game::Matrix4x4f projMat;
	std::vector<game::Triangle> tris;
	game::Mesh model;

	std::vector<game::Triangle> quad;
	game::Triangle test;

	Camera camera;
	uint32_t maxFPS;
	uint32_t scene;
	float_t tz;
	uint32_t* texture;
	uint32_t texW;
	uint32_t texH;

	game::FillMode state = game::FillMode::FilledColor;
	game::Pointi resolution = { 1280 , 720 }; //2560, 1440 };
	bool showText;

	Game() : game::Engine()
	{
		ZeroMemory(&projection, sizeof(game::Projection));
		//ZeroMemory(&topLeftTri, sizeof(game::Triangle));  // dont do this
		//ZeroMemory(&bottomRightTri, sizeof(game::Triangle));
		maxFPS = 0;
		scene = 2;
		tz = 0.0f;
		showText = true;
		texture = nullptr;
		texW = 64;
		texH = 64;
	}

	uint32_t numclips = 16;
	void GenerateClips(const uint32_t numberOfClips, game::Recti *clips2, const game::Pointi& size)
	{
		uint32_t cols = (uint32_t)sqrt(numberOfClips);//(int32_t)std::ceil(sqrt(numberOfClips));
		uint32_t rows = (uint32_t)(numberOfClips / (float_t)cols);//(int32_t)std::ceil(numberOfClips / (float_t)cols);

		uint32_t colsize = (uint32_t)std::ceil(size.width / (float_t)cols);
		uint32_t rowsize = (uint32_t)std::ceil(size.height / (float_t)rows);

		uint32_t rc = 0;
		uint32_t cc = 0;
		for (uint32_t row = 0; row < rows; row++)
		{
			rc = 0;
			for (uint32_t col = 0; col < cols; col++)
			{
				uint32_t access = row * cols + col;
				clips2[access].left = (rc) * (colsize - 1);
				clips2[access].right = (clips2[access].left + colsize);
				if (clips2[access].right > size.width - 1) clips2[access].right = size.width - 1;
				clips2[access].top = cc * (rowsize - 1);
				clips2[access].bottom = clips2[access].top + rowsize;
				if (clips2[access].bottom > size.height - 1) clips2[access].bottom = size.height - 1;
				rc++;
			}
			cc++;
		}
	}

	void Initialize()
	{
//#if defined(DEBUG) | defined(_DEBUG)
		//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		//_CrtSetBreakAlloc(613);
//#endif
		game::Attributes attributes;

		attributes.WindowTitle = "Window Title";
		attributes.VsyncOn = false;
		attributes.RenderingAPI = game::RenderAPI::DirectX11;
		attributes.DebugMode = false;
		geSetAttributes(attributes);

		//geSetFrameLock(10);

		GenerateClips(numclips, clip, resolution);
	}

	void LoadContent()
	{
		if (!pixelMode.Initialize(resolution))// 640, 360 }))
		{
			geLogLastError();
		}

		if (!software3D.Initialize(pixelMode.videoBuffer, pixelMode.GetPixelFrameBufferSize(),0))
		{
			geLogLastError();
		}

		software3D.SetState(GAME_SOFTWARE3D_STATE_FILL_MODE, state);

		// cone +z, conex +x, coney +y
		if (!Load("Content/torus2.obj", model))
		{
			std::cout << "Could not load model\n";
		}

		ConvertBlenderToThis(model);

		texture = new uint32_t[texW * texH];
		GenerateCheckerboard(texture, texW, texH);
		software3D._currentTexture = texture;
		software3D._texH = texH;
		software3D._texW = texW;


		//game::ImageLoader imageLoader;
		//uint32_t t = 0;
		//uint32_t* temp = (uint32_t*)imageLoader.Load("Content/skin_adventurer.png", texW, texH, t);
		//texture = new uint32_t[texW * texH];
		//memcpy(texture, temp, (size_t)texW * texH * 4);
		//software3D._currentTexture = texture;
		//software3D._texH = texH;
		//software3D._texW = texW;

		game::Random rnd;
		rnd.NewSeed();

		float_t z = 0.0f;// 100.0f;
		float_t size = 1.0f;

		// tl
		topLeftTri.vertices[0].x = -size;
		topLeftTri.vertices[0].y = -size;
		topLeftTri.vertices[0].z = z;
		topLeftTri.color[0] = game::Colors::Red;
		topLeftTri.uvs[0].u = 0.0f;
		topLeftTri.uvs[0].v = 0.0f;
		topLeftTri.faceNormal.x = 0.0f;
		topLeftTri.faceNormal.y = 0.0f;
		topLeftTri.faceNormal.z = -1.0f;


		// tr
		topLeftTri.vertices[1].x = size;
		topLeftTri.vertices[1].y = -size;
		topLeftTri.vertices[1].z = z;
		topLeftTri.uvs[1].u = 1.0f;
		topLeftTri.uvs[1].v = 0.0f;
		topLeftTri.color[1] = game::Colors::Green;

		// bl
		topLeftTri.vertices[2].x = -size;
		topLeftTri.vertices[2].y = size;
		topLeftTri.vertices[2].z = z;
		topLeftTri.uvs[2].u = 0.0f;
		topLeftTri.uvs[2].v = 1.0f;
		topLeftTri.color[2] = game::Colors::Blue;

		// tr
		bottomRightTri.vertices[0].x = size;
		bottomRightTri.vertices[0].y = -size;
		bottomRightTri.vertices[0].z = z;
		bottomRightTri.color[0] = game::Colors::Green;
		bottomRightTri.uvs[0].u = 1.0f;
		bottomRightTri.uvs[0].v = 0.0f;
		bottomRightTri.faceNormal.x = 0.0f;
		bottomRightTri.faceNormal.y = 0.0f;
		bottomRightTri.faceNormal.z = -1.0f;

		// br
		bottomRightTri.vertices[1].x = size;
		bottomRightTri.vertices[1].y = size;
		bottomRightTri.vertices[1].z = z;
		bottomRightTri.uvs[1].u = 1.0f;
		bottomRightTri.uvs[1].v = 1.0f;
		bottomRightTri.color[1] = game::Colors::White;

		// bl
		bottomRightTri.vertices[2].x = -size;
		bottomRightTri.vertices[2].y = size;
		bottomRightTri.vertices[2].z = z;
		bottomRightTri.uvs[2].u = 0.0f;
		bottomRightTri.uvs[2].v = 1.0f;
		bottomRightTri.color[2] = game::Colors::Blue;

		for (uint32_t i = 0; i < 3; i++)
		{
			topLeftTri.normals[i] = { 0.0f,0.0f,-1.0f };
			bottomRightTri.normals[i] = { 0.0f,0.0f,-1.0f };
		}


		// Generate a 1000 tris
		for (uint32_t i = 0; i < 1000; i++)
		{
			game::Triangle temp(topLeftTri);
			float_t tz = rnd.RndRange(0, 1000) / (float_t)rnd.RndRange(1, 10);
			for (uint32_t v = 0; v < 3; v++)
			{
				temp.vertices[v].x = (float_t)rnd.RndRange(0, resolution.x);
				temp.vertices[v].x = temp.vertices[v].x * 2.0f / (float_t)resolution.x - 1.0f;
				temp.vertices[v].y = (float_t)rnd.RndRange(0, resolution.y);
				temp.vertices[v].y = temp.vertices[v].y * 2.0f / (float_t)resolution.y - 1.0f;
				temp.vertices[v].z = 1000.0f / i;
			}

			game::EdgeEquation e0(temp.vertices[1], temp.vertices[2]);
			game::EdgeEquation e1(temp.vertices[2], temp.vertices[0]);
			game::EdgeEquation e2(temp.vertices[0], temp.vertices[1]);

			float_t area(e0.c + e1.c + e2.c);
			// If area is negative, it means wrong winding
			if (area < 0)
			{
				std::swap(temp.vertices[1], temp.vertices[2]);
			}
			tris.emplace_back(temp);
		}

		// Pre calc projection numbers
		game::my_PerspectiveFOV2(90.0f, resolution.x / (float_t)resolution.y, 0.1f, 100.0f, projection);
		// Pre calc projection matrix
		game::my_PerspectiveFOV2(90.0f, resolution.x / (float_t)resolution.y, 0.1f, 100.0f, projMat);
		quad.reserve(1000);
	}

	void Shutdown()
	{

	}

	void Update(const float_t msElapsed)
	{
		if (geKeyboard.WasKeyPressed(geK_ESCAPE))
		{
			geStopEngine();
		}

		if (geKeyboard.WasKeyPressed(geK_F11))
		{
			geToggleFullscreen();
		}

		if (geKeyboard.WasKeyPressed(geK_F1))
		{
			state++;
			software3D.SetState(GAME_SOFTWARE3D_STATE_FILL_MODE, state);
		}



		if (geKeyboard.WasKeyPressed(geK_LBRACKET))
		{
			software3D.SetState(GAME_SOFTWARE3D_STATE_THREADED, -1);
		}

		if (geKeyboard.WasKeyPressed(geK_RBRACKET))
		{
			software3D.SetState(GAME_SOFTWARE3D_STATE_THREADED, 0);
		}

		if (geKeyboard.WasKeyPressed(geK_1))
		{
			scene = 0;
		}

		if (geKeyboard.WasKeyPressed(geK_2))
		{
			scene = 1;
		}

		if (geKeyboard.WasKeyPressed(geK_3))
		{
			scene = 2;
		}

		if (geKeyboard.IsKeyHeld(geK_W))
		{
			if (geKeyboard.IsKeyHeld(geK_SHIFT))
			{
				//tz -= 5.0f * (msElapsed / 1000.0f);
				camera.position.z += 5.0f * (msElapsed / 1000.0f);
			}
			else
			{
				//tz -= 0.5f * (msElapsed / 1000.0f);
				camera.position.z += 0.1f * (msElapsed / 1000.0f);
			}			
		}

		if (geKeyboard.IsKeyHeld(geK_S))
		{
			if (geKeyboard.IsKeyHeld(geK_SHIFT))
			{
				//tz += 5.0f * (msElapsed / 1000.0f);
				camera.position.z -= 5.0f * (msElapsed / 1000.0f);
			}
			else
			{
				//tz _= 0.5f * (msElapsed / 1000.0f);
				camera.position.z -= 0.1f * (msElapsed / 1000.0f);
			}
		}


		// strafe left
		if (geKeyboard.IsKeyHeld(geK_Q))
		{
			if (geKeyboard.IsKeyHeld(geK_SHIFT))
			{
				//tz -= 5.0f * (msElapsed / 1000.0f);
				camera.position.x -= 5.0f * (msElapsed / 1000.0f);
			}
			else
			{
				//tz -= 0.5f * (msElapsed / 1000.0f);
				camera.position.x -= 0.1f * (msElapsed / 1000.0f);
			}
		}

		// strafe right
		if (geKeyboard.IsKeyHeld(geK_E))
		{
			if (geKeyboard.IsKeyHeld(geK_SHIFT))
			{
				//tz += 5.0f * (msElapsed / 1000.0f);
				camera.position.x += 5.0f * (msElapsed / 1000.0f);
			}
			else
			{
				//tz _= 0.5f * (msElapsed / 1000.0f);
				camera.position.x += 0.1f * (msElapsed / 1000.0f);
			}
		}

		// y is invertex because....
		if (geKeyboard.IsKeyHeld(geK_UP))
		{
			if (geKeyboard.IsKeyHeld(geK_SHIFT))
			{
				//tz -= 5.0f * (msElapsed / 1000.0f);
				camera.position.y -= 5.0f * (msElapsed / 1000.0f);
			}
			else
			{
				//tz -= 0.5f * (msElapsed / 1000.0f);
				camera.position.y -= 0.1f * (msElapsed / 1000.0f);
			}
		}

		// strafe right
		if (geKeyboard.IsKeyHeld(geK_DOWN))
		{
			if (geKeyboard.IsKeyHeld(geK_SHIFT))
			{
				//tz += 5.0f * (msElapsed / 1000.0f);
				camera.position.y += 5.0f * (msElapsed / 1000.0f);
			}
			else
			{
				//tz _= 0.5f * (msElapsed / 1000.0f);
				camera.position.y += 0.1f * (msElapsed / 1000.0f);
			}
		}

		game::Pointi mouse = geMouse.GetPositionRelative();
		if (mouse.x)
		{
			if (geMouse.IsButtonHeld(geM_LEFT))
			{
				camera.rotation.y += mouse.x * (3.14159f / 180.0f);
			}
		}
		if (mouse.y)
		{
			if (geMouse.IsButtonHeld(geM_LEFT))
			{
				camera.rotation.x += -mouse.y * (3.14159f / 180.0f);
			}
		}


		if (geKeyboard.WasKeyPressed(geK_F3))
		{
			showText = !showText;
		}
	}

	inline void PerpectiveDivide(game::Triangle& triangle)
	{
		triangle.vertices[0] /= triangle.vertices[0].w;
		triangle.vertices[1] /= triangle.vertices[1].w;
		triangle.vertices[2] /= triangle.vertices[2].w;
	}

	inline void ScaleToScreen(game::Triangle& triangle) const
	{
		triangle.vertices[0].x += 1.0f;
		triangle.vertices[1].x += 1.0f;
		triangle.vertices[2].x += 1.0f;

		triangle.vertices[0].y += 1.0f;
		triangle.vertices[1].y += 1.0f;
		triangle.vertices[2].y += 1.0f;

		triangle.vertices[0].x *= 0.5f * (float_t)resolution.x;
		triangle.vertices[1].x *= 0.5f * (float_t)resolution.x;
		triangle.vertices[2].x *= 0.5f * (float_t)resolution.x;

		triangle.vertices[0].y *= 0.5f * (float_t)resolution.y;
		triangle.vertices[1].y *= 0.5f * (float_t)resolution.y;
		triangle.vertices[2].y *= 0.5f * (float_t)resolution.y;
	}

	game::Vector3f Vector_IntersectPlane(game::Vector3f& plane_p, game::Vector3f& plane_n, game::Vector3f& lineStart, game::Vector3f& lineEnd, float& t) noexcept
	{
		//plane_n.Normalize();// Vector_Normalise(plane_n);
		float plane_d = -plane_n.Dot(plane_p);// Vector_DotProduct(plane_n, plane_p);
		float ad = lineStart.Dot(plane_n);// Vector_DotProduct(lineStart, plane_n);
		float bd = lineEnd.Dot(plane_n);// Vector_DotProduct(lineEnd, plane_n);
		t = (-plane_d - ad) / (bd - ad);
		game::Vector3f lineStartToEnd = lineEnd - lineStart;// Vector_Sub(lineEnd, lineStart);
		float wt = lineEnd.w - lineStart.w;
		wt = lineStartToEnd.w * t;
		game::Vector3f lineToIntersect = lineStartToEnd * t;// Vector_Mul(lineStartToEnd, t);
		lineToIntersect += lineStart;
		lineToIntersect.w = wt;
		return lineToIntersect;// lineStart + lineToIntersect;// Vector_Add(lineStart, lineToIntersect);
	}


	// Returns +1 if the triangle ABC is CCW, -1 if CW, and 0 if collinear
	float check_winding(game::Vector3f A, game::Vector3f B, game::Vector3f C)
	{
		game::Vector3f AB = B - A; // Vector from A to B
		game::Vector3f AC = C - A; // Vector from A to C
		game::Vector3f N = AB.Cross(AC); // Cross product of AB and AC
		return (N.z); // Sign of the z-component of N
	}

	int Triangle_ClipAgainstPlane(game::Vector3f plane_p, game::Vector3f plane_n, game::Triangle& in_tri, game::Triangle& out_tri1, game::Triangle& out_tri2)
	{
		//// Make sure plane normal is indeed normal
		//plane_n.Normalize();// plane_n = Vector_Normalise(plane_n);


		// Return signed shortest distance from point to plane, plane normal must be normalised
		auto dist = [&](game::Vector3f& p)
			{
				//Vector3f n = p;// Vector_Normalise(p);
				//n.Normalize();
				return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - (plane_n.Dot(plane_p)));// Vector_DotProduct(plane_n, plane_p));
			};

		// Create two temporary storage arrays to classify points either side of plane
		// If distance sign is positive, point lies on "inside" of plane
		game::Vector3f* inside_points[3] = {};  int nInsidePointCount = 0;
		game::Vector3f* outside_points[3] = {}; int nOutsidePointCount = 0;

		// Get signed distance of each point in triangle to plane
		float d0 = dist(in_tri.vertices[0]);
		float d1 = dist(in_tri.vertices[1]);
		float d2 = dist(in_tri.vertices[2]);

		game::Vector3f in_normals[3];
		game::Vector3f out_normals[3];

		game::Vector2f in_uv[3];
		game::Vector2f out_uv[3];

		float in_d[3]{};
		float out_d[3]{};


		if (d0 >= 0.0f)
		{
			inside_points[nInsidePointCount++] = &in_tri.vertices[0];
			in_normals[nInsidePointCount - 1] = in_tri.normals[0];
			in_uv[nInsidePointCount - 1] = in_tri.uvs[0];
			in_d[nInsidePointCount - 1] = in_tri.vertices[0].w;
		}
		else
		{
			outside_points[nOutsidePointCount++] = &in_tri.vertices[0];
			out_normals[nOutsidePointCount - 1] = in_tri.normals[0];
			out_uv[nOutsidePointCount - 1] = in_tri.uvs[0];
			out_d[nOutsidePointCount - 1] = in_tri.vertices[0].w;
		}
		if (d1 >= 0.0f)
		{
			inside_points[nInsidePointCount++] = &in_tri.vertices[1];
			in_normals[nInsidePointCount - 1] = in_tri.normals[1];
			in_uv[nInsidePointCount - 1] = in_tri.uvs[1];
			in_d[nInsidePointCount - 1] = in_tri.vertices[1].w;
		}
		else
		{
			outside_points[nOutsidePointCount++] = &in_tri.vertices[1];
			out_normals[nOutsidePointCount - 1] = in_tri.normals[1];
			out_uv[nOutsidePointCount - 1] = in_tri.uvs[1];
			out_d[nOutsidePointCount - 1] = in_tri.vertices[1].w;
		}
		if (d2 >= 0.0f)
		{
			inside_points[nInsidePointCount++] = &in_tri.vertices[2];
			in_normals[nInsidePointCount - 1] = in_tri.normals[2];
			in_uv[nInsidePointCount - 1] = in_tri.uvs[2];
			in_d[nInsidePointCount - 1] = in_tri.vertices[2].w;
		}
		else
		{
			outside_points[nOutsidePointCount++] = &in_tri.vertices[2];
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
			out_tri1 = in_tri;

			// The inside point is valid, so keep that...
			out_tri1.vertices[0] = *inside_points[0];
			out_tri1.normals[0] = in_normals[0];
			out_tri1.uvs[0] = in_uv[0];


			// but the two new points are at the locations where the 
			// original sides of the triangle (lines) intersect with the plane
			float t = 0.0;

			// First intersection
			out_tri1.vertices[1] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0], t);

			// Correct vertex normal due to clipping
			out_tri1.normals[1].x = t * (out_normals[0].x - in_normals[0].x) + in_normals[0].x;
			out_tri1.normals[1].y = t * (out_normals[0].y - in_normals[0].y) + in_normals[0].y;
			out_tri1.normals[1].z = t * (out_normals[0].z - in_normals[0].z) + in_normals[0].z;
			out_tri1.vertices[1].w = t * (out_d[0] - in_d[0]) + in_d[0];
			out_tri1.uvs[1].x = t * (out_uv[0].x - in_uv[0].x) + in_uv[0].x;
			out_tri1.uvs[1].y = t * (out_uv[0].y - in_uv[0].y) + in_uv[0].y;


			// Second intersection
			out_tri1.vertices[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1], t);

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
			out_tri1 = in_tri;
			out_tri2 = in_tri;

			// The first triangle consists of the two inside points and a new
			// point determined by the location where one side of the triangle
			// intersects with the plane
			out_tri1.vertices[0] = *inside_points[0];
			out_tri1.normals[0] = in_normals[0];
			out_tri1.uvs[0] = in_uv[0];

			out_tri1.vertices[1] = *inside_points[1];
			out_tri1.normals[1] = in_normals[1];
			out_tri1.uvs[1] = in_uv[1];

			float t = 0.0;

			// First intersection
			out_tri1.vertices[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0], t);

			// Correct the vertex normal due to clipping
			out_tri1.normals[2].x = t * (out_normals[0].x - in_normals[0].x) + in_normals[0].x;
			out_tri1.normals[2].y = t * (out_normals[0].y - in_normals[0].y) + in_normals[0].y;
			out_tri1.normals[2].z = t * (out_normals[0].z - in_normals[0].z) + in_normals[0].z;
			out_tri1.vertices[2].w = t * (out_d[0] - in_d[0]) + in_d[0];
			out_tri1.uvs[2].x = t * (out_uv[0].x - in_uv[0].x) + in_uv[0].x;
			out_tri1.uvs[2].y = t * (out_uv[0].y - in_uv[0].y) + in_uv[0].y;


			// 2nd intersection
			out_tri2.vertices[0] = *inside_points[1];
			out_tri2.normals[0] = in_normals[1];
			out_tri2.uvs[0] = in_uv[1];

			out_tri2.vertices[1] = out_tri1.vertices[2];
			out_tri2.normals[1] = out_tri1.normals[2];
			out_tri2.uvs[1] = out_tri1.uvs[2];

			out_tri2.vertices[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0], t);


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

	void Render(const float_t msElapsed)
	{
		static float_t rotation = 0.0f;

		rotation += (2 * 3.14f / 10.0f) * (msElapsed / 1000.0f);
		software3D.time = rotation;
		geClear(GAME_FRAME_BUFFER_BIT, game::Colors::Blue);

		pixelMode.Clear(game::Colors::Black);
		software3D._colorBuffer = pixelMode.videoBuffer;
		software3D.ClearDepth(100.0f);

		quad.clear();

		game::Vector3f t(0.0f, 0.0f, 2.0f);
		t -= camera.position;

		game::Matrix4x4f rotx;
		game::Matrix4x4f roty;
		game::Matrix4x4f rotz;
		game::Matrix4x4f rotationMat;
		game::Matrix4x4f translateMat;
		game::Matrix4x4f viewMat;
		//game::Matrix4x4f viewMat;
		game::Matrix4x4f mvpMat;
		translateMat.SetTranslation(t.x, t.y, t.z); // doesnt work
		rotx.SetRotationX(-camera.rotation.x);	// works
		roty.SetRotationY(-camera.rotation.y);	// works
		rotz.SetRotationZ(0);					// works
		rotationMat = rotx * roty * rotz;  // works
		viewMat = translateMat * rotationMat; // works
		mvpMat = projMat * viewMat; //  * mesh.modelMat; // works


		if (scene == 0)
		{
			test = topLeftTri;
			test.faceNormal = test.faceNormal * rotationMat;
			test.normals[0] = test.normals[0] * rotationMat;
			test.normals[1] = test.normals[1] * rotationMat;
			test.normals[2] = test.normals[2] * rotationMat;
			test.vertices[0] = (topLeftTri.vertices[0] * mvpMat);
			test.vertices[1] = (topLeftTri.vertices[1] * mvpMat);
			test.vertices[2] = (topLeftTri.vertices[2] * mvpMat);

			PerpectiveDivide(test);
			ScaleToScreen(test);
			quad.emplace_back(test);

			test = bottomRightTri;
			test.faceNormal = test.faceNormal * rotationMat;
			test.normals[0] = test.normals[0] * rotationMat;
			test.normals[1] = test.normals[1] * rotationMat;
			test.normals[2] = test.normals[2] * rotationMat;
			test.vertices[0] = bottomRightTri.vertices[0] * mvpMat;
			test.vertices[1] = bottomRightTri.vertices[1] * mvpMat;
			test.vertices[2] = bottomRightTri.vertices[2] * mvpMat;

			PerpectiveDivide(test);
			ScaleToScreen(test);
			quad.emplace_back(test);
		}

		if (scene == 1)
		{
			for (int i = 0; i < tris.size(); i++)
			{
				test = tris[i];
				test.faceNormal = test.faceNormal * rotationMat;
				test.normals[0] = test.normals[0] * rotationMat;
				test.normals[1] = test.normals[1] * rotationMat;
				test.normals[2] = test.normals[2] * rotationMat;
				test.vertices[0] = (tris[i].vertices[0] * mvpMat);
				test.vertices[1] = (tris[i].vertices[1] * mvpMat);
				test.vertices[2] = (tris[i].vertices[2] * mvpMat);

				PerpectiveDivide(test);
				ScaleToScreen(test);
				quad.emplace_back(test);
			}
		}

		if (scene == 2)
		{
			for (int i = 0; i < model.tris.size(); i++)
			{
				test = model.tris[i];
				test.faceNormal = test.faceNormal * rotationMat;
				test.normals[0] = test.normals[0] * rotationMat;
				test.normals[1] = test.normals[1] * rotationMat;
				test.normals[2] = test.normals[2] * rotationMat;
				test.vertices[0] = (model.tris[i].vertices[0] * mvpMat);
				test.vertices[1] = (model.tris[i].vertices[1] * mvpMat);
				test.vertices[2] = (model.tris[i].vertices[2] * mvpMat);

				//if ((test.vertices[0].z < -0.1) ||
				//	(test.vertices[1].z < -0.1) ||
				//	(test.vertices[2].z < -0.1))
				{
					game::Vector3f planePoint(0.0f, 0.0f, 0.0f);
					game::Vector3f planeNormal(0.0f, 0.0f, 1.0f);

					game::Triangle out1;
					game::Triangle out2;
					uint32_t numtris = Triangle_ClipAgainstPlane(planePoint, planeNormal, test, out1, out2);
					if (numtris == 2)
					{
						PerpectiveDivide(out2);
						ScaleToScreen(out2);
						if (check_winding(out2.vertices[0], out2.vertices[1], out2.vertices[2]) < 0)
						{
							std::swap(out2.vertices[1], out2.vertices[0]);
							std::swap(out2.normals[1], out2.normals[0]);
							std::swap(out2.uvs[1], out2.uvs[0]);
							std::swap(out2.color[1], out2.color[0]);
						}
						quad.emplace_back(out2);
					}

					PerpectiveDivide(out1);
					ScaleToScreen(out1);
					if (check_winding(out1.vertices[0], out1.vertices[1], out1.vertices[2]) < 0)
					{
						std::swap(out1.vertices[1], out1.vertices[0]);
						std::swap(out1.normals[1], out1.normals[0]);
						std::swap(out1.uvs[1], out1.uvs[0]);
						std::swap(out1.color[1], out1.color[0]);
					}

					quad.emplace_back(out1);
				}
				//else
				//{
				//	PerpectiveDivide(test);
				//	ScaleToScreen(test);
				//	quad.emplace_back(test);
				//}
			}
		}

		uint32_t fenceCount = 0;
		for (uint32_t c = 0; c < numclips; c++)
		{
			clippedTris[c].clear();
			software3D.ScreenClip(quad, clip[c], clippedTris[c]);
			if (!clippedTris[c].size()) continue;
			std::sort(clippedTris[c].begin(), clippedTris[c].end(), [](const game::Triangle& a, const game::Triangle& b) 
				{
					float_t az = a.vertices[0].z + a.vertices[1].z + a.vertices[2].z;
					float_t bz = b.vertices[0].z + b.vertices[1].z + b.vertices[2].z;
					return az < bz;
				});
			//pixelMode.Rect(clip[c], game::Colors::Yellow);
			software3D.Render(clippedTris[c], clip[c]);
			fenceCount += (uint32_t)clippedTris[c].size();
		}
		software3D.Fence(fenceCount);

		// show depth buffer 468
		if (geKeyboard.IsKeyHeld(geK_D))
		{

			game::Color dColor;
			float_t depth = 0.0f;
			float_t* zbuffer = software3D.depthBuffer;
			uint32_t* vbuffer = pixelMode.videoBuffer;
			for (int pos = 0; pos < pixelMode.GetPixelFrameBufferSize().y * pixelMode.GetPixelFrameBufferSize().x; pos++)
			{
				depth = *zbuffer;
				zbuffer++;
				depth += 1.0f;  // 1 added because z becomes < 1.0f near camera and makes depth > 1.0 making colors
								// go all weird
				depth = 1.0f/depth;
				dColor.Set(1.0f * depth, 1.0f * depth, 1.0f * depth, 1.0f);
				*vbuffer = dColor.packedABGR;
				vbuffer++;
			}
			if (showText)
				pixelMode.Text("Showing Depth buffer.", 0, 60, game::Colors::Yellow, 1);
		}

		if (showText)
		{
			//pixelMode.Text("Translate Z : " + std::to_string(tz), 0, 40, game::Colors::Yellow, 1);
			game::Pointi m = pixelMode.GetScaledMousePosition();
			float_t* zb = software3D.depthBuffer;
			float_t depthAtMouse = zb[(m.y * pixelMode.GetPixelFrameBufferSize().x + m.x)];
			pixelMode.Text("Depth at mouse: " + std::to_string(depthAtMouse), 0, 40, game::Colors::Yellow, 1);


			pixelMode.Text("FPS: " + std::to_string(geGetFramesPerSecond()), 0, 0, game::Colors::Yellow, 1);
			if (geGetFramesPerSecond() > maxFPS) maxFPS = geGetFramesPerSecond();
			pixelMode.Text("Max FPS: " + std::to_string(maxFPS), 0, 10, game::Colors::Yellow, 1);
			std::stringstream ss;
			ss << "Fill Mode: " << state;
			pixelMode.Text(ss.str(), 0, 20, game::Colors::Yellow, 1);
			pixelMode.Text("Working Threads: " + std::to_string(software3D.NumberOfThreads()), 0, 30, game::Colors::Yellow, 1);
		}

		pixelMode.Render();
		if (geKeyboard.WasKeyPressed(geK_F5))
		{
			game::ImageSaver save;
			if (!save.Save(pixelMode.videoBuffer, "test.png", resolution.width, resolution.height, 0))
			{
				std::cout << "save failed---- :(\n";
			}
		}
	}


	void GenerateCheckerboard(uint32_t* buff, unsigned int w, unsigned int h)
	{
		game::Color col1 = game::Colors::Red;
		game::Color col2 = game::Colors::Blue;
		for (uint32_t y = 0; y < h; y++)
		{
			if (y % 2 == 0)
				std::swap(col1, col2);
			for (uint32_t x = 0; x < w; x++)
			{
				if (x % 2 == 0)
					std::swap(col1, col2);
				buff[y * w + x] = col1.packedABGR;
			}
		}
	}

	void ConvertBlenderToThis(game::Mesh& mesh)
	{
		for (int tri = 0; tri < mesh.tris.size(); tri++)
		{

			// make left handed
			//mesh.tris[tri].vertices[0].z = -mesh.tris[tri].vertices[0].z;
			//mesh.tris[tri].vertices[1].z = -mesh.tris[tri].vertices[1].z;
			//mesh.tris[tri].vertices[2].z = -mesh.tris[tri].vertices[2].z;

			std::swap(mesh.tris[tri].vertices[0].y, mesh.tris[tri].vertices[0].z);
			std::swap(mesh.tris[tri].vertices[1].y, mesh.tris[tri].vertices[1].z);
			std::swap(mesh.tris[tri].vertices[2].y, mesh.tris[tri].vertices[2].z);
			//mesh.tris[tri].vertices[0].y = -mesh.tris[tri].vertices[0].y;
			//mesh.tris[tri].vertices[1].y = -mesh.tris[tri].vertices[1].y;
			//mesh.tris[tri].vertices[2].y = -mesh.tris[tri].vertices[2].y;
			//mesh.tris[tri].vertices[0].x = -mesh.tris[tri].vertices[0].x;
			//mesh.tris[tri].vertices[1].x = -mesh.tris[tri].vertices[1].x;
			//mesh.tris[tri].vertices[2].x = -mesh.tris[tri].vertices[2].x;

			//mesh.tris[tri].normals[0].z = -mesh.tris[tri].normals[0].z;
			//mesh.tris[tri].normals[1].z = -mesh.tris[tri].normals[1].z;
			//mesh.tris[tri].normals[2].z = -mesh.tris[tri].normals[2].z;
			//mesh.tris[tri].normals[0].y = -mesh.tris[tri].normals[0].y;
			//mesh.tris[tri].normals[1].y = -mesh.tris[tri].normals[1].y;
			//mesh.tris[tri].normals[2].y = -mesh.tris[tri].normals[2].y;
			std::swap(mesh.tris[tri].normals[0].y, mesh.tris[tri].normals[0].z);
			std::swap(mesh.tris[tri].normals[1].y, mesh.tris[tri].normals[1].z);
			std::swap(mesh.tris[tri].normals[2].y, mesh.tris[tri].normals[2].z);
			//mesh.tris[tri].normals[0].x = -mesh.tris[tri].normals[0].x;
			//mesh.tris[tri].normals[1].x = -mesh.tris[tri].normals[1].x;
			//mesh.tris[tri].normals[2].x = -mesh.tris[tri].normals[2].x;

			std::swap(mesh.tris[tri].faceNormal.y, mesh.tris[tri].faceNormal.z);
			//mesh.tris[tri].faceNormal.z = mesh.tris[tri].faceNormal.z * -1.0f;
			//mesh.tris[tri].faceNormal.y = mesh.tris[tri].faceNormal.y * -1.0f;
			//mesh.tris[tri].faceNormal.x = mesh.tris[tri].faceNormal.x * -1.0f;

			//std::swap(mesh.tris[tri].uvs[0].u, mesh.tris[tri].uvs[0].v);
			//std::swap(mesh.tris[tri].uvs[1].u, mesh.tris[tri].uvs[1].v);
			//std::swap(mesh.tris[tri].uvs[2].u, mesh.tris[tri].uvs[2].v);

			mesh.tris[tri].uvs[0].v = 1.0f - mesh.tris[tri].uvs[0].v;
			mesh.tris[tri].uvs[1].v = 1.0f - mesh.tris[tri].uvs[1].v;
			mesh.tris[tri].uvs[2].v = 1.0f - mesh.tris[tri].uvs[2].v;

			//mesh.tris[tri].uvs[0].u = 1.0f - mesh.tris[tri].uvs[0].u;
			//mesh.tris[tri].uvs[1].u = 1.0f - mesh.tris[tri].uvs[1].u;
			//mesh.tris[tri].uvs[2].u = 1.0f - mesh.tris[tri].uvs[2].u;
		}
	}

	bool Load(std::string file, game::Mesh& mesh)
	{
		std::ifstream f(file.c_str());
		std::vector<game::Vector3f> verts;
		std::vector<game::Vector3f> norms;
		std::vector<float> vcount;
		std::vector<game::Vector3i> fcount;
		std::vector<game::Vector3f> vnorms;

		std::vector<game::Vector2f> uvs;
		game::Vector3f vert;
		bool hasNormals = false;
		bool hasUVs = false;
		char line[256];

		// Check to see if file has normals
		if (f.is_open())
		{
			while (!f.eof())
			{
				f.getline(line, 256);
				if (line[0] == 'v' && line[1] == 'n') hasNormals = true;
				if (line[0] == 'v' && line[1] == 't') hasUVs = true;
			}
		}
		// Reset file
		f.clear();
		f.seekg(0);
		hasNormals = false;

		// Parse the file
		if (f.is_open())
		{
			uint8_t junk = 0;
			uint32_t p1 = 0, p2 = 0, p3 = 0;
			uint32_t n1 = 0, n2 = 0, n3 = 0;
			uint32_t uv1 = 0, uv2 = 0, uv3 = 0;
			while (!f.eof())
			{
				junk = 0;
				f.getline(line, 256);
				std::stringstream ss;
				ss << line;

				if (line[0] == 'v')
				{
					if (line[1] == 'n') // Vertex normals
					{
						ss >> junk >> junk >> vert.x >> vert.y >> vert.z;
						//vert.z = -vert.z;
						norms.emplace_back(vert);
						continue;
					}
					else if (line[1] == 't')  // texture uvs
					{
						ss >> junk >> junk >> vert.x >> vert.y;
						uvs.emplace_back(game::Vector2f(vert.x, vert.y));
						continue;
					}
					else
					{
						ss >> junk >> vert.x >> vert.y >> vert.z;
						verts.emplace_back(vert);
						// start counting verts
						vcount.emplace_back(1.0f);
						// if it has no normals make temporary ones
						if (!hasNormals)
						{
							norms.emplace_back(game::Vector3f(0, 0, 0));
						}
						continue;
					}
				}
				if (line[0] == 'f')
				{
					if (hasUVs && hasNormals)
					{
						ss >> junk >> p1 >> junk >> uv1 >> junk >> n1;
						ss >> p2 >> junk >> uv2 >> junk >> n2;
						ss >> p3 >> junk >> uv3 >> junk >> n3;
					}
					else if (hasNormals)
					{
						ss >> junk >> p1 >> junk >> junk >> n1;
						ss >> p2 >> junk >> junk >> n2;
						ss >> p3 >> junk >> junk >> n3;
					}
					else if (hasUVs)
					{
						// may have to get rid of the ns as junk
						ss >> junk >> p1 >> junk >> uv1 >> junk >> n1;
						ss >> p2 >> junk >> uv2 >> junk >> n2;
						ss >> p3 >> junk >> uv3 >> junk >> n3;
					}
					else
					{
						ss >> junk >> p1 >> p2 >> p3;
					}
					game::Triangle tri;
					// Vertices
					tri.vertices[0] = verts[(size_t)p1 - 1];
					tri.vertices[1] = verts[(size_t)p2 - 1];
					tri.vertices[2] = verts[(size_t)p3 - 1];
					// UV (texture) coords
					if (hasUVs)
					{
						tri.uvs[0] = uvs[(size_t)uv1 - 1];// Vector2d(uvs[uv1 - 1].x, uvs[uv1 - 1].y);
						tri.uvs[1] = uvs[(size_t)uv2 - 1];// Vector2d(uvs[uv2 - 1].x, uvs[uv2 - 1].y);
						tri.uvs[2] = uvs[(size_t)uv3 - 1];// Vector2d(uvs[uv3 - 1].x, uvs[uv3 - 1].y);
					}
					else
					{
						tri.uvs[0];// = game::Vector2f(0, 0);
						tri.uvs[1];// = game::Vector2f(0, 0);
						tri.uvs[2];// = game::Vector2f(0, 0);
					}


					// count the vertices
					if (!hasNormals)
					{
						vcount[(size_t)p1 - 1]++;
						vcount[(size_t)p2 - 1]++;
						vcount[(size_t)p3 - 1]++;
						game::Vector3i t;
						t.x = p1 - 1;
						t.y = p2 - 1;
						t.z = p3 - 1;
						fcount.emplace_back(t);
					}


					game::Vector3f a, b;
					// Calculate the face normal of the triangle
					a = tri.vertices[1] - tri.vertices[0];
					b = tri.vertices[2] - tri.vertices[0];
					// this was changed to make face normals work with gamelib2
					//tri.faceNormal = b.Cross(a);
					tri.faceNormal = a.Cross(b);  // orig


					if (hasNormals)
					{
						// Add the face normal to the vertex normals
						tri.faceNormal.Normalize();
						tri.normals[0] = norms[(size_t)n1 - 1];// * -1.0f;
						tri.normals[1] = norms[(size_t)n2 - 1];// * -1.0f;
						tri.normals[2] = norms[(size_t)n3 - 1];// * -1.0f;
						tri.normals[0].Normalize();
						tri.normals[1].Normalize();
						tri.normals[2].Normalize();
					}
					else
					{
						// Sum the normals
						norms[(size_t)p1 - 1] += tri.faceNormal;// *-1.0f;
						norms[(size_t)p2 - 1] += tri.faceNormal;// *-1.0f;
						norms[(size_t)p3 - 1] += tri.faceNormal;// *-1.0f;
						tri.faceNormal.Normalize();

					}

					mesh.tris.emplace_back(tri);

					continue;
				}
			}

			if (!hasNormals)
			{
				for (int i = 0; i < mesh.tris.size(); i++)
				{
					mesh.tris[i].normals[0] = norms[fcount[i].x] / vcount[fcount[i].x];
					mesh.tris[i].normals[1] = norms[fcount[i].y] / vcount[fcount[i].y];
					mesh.tris[i].normals[2] = norms[fcount[i].z] / vcount[fcount[i].z];
					mesh.tris[i].normals[0].Normalize();
					mesh.tris[i].normals[1].Normalize();
					mesh.tris[i].normals[2].Normalize();
				}
			}
			for (int i = 0; i < mesh.tris.size(); i++)
			{
				mesh.tris[i].color[0] = game::Colors::White;
				mesh.tris[i].color[1] = game::Colors::White;
				mesh.tris[i].color[2] = game::Colors::White;
			}

			return true;
		}
		else return false;

	}
};

int32_t main()
{
	game::Logger logger("Log.html");
	Game engine;
	engine.geSetLogger(&logger);

	testmy_PerspectiveFOV (90.0f, 16.0f / 9.0f, 0.1f, 100.0f);
	testmy_PerspectiveFOV2(90.0f, 16.0f / 9.0f, 0.1f, 100.0f);

	// Create the needed bits for the engine
	if (!engine.geCreate())
	{
		engine.geLogLastError();
		return EXIT_FAILURE;
	}

	// Start the engine
	engine.geStartEngine();

	return EXIT_SUCCESS;
}