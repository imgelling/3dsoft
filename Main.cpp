#define GAME_SUPPORT_DIRECTX11
#include "game.h"
#include "GameSoftware3D.h"



// end GameSoftwareRenderer.h

class Game : public game::Engine
{

public:
	game::PixelMode pixelMode;
	game::Triangle htri;
	game::Triangle htri2;
	game::Software3D software3D;

	game::Recti clip[16];
	float_t projMat[16];
	std::vector<game::Triangle> tris;

	uint32_t maxFPS;

	game::FillMode state = game::FillMode::WireFrameFilled;

	Game() : game::Engine()
	{
		ZeroMemory(&projMat, sizeof(float_t)*16);
		maxFPS = 0;
	}

	void Initialize()
	{
		game::Attributes attributes;

		attributes.WindowTitle = "Window Title";
		attributes.VsyncOn = false;
		attributes.RenderingAPI = game::RenderAPI::DirectX11;
		geSetAttributes(attributes);

		//geSetFrameLock(60);

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

		if (!software3D.Initialize(pixelMode.videoBuffer, pixelMode.GetPixelFrameBufferSize(),16))
		{
			geLogLastError();
		}

		game::Random rnd;

		rnd.NewSeed();

		float z = 0.0f;
		//// Clockwise vertex winding
		//// tl
		//tri.vertices[0].x = 270;
		//tri.vertices[0].y = 130;
		//tri.vertices[0].z = z;
		//tri.color[0] = game::Colors::Red;

		//// br
		//tri.vertices[1].x = 370;
		//tri.vertices[1].y = 230;
		//tri.vertices[1].z = z;
		//tri.color[1] = game::Colors::Blue;

		//// bl
		//tri.vertices[2].x = 270;
		//tri.vertices[2].y = 230;
		//tri.vertices[2].z = z;
		//tri.color[2] = game::Colors::Green;


		//htri = tri;

		float size = 1.0f;

		// tl
		htri.vertices[0].x = -size;
		htri.vertices[0].y = -size;
		htri.vertices[0].z = z;
		htri.color[0] = game::Colors::Red;

		// br
		htri.vertices[1].x = size;
		htri.vertices[1].y = size;
		htri.vertices[1].z = z;
		htri.color[1] = game::Colors::Blue;

		// bl
		htri.vertices[2].x = -size;
		htri.vertices[2].y = size;
		htri.vertices[2].z = z;
		htri.color[2] = game::Colors::Green;


		// tr
		htri2.vertices[0].x = size;
		htri2.vertices[0].y = -size;
		htri2.vertices[0].z = z;
		htri2.color[0] = game::Colors::White;

		// br
		htri2.vertices[1].x = size;
		htri2.vertices[1].y = size;
		htri2.vertices[1].z = z;
		htri2.color[1] = game::Colors::Blue;

		// tl
		htri2.vertices[2].x = -size;
		htri2.vertices[2].y = -size;
		htri2.vertices[2].z = z;
		htri2.color[2] = game::Colors::Red;


		for (uint32_t i = 0; i < 1000; i++)
		{
			game::Triangle temp(htri);
			for (uint32_t v = 0; v < 3; v++)
			{
				temp.vertices[v].x = (float_t)rnd.RndRange(0, 640);
				temp.vertices[v].x = temp.vertices[v].x * 2.0f / 640.0f - 1.0f;
				temp.vertices[v].y = (float_t)rnd.RndRange(0, 360);
				temp.vertices[v].y = temp.vertices[v].y * 2.0f / 360.0f - 1.0f;
				temp.vertices[v].z = rnd.RndRange(0, 1000) / (float)rnd.RndRange(1,1000);
			}
			tris.emplace_back(temp);
		}
		my_PerspectiveFOV2(90.0f, 16.0f / 9.0f, 0.1f, 1000.0f, projMat);

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
			software3D.SetState(GAME_SOFTWARE3D_STATE_FILL_MODE, (uint32_t)state);
		}

		if (geKeyboard.WasKeyPressed(geK_LBRACKET))
		{
			software3D.SetState(GAME_SOFTWARE3D_STATE_THREADED, -1);
		}

