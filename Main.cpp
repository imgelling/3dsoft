#define GAME_SUPPORT_DIRECTX11
#include "game.h"
#include "GameSoftware3D.h"



class Game : public game::Engine
{

public:
	// Pixel renderers
	game::PixelMode pixelMode;
	game::Software3D software3D;


	// 3D stuff
	game::Triangle topLeftTri;
	game::Triangle bottomRightTri;
	game::Recti clip[16];
	std::vector<game::Triangle> clippedTris[16];
	//float_t projMat[16];
	game::Projection projection;
	std::vector<game::Triangle> tris;
	game::Mesh model;

	uint32_t maxFPS;

	uint32_t scene;
	float_t tz;

	game::FillMode state = game::FillMode::WireFrameFilled;

	Game() : game::Engine()
	{
		ZeroMemory(&projection, sizeof(game::Projection));
		maxFPS = 0;
		scene = 2;
		tz = 0.0f;
	}

	void Initialize()
	{
		game::Attributes attributes;

		attributes.WindowTitle = "Window Title";
		attributes.VsyncOn = false;
		attributes.RenderingAPI = game::RenderAPI::DirectX11;
		geSetAttributes(attributes);

		//geSetFrameLock(60);

		// tl
		// x = rectx
		// y = recty
		// right = 
		// check the webpage

		//tl
		clip[0].x = 0;
		clip[0].y = 0;
		clip[0].right = (640 - 1) / 2;
		clip[0].bottom = (360 - 1) / 2;

		// tr
		clip[1].x = (640 - 1) / 2;
		clip[1].y = 0;
		clip[1].right = 640 - 1;
		clip[1].bottom = (360 - 1) / 2;

		// bl
		clip[2].x = 0;
		clip[2].y = (360 - 1) / 2;
		clip[2].right = (640 - 1) / 2;
		clip[2].bottom = 360 - 1;

		// br
		clip[3].x = (640 - 1) / 2;
		clip[3].y = (360 - 1) / 2;
		clip[3].right = 640 - 1;
		clip[3].bottom = 360 - 1;


		// --------------tr
		//tl
		clip[4].x = 0 + 640;
		clip[4].y = 0;
		clip[4].right = (640 - 1) / 2 + 640;
		clip[4].bottom = (360 - 1) / 2;

		// tr
		clip[5].x = (640 - 1) / 2 + 640;
		clip[5].y = 0;
		clip[5].right = 640 - 1 + 640;
		clip[5].bottom = (360 - 1) / 2;

		// bl
		clip[6].x = 0 + 640;
		clip[6].y = (360 - 1) / 2;
		clip[6].right = (640 - 1) / 2 + 640;
		clip[6].bottom = 360 - 1;

		// br
		clip[7].x = (640 - 1) / 2 + 640;
		clip[7].y = (360 - 1) / 2;
		clip[7].right = 640 - 1 + 640;
		clip[7].bottom = 360 - 1;


		// --------------bl
		//tl
		clip[8].x = 0;
		clip[8].y = 0 + 360;
		clip[8].right = (640 - 1) / 2;
		clip[8].bottom = (360 - 1) / 2 + 360;

		// tr
		clip[9].x = (640 - 1) / 2;
		clip[9].y = 0 + 360;
		clip[9].right = 640 - 1;
		clip[9].bottom = (360 - 1) / 2 + 360;

		// bl
		clip[10].x = 0;
		clip[10].y = (360 - 1) / 2 + 360;
		clip[10].right = (640 - 1) / 2;
		clip[10].bottom = 360 - 1 + 360;

		// br
		clip[11].x = (640 - 1) / 2;
		clip[11].y = (360 - 1) / 2 + 360;
		clip[11].right = 640 - 1;
		clip[11].bottom = 360 - 1 + 360;


		// ----------------br
		//tl
		clip[12].x = 0 + 640;
		clip[12].y = 0 + 360;
		clip[12].right = (640 - 1) / 2 + 640;
		clip[12].bottom = (360 - 1) / 2 + 360;

		// tr
		clip[13].x = (640 - 1) / 2 + 640;
		clip[13].y = 0 + 360;
		clip[13].right = 640 - 1 + 640;
		clip[13].bottom = (360 - 1) / 2 + 360;

		// bl
		clip[14].x = 0 + 640;
		clip[14].y = (360 - 1) / 2 + 360;
		clip[14].right = (640 - 1) / 2 + 640;
		clip[14].bottom = 360 - 1 + 360;

		// br
		clip[15].x = (640 - 1) / 2 + 640;
		clip[15].y = (360 - 1) / 2 + 360;
		clip[15].right = 640 - 1 + 640;
		clip[15].bottom = 360 - 1 + 360;
	}

