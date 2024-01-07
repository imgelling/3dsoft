#define GAME_SUPPORT_DIRECTX11
#include "game.h"
#include "GameSoftware3D.h"



// end GameSoftwareRenderer.h

class Game : public game::Engine
{

public:
	game::PixelMode pixelMode;
	game::Triangle tri;
	game::Triangle tri2;
	game::Triangle htri;
	game::Software3D software3D;

	game::Recti clip[4];
	std::vector<game::Triangle> tris;
	std::vector<game::Triangle> clippedTris;

	game::FillMode state = game::FillMode::WireFrameFilled;

	Game() : game::Engine()
	{
		ZeroMemory(&tri, sizeof(game::Triangle));
	}

	void Initialize()
	{
		game::Attributes attributes;

		attributes.WindowTitle = "Window Title";
		attributes.VsyncOn = false;
		attributes.RenderingAPI = game::RenderAPI::DirectX11;
		geSetAttributes(attributes);

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
	}

	void LoadContent()
	{
		//geSetFrameLock(60);
		if (!pixelMode.Initialize({ 640, 360 }))
		{
			geLogLastError();
		}

		if (!software3D.Initialize(pixelMode.videoBuffer, pixelMode.GetPixelFrameBufferSize(),16))
		{
			geLogLastError();
		}

		game::Random rnd;

		rnd.NewSeed();


		// Clockwise vertex winding
		// tl
		tri.vertices[0].x = 270;
		tri.vertices[0].y = 130;
		tri.color[0] = game::Colors::Red;

		// br
		tri.vertices[1].x = 370;
		tri.vertices[1].y = 230;
		tri.color[1] = game::Colors::Blue;

		// bl
		tri.vertices[2].x = 270;
		tri.vertices[2].y = 230;
		tri.color[2] = game::Colors::Green;


		//htri.vertices[0] = Projecttoh(tri.vertices[0]);
		//htri.vertices[1] = Projecttoh(tri.vertices[1]);
		//htri.vertices[2] = Projecttoh(tri.vertices[2]);

		//for (int i = 0; i < 3; i++)
		//{
		//	htri.vertices[i] = Projecttoh(tri.vertices[i]);
		//	std::cout << htri.vertices[i].x << "," << htri.vertices[i].y << "\n";
		//}


		// Clockwise vertex winding
		// tr
		tri2.vertices[0].x = 370;
		tri2.vertices[0].y = 130;
		tri2.color[0] = game::Colors::White;

		// br
		tri2.vertices[1].x = 370;
		tri2.vertices[1].y = 230;
		tri2.color[1] = game::Colors::Blue;

		// tl
		tri2.vertices[2].x = 270;
		tri2.vertices[2].y = 130;
		tri2.color[2] = game::Colors::Red;


		for (uint32_t i = 0; i < 1000; i++)
		{
			game::Triangle temp(tri);
			for (uint32_t v = 0; v < 3; v++)
			{
				temp.vertices[v].x = (float_t)rnd.RndRange(0, 640);
				temp.vertices[v].y = (float_t)rnd.RndRange(0, 360);
				temp.vertices[v].z = (float_t)rnd.RndRange(0, 360);
			}
			tris.emplace_back(temp);
		}
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

	void Rotate(float_t& x, float_t& y, const float_t theta)
	{
		float_t x_new = (x - 320) * cos(-theta) - (y - 180) * sin(-theta);
		float_t y_new = (x - 320) * sin(-theta) + (y - 180) * cos(-theta);
		x = x_new + 320.0f;
		y = y_new + 180.0f;
	}	

	void Rotateh(float_t& x, float_t& y, const float_t theta)
	{
		float_t x_new = (x) * cos(theta) - (y) * sin(theta);
		float_t y_new = (x) * sin(theta) + (y) * cos(theta);
		x = x_new;
		y = y_new;
	}

	// For clipping only need znear so a lot can be precalc for plane
	// make sure plane is normalized
	// precal 
	// plane normal dot plane point
	// store c

	game::Triangle Projecttoh(const game::Triangle vertex)
	{
		game::Triangle ret(vertex);
		for (int i = 0; i < 3; i++)
		{
			ret.vertices[i].x = (vertex.vertices[i].x * 2.0 / (float_t)pixelMode.GetPixelFrameBufferSize().x) - 1.0f;
			ret.vertices[i].y = (vertex.vertices[i].y * 2.0f / (float_t)pixelMode.GetPixelFrameBufferSize().y) -1.0f;
		}

		return ret;
	}

	game::Triangle Project(const game::Triangle vertex)
	{
		game::Triangle ret(vertex);
		for (int i = 0; i < 3; i++)
		{
			ret.vertices[i].x = ((vertex.vertices[i].x + 1.0f) * 0.5f * (float_t)pixelMode.GetPixelFrameBufferSize().x);// (vertex.x * 2.0 / (float_t)pixelMode.GetPixelFrameBufferSize().x) - 1.0f;
			ret.vertices[i].y = (((vertex.vertices[i].y + 1.0f) * 0.5f * (float_t)pixelMode.GetPixelFrameBufferSize().y));// 1.0f - (vertex.y * 2.0f / (float_t)pixelMode.GetPixelFrameBufferSize().y);
		}
		return ret;
	}

	void Render(const float_t msElapsed)
	{
		static float_t rotation = 0.0f;

		rotation += (2 * 3.14f / 10.0f) * (msElapsed / 1000.0f);
		geClear(GAME_FRAME_BUFFER_BIT, game::Colors::Blue);

		game::Triangle rotatedTri(tri);
		game::Triangle rotatedTri2(tri2);

		Rotate(rotatedTri.vertices[0].x, rotatedTri.vertices[0].y, rotation);
		Rotate(rotatedTri.vertices[1].x, rotatedTri.vertices[1].y, rotation);
		Rotate(rotatedTri.vertices[2].x, rotatedTri.vertices[2].y, rotation);

		Rotate(rotatedTri2.vertices[0].x, rotatedTri2.vertices[0].y, rotation);
		Rotate(rotatedTri2.vertices[1].x, rotatedTri2.vertices[1].y, rotation);
		Rotate(rotatedTri2.vertices[2].x, rotatedTri2.vertices[2].y, rotation);


		pixelMode.Clear(game::Colors::Black);

		std::vector<game::Triangle> quad;

		//quad.emplace_back(rotatedTri);
		quad.emplace_back(rotatedTri2);

		game::Triangle test;

		htri = Projecttoh(rotatedTri);
		Rotateh(test.vertices[0].x, test.vertices[0].y, rotation);
		Rotateh(test.vertices[1].x, test.vertices[1].y, rotation);
		Rotateh(test.vertices[2].x, test.vertices[2].y, rotation);

		test = Project(htri);
		quad.emplace_back(test);


		software3D.Render(quad);
		software3D.Fence(quad.size());
		//software3D.Render(tris);
		//software3D.Fence(tris.size());
		

		pixelMode.Text("FPS: " + std::to_string(geGetFramesPerSecond()), 0, 0, game::Colors::Yellow, 1);
		std::stringstream ss;
		ss << "Fill Mode: " << state;
		pixelMode.Text(ss.str(), 0, 10, game::Colors::Yellow, 1);
		pixelMode.Text("Working Threads: " + std::to_string(software3D.NumberOfThreads()), 0, 20, game::Colors::Yellow, 1);


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