		if (geKeyboard.WasKeyPressed(geK_RBRACKET))
		{
			software3D.SetState(GAME_SOFTWARE3D_STATE_THREADED, 2);
		}
	}

	inline game::Triangle RotateTrihZ(const game::Triangle& tri, const float_t theta)
	{
		game::Triangle ret(tri);
		float_t ctheta = cos(theta);
		float_t stheta = sin(theta);
		//for (int i = 0; i < 3; i++)
		{
			ret.vertices[0].x = (tri.vertices[0].x) * ctheta - (tri.vertices[0].y) * stheta;
			ret.vertices[0].y = (tri.vertices[0].x) * stheta + (tri.vertices[0].y) * ctheta;
			ret.vertices[1].x = (tri.vertices[1].x) * ctheta - (tri.vertices[1].y) * stheta;
			ret.vertices[1].y = (tri.vertices[1].x) * stheta + (tri.vertices[1].y) * ctheta;
			ret.vertices[2].x = (tri.vertices[2].x) * ctheta - (tri.vertices[2].y) * stheta;
			ret.vertices[2].y = (tri.vertices[2].x) * stheta + (tri.vertices[2].y) * ctheta;
		}
		return ret;
	}

	inline game::Triangle RotateTrihX(const game::Triangle& tri, const float_t theta)
	{
		game::Triangle ret(tri);
		float_t ctheta = cos(theta);
		float_t stheta = sin(theta);
		//for (int i = 0; i < 3; i++)
		{
			ret.vertices[0].y = (tri.vertices[0].y) * ctheta - (tri.vertices[0].z) * stheta;
			ret.vertices[0].z = (tri.vertices[0].y) * stheta + (tri.vertices[0].z) * ctheta;
			ret.vertices[1].y = (tri.vertices[1].y) * ctheta - (tri.vertices[1].z) * stheta;
			ret.vertices[1].z = (tri.vertices[1].y) * stheta + (tri.vertices[1].z) * ctheta;
			ret.vertices[2].y = (tri.vertices[2].y) * ctheta - (tri.vertices[2].z) * stheta;
			ret.vertices[2].z = (tri.vertices[2].y) * stheta + (tri.vertices[2].z) * ctheta;
		}
		return ret;
	}

	inline game::Triangle RotateTrihY(const game::Triangle& tri, const float_t theta)
	{
		game::Triangle ret(tri);
		float_t ctheta = cos(theta);
		float_t stheta = sin(theta);
		//for (int i = 0; i < 3; i++)
		{
			ret.vertices[0].x = (tri.vertices[0].x) * ctheta + (tri.vertices[0].z) * stheta;
			ret.vertices[0].z = (tri.vertices[0].x) * -stheta + (tri.vertices[0].z) * ctheta;
			ret.vertices[1].x = (tri.vertices[1].x) * ctheta + (tri.vertices[1].z) * stheta;
			ret.vertices[1].z = (tri.vertices[1].x) * -stheta + (tri.vertices[1].z) * ctheta;
			ret.vertices[2].x = (tri.vertices[2].x) * ctheta + (tri.vertices[2].z) * stheta;
			ret.vertices[2].z = (tri.vertices[2].x) * -stheta + (tri.vertices[2].z) * ctheta;
		}
		return ret;
	}

	inline game::Triangle TranslateTri(const game::Triangle& tri, const float_t _x, const float_t _y, const float_t _z)
	{
		game::Triangle ret(tri);
		//for (int i = 0; i < 3; i++)
		{
			ret.vertices[0].x += _x;
			ret.vertices[0].y += _y;
			ret.vertices[0].z += _z;
			ret.vertices[1].x += _x;
			ret.vertices[1].y += _y;
			ret.vertices[1].z += _z;
			ret.vertices[2].x += _x;
			ret.vertices[2].y += _y;
			ret.vertices[2].z += _z;
		}
		return ret;
	}

	// For clipping only need znear so a lot can be precalc for plane
	// make sure plane is normalized
	// precal 
	// plane normal dot plane point
	// store c

	// gl
	static void my_PerspectiveFOV(float_t fov, float_t aspect, float_t nearz, float_t farz, float_t* mret) {
		float_t D2R = 3.14f / 180.0f;
		float_t yScale = 1.0f / tan(D2R * fov / 2);
		float_t xScale = yScale / aspect;
		float_t nearmfar = farz - nearz;
		float_t m[] = {
			xScale, 0,      0,                           0,
			0,      yScale, 0,                           0,
			0,      0,      (farz + nearz) / nearmfar,   1,
			0,      0,      2 * farz * nearz / nearmfar, 0
		};
		memcpy(mret, m, sizeof(float_t) * 16);
	}

	static void my_PerspectiveFOV2(float_t fov, float_t aspect, float_t nearz, float_t farz, float_t (&mret)[16]) {
		float_t D2R = 3.14f / 180.0f;
		float_t yScale = 1.0f / tan(D2R * fov / 2.0f);
		float_t xScale = yScale / aspect;
		float_t nearmfar = farz - nearz;
		float_t m[] = {
			xScale, 0,      0,                           0,
			0,      yScale, 0,                           0,
			0,      0,      farz / nearmfar,			 1,
			0,      0,		-nearz * farz / nearmfar,	 0
		};
		//for (int i = 0; i < 16; i++)
			//mret[i] = m[i];
		memcpy(&mret, m, sizeof(float_t) * 16);
	}

	

	inline game::Triangle Project(const game::Triangle vertex) const
	{
		game::Triangle ret(vertex);

		for (int i = 0; i < 3; i++)
		{
			//game::Vector3f ret;
			ret.vertices[i].x = (ret.vertices[i].x * projMat[0] + ret.vertices[i].y * projMat[4] + ret.vertices[i].z * projMat[8] + ret.vertices[i].w * projMat[12]);
			ret.vertices[i].y = (ret.vertices[i].x * projMat[1] + ret.vertices[i].y * projMat[5] + ret.vertices[i].z * projMat[9] + ret.vertices[i].w * projMat[13]);
			ret.vertices[i].z = (ret.vertices[i].x * projMat[2] + ret.vertices[i].y * projMat[6] + ret.vertices[i].z * projMat[10] + ret.vertices[i].w * projMat[14]);
			ret.vertices[i].w = (ret.vertices[i].x * projMat[3] + ret.vertices[i].y * projMat[7] + ret.vertices[i].z * projMat[11] + ret.vertices[i].w * projMat[15]);
			//ret.vertices[i].x = ((vertex.vertices[i].x) * xScale);
			{
				// this scale is not really part of projection
				ret.vertices[i].x *= 1.0f / ret.vertices[i].z;
				ret.vertices[i].x += 1.0f;
				ret.vertices[i].x *= 0.5f * (float_t)pixelMode.GetPixelFrameBufferSize().x;// cale);// (vertex.x * 2.0 / (float_t)pixelMode.GetPixelFrameBufferSize().x) - 1.0f;
				
			}
			//ret.vertices[i].y = ((vertex.vertices[i].y) * yScale);
			{
				// this scale is not really part of projection
				ret.vertices[i].y *= 1.0f / ret.vertices[i].z;
				ret.vertices[i].y += 1.0f;
				ret.vertices[i].y *= 0.5f * (float_t)pixelMode.GetPixelFrameBufferSize().y;				
			}
			//ret.vertices[i].z = (ret.vertices[i].z * zScale - 1.0f) * zScale2;
			//std::cout << ret.vertices[i].z << "\n";
		}
		return ret;
	}

	void Render(const float_t msElapsed)
	{
		static float_t rotation = 0.0f;

		rotation += (2 * 3.14f / 10.0f) * (msElapsed / 1000.0f);
		geClear(GAME_FRAME_BUFFER_BIT, game::Colors::Blue);

		pixelMode.Clear(game::Colors::Black);
		software3D.ClearDepth();

		std::vector<game::Triangle> quad;
		game::Triangle test;

		test = RotateTrihY(htri, rotation);
		test = RotateTrihX(test, -rotation);
		test = RotateTrihZ(test, rotation * 0.5f);
		test = TranslateTri(test, 0.0f, 0.0f, 2.0f);
		test = Project(test);
		quad.emplace_back(test);

		test = RotateTrihY(htri2, rotation);
		test = RotateTrihX(test, -rotation);
		test = RotateTrihZ(test, rotation * 0.5f);
		test = TranslateTri(test, 0.0f, 0.0f, 2.0f);
		test = Project(test);
		quad.emplace_back(test);

		//for (int i = 0; i < tris.size(); i++)
		//{
		//	test = RotateTrihY(tris[i], rotation);
		//	test = RotateTrihX(test, -rotation);
		//	test = RotateTrihZ(test, rotation * 0.5f);
		//	test = TranslateTri(test, 0.0f, 0.0f, 1.5f);
		//	test = Project(test);
		//	quad.emplace_back(test);
		//}
		//

		//game::Recti f(0, 0, 639, 359);
		software3D.Render(quad, clip[0]);
		software3D.Render(quad, clip[1]);
		software3D.Render(quad, clip[2]);
		software3D.Render(quad, clip[3]);
		software3D.Render(quad, clip[4]);
		software3D.Render(quad, clip[5]);
		software3D.Render(quad, clip[6]);
		software3D.Render(quad, clip[7]);
		software3D.Render(quad, clip[8]);
		software3D.Render(quad, clip[9]);
		software3D.Render(quad, clip[10]);
		software3D.Render(quad, clip[11]);
		software3D.Render(quad, clip[12]);
		software3D.Render(quad, clip[13]);
		software3D.Render(quad, clip[14]);
		software3D.Render(quad, clip[15]);

		//software3D.Render(quad, f);
		software3D.Fence(quad.size() * 16);
		//for (int i = 0; i < 16; i++)
		//	pixelMode.Rect(clip[i], game::Colors::Yellow);
		

		pixelMode.Text("FPS: " + std::to_string(geGetFramesPerSecond()), 0, 0, game::Colors::Yellow, 1);
		if (geGetFramesPerSecond() > maxFPS) maxFPS = geGetFramesPerSecond();
		pixelMode.Text("Max FPS: " + std::to_string(maxFPS), 0, 10, game::Colors::Yellow, 1);
		std::stringstream ss;
		ss << "Fill Mode: " << state;
		pixelMode.Text(ss.str(), 0, 20, game::Colors::Yellow, 1);
		pixelMode.Text("Working Threads: " + std::to_string(software3D.NumberOfThreads()), 0, 30, game::Colors::Yellow, 1);


		pixelMode.Render();
	}
};

int32_t main()
{
	game::Logger logger("Log.html");
	Game engine;
	engine.geSetLogger(&logger);

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