	void LoadContent()
	{
		if (!pixelMode.Initialize({ 1280, 720 }))// 640, 360 }))
		{
			geLogLastError();
		}

		if (!software3D.Initialize(pixelMode.videoBuffer, pixelMode.GetPixelFrameBufferSize(),0))
		{
			geLogLastError();
		}

		if (!Load("Content/monkeysmooth.obj", model))
		{
			std::cout << "Could not load model\n";
		}

		game::Random rnd;

		rnd.NewSeed();

		float z = 0.0f;// 100.0f;
		float size = 1.0f;

		// tl
		topLeftTri.vertices[0].x = -size;
		topLeftTri.vertices[0].y = -size;
		topLeftTri.vertices[0].z = z;
		topLeftTri.color[0] = game::Colors::Red;

		// tr
		topLeftTri.vertices[1].x = size;
		topLeftTri.vertices[1].y = -size;
		topLeftTri.vertices[1].z = z;
		topLeftTri.color[1] = game::Colors::Green;

		// bl
		topLeftTri.vertices[2].x = -size;
		topLeftTri.vertices[2].y = size;
		topLeftTri.vertices[2].z = z;
		topLeftTri.color[2] = game::Colors::Blue;


		// tr
		bottomRightTri.vertices[0].x = size;
		bottomRightTri.vertices[0].y = -size;
		bottomRightTri.vertices[0].z = z;
		bottomRightTri.color[0] = game::Colors::Green;

		// br
		bottomRightTri.vertices[1].x = size;
		bottomRightTri.vertices[1].y = size;
		bottomRightTri.vertices[1].z = z;
		bottomRightTri.color[1] = game::Colors::White;

		// bl
		bottomRightTri.vertices[2].x = -size;
		bottomRightTri.vertices[2].y = size;
		bottomRightTri.vertices[2].z = z;
		bottomRightTri.color[2] = game::Colors::Blue;


		// Generate a 1000 tris
		for (uint32_t i = 0; i < 1000; i++)
		{
			game::Triangle temp(topLeftTri);
			float tz = rnd.RndRange(0, 1000) / (float)rnd.RndRange(1, 10);
			for (uint32_t v = 0; v < 3; v++)
			{
				temp.vertices[v].x = (float_t)rnd.RndRange(0, 1280);
				temp.vertices[v].x = temp.vertices[v].x * 2.0f / 1280.0f - 1.0f;
				temp.vertices[v].y = (float_t)rnd.RndRange(0, 720);
				temp.vertices[v].y = temp.vertices[v].y * 2.0f / 720.0f - 1.0f;
				temp.vertices[v].z = tz;// 0.0f;// -(float_t)i;// / 100.0f;
			}

			game::EdgeEquation e0(temp.vertices[1], temp.vertices[2]);
			game::EdgeEquation e1(temp.vertices[2], temp.vertices[0]);
			game::EdgeEquation e2(temp.vertices[0], temp.vertices[1]);

			float area(e0.c + e1.c + e2.c);
			// If area is negative, it means wrong winding
			if (area < 0)
			{
				std::swap(temp.vertices[1], temp.vertices[2]);
			}
			tris.emplace_back(temp);
		}

		// Pre calc projection matrix
		my_PerspectiveFOV2(90.0f, 16.0f / 9.0f, 0.1f, 100.0f, projection);
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

		if (geKeyboard.WasKeyPressed(geK_W))
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

		if (geKeyboard.IsKeyHeld(geK_UP))
		{
			tz -= 0.1f;
		}

		if (geKeyboard.IsKeyHeld(geK_DOWN))
		{
			tz += 0.1f;
		}
	}

	// left handed -1 to 1
	static void my_PerspectiveFOV(float_t fov, float_t aspect, float_t nearz, float_t farz, game::Projection& proj)
	{
		float_t D2R = 3.14159f / 180.0f;
		float_t yScale = 1.0f / tan(D2R * fov / 2);
		float_t xScale = yScale / aspect;
		float_t nearmfar = farz - nearz;
		//float_t m[] = {
		//	xScale, 0,      0,                           0,
		//	0,      yScale, 0,                           0,
		//	0,      0,      (farz + nearz) / nearmfar,   1,
		//	0,      0,      (2 * farz * nearz) / nearmfar, 0
		//};
		//memcpy(mret, m, sizeof(float_t) * 16);
		proj.a = xScale;
		proj.b = yScale;
		proj.c = (farz + nearz) / nearmfar;
		proj.d = 1.0f;
		proj.e = (2.0f * farz * nearz) / nearmfar;
	}

	// left handed (D3DXFovLH) 0 to +1
	////		float_t m[] = {
	//  xScale, 0, 0, 0,
	//	0, yScale, 0, 0,
	//	0, 0, farz / (farz - nearz), 1,
	//	0, 0, -(nearz * farz) / (farz - nearz), 0
	//	};
	//	//
	static void my_PerspectiveFOV2(const float_t fov, const float_t aspect, const float_t nearz, const float_t farz, game::Projection& proj)
	{
		float_t D2R = 3.14159f / 180.0f;

		float_t halfFOV = tan(0.25f / 2.0f);
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

	static void testmy_PerspectiveFOV(const float_t fov, const float_t aspect, const float_t nearz, const float_t farz)
	{
		float_t halfFOV = tan(0.25f / 2.0f);
		float_t yScale = 1.0f / halfFOV;// 1.0f / tan(0.25 / 2.0f);
		float_t xScale = 1.0f / (aspect * halfFOV);//yScale / aspect;
		float_t m[] = {
			xScale, 0,      0,                                         0,
			0,      yScale, 0,                                         0,
			0,      0,      (farz + nearz) / (farz - nearz),    	 1.0f,
			0,      0,		(2.0f * farz * nearz) / (farz - nearz),    0
		};
		//proj.a = xScale;
		//proj.b = yScale;
		//proj.c = farz / nearmfar;
		//proj.d = 1.0f;
		//proj.e = -(nearz * farz) / nearmfar;
		//memcpy(&mret, m, sizeof(float_t) * 16);
		int e = 0;
		game::Vector3f N(0.0f, 0.0f, 0.1f);// glm::vec4 N = Projection * glm::vec4(0.f, 0.f, Near, 1.f);
		game::Vector3f F(0.0f, 0.0f, 100.0f); //glm::vec4 F = Projection * glm::vec4(0.f, 0.f, Far, 1.f);
		game::Vector3f ret;
		ret.x = (N.x * m[0] + N.y * m[4] + N.z * m[8] + N.w * m[12]);
		ret.y = (N.x * m[1] + N.y * m[5] + N.z * m[9] + N.w * m[13]);
		ret.z = (N.x * m[2] + N.y * m[6] + N.z * m[10] + N.w * m[14]);
		ret.w = (N.x * m[3] + N.y * m[7] + N.z * m[11] + N.w * m[15]);
		ret /= ret.w;
		e += (ret.z != -1.0f);
		std::cout << "\nFOV -1 to +1\n";
		std::cout << "Nz = " << ret.z << "\n";
		std::cout << "error = " << e << "\n";


		N = F;
		ret.x = (N.x * m[0] + N.y * m[4] + N.z * m[8] + N.w * m[12]);
		ret.y = (N.x * m[1] + N.y * m[5] + N.z * m[9] + N.w * m[13]);
		ret.z = (N.x * m[2] + N.y * m[6] + N.z * m[10] + N.w * m[14]);
		ret.w = (N.x * m[3] + N.y * m[7] + N.z * m[11] + N.w * m[15]);
		ret /= ret.w;
		e += (ret.z != 1.0f);
		std::cout << "Fz = " << ret.z << "\n";
		std::cout << "error = " << e << "\n";

	}

	static void testmy_PerspectiveFOV2(const float_t fov, const float_t aspect, const float_t nearz, const float_t farz)
	{
		float_t halfFOV = tan(0.25f / 2.0f);
		float_t yScale = 1.0f / halfFOV;// 1.0f / tan(0.25 / 2.0f);
		float_t xScale = 1.0f / (aspect * halfFOV);//yScale / aspect;
		
		float_t m[] = {
			xScale, 0,      0,                           0,
			0,      yScale, 0,                           0,
			0,      0,      farz / (farz - nearz),			 1,
			0,      0,		-(nearz * farz) / (farz - nearz),	 0
		};
		//proj.a = xScale;
		//proj.b = yScale;
		//proj.c = farz / nearmfar;
		//proj.d = 1.0f;
		//proj.e = -(nearz * farz) / nearmfar;
		//memcpy(&mret, m, sizeof(float_t) * 16);
		int e = 0;
		game::Vector3f N(0.0f, 0.0f, 0.1f);// glm::vec4 N = Projection * glm::vec4(0.f, 0.f, Near, 1.f);
		game::Vector3f F(0.0f, 0.0f, 100.0f); //glm::vec4 F = Projection * glm::vec4(0.f, 0.f, Far, 1.f);
		game::Vector3f ret;
		//ret.x = (N.x * m[0] + N.y * m[4] + N.z * m[8] + N.w * m[12]);
		//ret.y = (N.x * m[1] + N.y * m[5] + N.z * m[9] + N.w * m[13]);
		//ret.z = (N.x * m[2] + N.y * m[6] + N.z * m[10] + N.w * m[14]);
		//ret.w = (N.x * m[3] + N.y * m[7] + N.z * m[11] + N.w * m[15]);
		ret.x = (N.x * m[0]);// +N.y * m[4] + N.z * m[8] + N.w * m[12]);
		ret.y = N.y * m[5];// (N.x * m[1] + N.y * m[5] + N.z * m[9] + N.w * m[13]);
		ret.z = /*(N.x * m[2] + N.y * m[6] + */(N.z * m[10] + N.w * m[14]);
		ret.w = /*(N.x * m[3] + N.y * m[7] + */ N.z * m[11];// +N.w * m[15]);
		ret /= ret.w;
		e += (ret.z != 0);
		std::cout << "\nFOV2 0 to +1\n";
		std::cout << "Nz = " << ret.z << "\n";
		std::cout << "error = " << e << "\n";


		N = F;
		ret.x = (N.x * m[0] + N.y * m[4] + N.z * m[8] + N.w * m[12]);
		ret.y = (N.x * m[1] + N.y * m[5] + N.z * m[9] + N.w * m[13]);
		ret.z = (N.x * m[2] + N.y * m[6] + N.z * m[10] + N.w * m[14]);
		ret.w = (N.x * m[3] + N.y * m[7] + N.z * m[11] + N.w * m[15]);
		ret /= ret.w;
		e += (ret.z != 1);
		std::cout << "Fz = " << ret.z << "\n";
		std::cout << "error = " << e << "\n";

	}

	inline game::Triangle Project(const game::Triangle& vertex, const game::Projection& proj) const noexcept
	{
		game::Triangle ret;

		for (int i = 0; i < 3; i++)
		{
			ret.vertices[i].x = vertex.vertices[i].x * proj.a;// projMat[0];// xScale;// (ret.vertices[i].x * projMat[0] + ret.vertices[i].y * projMat[4] + ret.vertices[i].z * projMat[8] + ret.vertices[i].w * projMat[12]);
			ret.vertices[i].y = vertex.vertices[i].y * proj.b;// projMat[5];// yScale;// (ret.vertices[i].x * projMat[1] + ret.vertices[i].y * projMat[5] + ret.vertices[i].z * projMat[9] + ret.vertices[i].w * projMat[13]);
			ret.vertices[i].z = (vertex.vertices[i].z * proj.c) + (vertex.vertices[i].w * proj.e);// projMat[10] + ret.vertices[i].w * projMat[14]);
			ret.vertices[i].w = vertex.vertices[i].z;// *proj.d;// projMat[15];// (ret.vertices[i].x * projMat[3] + ret.vertices[i].y * projMat[7] + ret.vertices[i].z * projMat[11] + ret.vertices[i].w * projMat[15]);
			
			// Projection divide
			ret.vertices[i] /= ret.vertices[i].w;
			//std::cout << ret.vertices[i].z;
			{
				// this scale is not really part of projection
				//ret.vertices[i].x *= ret.vertices[i].w;// 1.0f / ret.vertices[i].w;
				ret.vertices[i].x += 1.0f;
				ret.vertices[i].x *= 0.5f * (float_t)pixelMode.GetPixelFrameBufferSize().x;// cale);// (vertex.x * 2.0 / (float_t)pixelMode.GetPixelFrameBufferSize().x) - 1.0f;
				
			}
			{
				// this scale is not really part of projection
				//ret.vertices[i].y *= ret.vertices[i].w;//1.0f / ret.vertices[i].w;
				ret.vertices[i].y += 1.0f;
				ret.vertices[i].y *= 0.5f * (float_t)pixelMode.GetPixelFrameBufferSize().y;				
			}
			// needed?
			//ret.vertices[i].z = 1.0f;// ret.vertices[i].w;
			ret.color[i] = vertex.color[i];
		}
		return ret;
	}

	void Render(const float_t msElapsed)
	{
		static float_t rotation = 0.0f;

		rotation += (2 * 3.14f / 10.0f) * (msElapsed / 1000.0f);
		geClear(GAME_FRAME_BUFFER_BIT, game::Colors::Blue);

		pixelMode.Clear(game::Colors::Black);
		software3D.ClearDepth(100.0f);

		std::vector<game::Triangle> quad;
		game::Triangle test;

		if (scene == 0)
		{
			game::Vector3f t(0.0f, 0.0f, 10.0f+tz);
			test = software3D.RotateXYZ(topLeftTri, -rotation, rotation, rotation * 0.5f);
			test = software3D.Translate(test, t);
			test = Project(test,projection);
			quad.emplace_back(test);

			test = software3D.RotateXYZ(bottomRightTri, -rotation, rotation, rotation * 0.5f);
			test = software3D.Translate(test, t);
			test = Project(test, projection);
			quad.emplace_back(test);
		}

		if (scene == 1)
		{
			game::Vector3f t(0.0f, 0.0f, 10.0f + tz);
			for (int i = 0; i < tris.size(); i++)
			{
				test = software3D.RotateXYZ(tris[i], -rotation, rotation, rotation * 0.5f);
				test = software3D.Translate(test, t);
				test = Project(test, projection);
				quad.emplace_back(test);
			}
		}

		if (scene == 2)
		{
			game::Vector3f t(0.0f, 0.0f, 10.0f + tz);
			for (int i = 0; i < model.tris.size(); i++)
			{
				test = software3D.RotateXYZ(model.tris[i], -rotation, rotation, rotation * 0.5f);
				test = software3D.Translate(test, t);
				test = Project(test, projection);
				quad.emplace_back(test);
			}
		}

		// max 1759 htris
		uint32_t fenceCount = 0;
		for (int c = 0; c < 16; c++)
		{
			software3D.Clip(quad, clip[c], clippedTris[c]);
			if (!clippedTris[c].size()) continue;
			std::sort(clippedTris[c].begin(), clippedTris[c].end(), [](const game::Triangle& a, const game::Triangle& b) 
				{
					float az = a.vertices[0].z + a.vertices[1].z + a.vertices[2].z;
					float bz = b.vertices[0].z + b.vertices[1].z + b.vertices[2].z;
					return az > bz;
				});
			software3D.Render(clippedTris[c], clip[c]);
			fenceCount += (uint32_t)clippedTris[c].size();
		}
		software3D.Fence(fenceCount);

		//// max 1052
		//software3D.Render(quad, clip[0]);
		//software3D.Render(quad, clip[1]);
		//software3D.Render(quad, clip[2]);
		//software3D.Render(quad, clip[3]);
		//software3D.Render(quad, clip[4]);
		//software3D.Render(quad, clip[5]);
		//software3D.Render(quad, clip[6]);
		//software3D.Render(quad, clip[7]);
		//software3D.Render(quad, clip[8]);
		//software3D.Render(quad, clip[9]);
		//software3D.Render(quad, clip[10]);
		//software3D.Render(quad, clip[11]);
		//software3D.Render(quad, clip[12]);
		//software3D.Render(quad, clip[13]);
		//software3D.Render(quad, clip[14]);
		//software3D.Render(quad, clip[15]);
		//software3D.Fence(quad.size() * 16);// fenceCount);

		//// max 480fps
		//game::Recti f(0, 0, 1279, 719);
		//software3D.Render(quad, f);
		//software3D.Fence(quad.size());

		//for (int i = 0; i < 16; i++)
			//pixelMode.Rect(clip[i], game::Colors::Yellow);

		// show depth buffer
		if (geKeyboard.IsKeyHeld(geK_D))
		{

			game::Color dColor;
			float depth = 0.0f;
			float* zbuffer = software3D.GetDepth();
			uint32_t* vbuffer = pixelMode.videoBuffer;
			for (int pos = 0; pos < pixelMode.GetPixelFrameBufferSize().y * pixelMode.GetPixelFrameBufferSize().x; pos++)
			{
				depth = *zbuffer;
				zbuffer++;
				//depth += 1.0f;
				depth = 1.0f/depth;
				dColor.Set(1.0f * depth, 1.0f * depth, 1.0f * depth, 1.0f);
				*vbuffer = dColor.packedABGR;
				vbuffer++;
			}
			pixelMode.Text("Showing Depth buffer.", 0, 40, game::Colors::Yellow, 1);
		}
		pixelMode.Text("Translate Z : " + std::to_string(tz), 0, 50, game::Colors::Yellow, 1);
		

		pixelMode.Text("FPS: " + std::to_string(geGetFramesPerSecond()), 0, 0, game::Colors::Yellow, 1);
		if (geGetFramesPerSecond() > maxFPS) maxFPS = geGetFramesPerSecond();
		pixelMode.Text("Max FPS: " + std::to_string(maxFPS), 0, 10, game::Colors::Yellow, 1);
		std::stringstream ss;
		ss << "Fill Mode: " << state;
		pixelMode.Text(ss.str(), 0, 20, game::Colors::Yellow, 1);
		pixelMode.Text("Working Threads: " + std::to_string(software3D.NumberOfThreads()), 0, 30, game::Colors::Yellow, 1);


		pixelMode.Render();
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

		// Parse the file
		if (f.is_open())
		{
			unsigned char junk = 0;
			unsigned int p1 = 0, p2 = 0, p3 = 0;
			unsigned int n1 = 0, n2 = 0, n3 = 0;
			unsigned int uv1 = 0, uv2 = 0, uv3 = 0;
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
						// added new
						vert.y = vert.y * -1;
						vert.z = vert.z * -1;
						//vert.x = vert.x * -1;
						// Also swapped ps and ns 2 and 3 for winding
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
					tri.vertices[0] = verts[p1 - 1];
					tri.vertices[1] = verts[p3 - 1];
					tri.vertices[2] = verts[p2 - 1];
					// UV (texture) coords
					if (hasUVs)
					{
						tri.uvs[0] = uvs[uv1 - 1];// Vector2d(uvs[uv1 - 1].x, uvs[uv1 - 1].y);
						tri.uvs[1] = uvs[uv3 - 1];// Vector2d(uvs[uv2 - 1].x, uvs[uv2 - 1].y);
						tri.uvs[2] = uvs[uv2 - 1];// Vector2d(uvs[uv3 - 1].x, uvs[uv3 - 1].y);
					}
					else
					{
						tri.uvs[0] = game::Vector2f(0, 0);
						tri.uvs[1] = game::Vector2f(0, 0);
						tri.uvs[2] = game::Vector2f(0, 0);
					}


					// count the vertices
					if (!hasNormals)
					{
						vcount[p1 - 1]++;
						vcount[p3 - 1]++;
						vcount[p2 - 1]++;
						game::Vector3i t;
						t.x = p1 - 1;
						t.y = p3 - 1;
						t.z = p2 - 1;
						fcount.emplace_back(t);
					}


					game::Vector3f a, b;
					// Calculate the face normal of the triangle
					a = tri.vertices[1] - tri.vertices[0];
					b = tri.vertices[2] - tri.vertices[0];
					tri.faceNormal = a.Cross(b);


					if (hasNormals)
					{
						// Add the face normal to the vertex normals
						tri.faceNormal.Normalize();
						tri.normals[0] = norms[n1 - 1];
						tri.normals[1] = norms[n3 - 1];
						tri.normals[2] = norms[n2 - 1];
					}
					else
					{
						// Sum the normals
						norms[p1 - 1] += tri.faceNormal;
						norms[p3 - 1] += tri.faceNormal;
						norms[p2 - 1] += tri.faceNormal;
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
				mesh.tris[i].color[0] = game::Colors::DarkGray;
				mesh.tris[i].color[1] = game::Colors::DarkGray;
				mesh.tris[i].color[2] = game::Colors::DarkGray;
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

	engine.testmy_PerspectiveFOV (90.0f, 16.0f / 9.0f, 0.1f, 100.0f);
	engine.testmy_PerspectiveFOV2(90.0f, 16.0f / 9.0f, 0.1f, 100.0f);

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