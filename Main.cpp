#include <stdint.h>
#include "game.h"

struct vertex
{
	float x, y, z;
};

struct triangle
{
	vertex vertices[3];
};

class Game : public game::Engine
{

public:
	game::PixelMode pixelMode;
	triangle tri;
	game::ThreadPool threadPool;
	std::atomic<uint64_t> fence;

	Game() : game::Engine()
	{
		ZeroMemory(&tri, sizeof(triangle));
		fence = 0;
	}

	void Initialize()
	{
		game::Attributes attributes;

		attributes.WindowTitle = "Window Title";
		attributes.VsyncOn = false;
		geSetAttributes(attributes);

	}

	void LoadContent()
	{
		if (!pixelMode.Initialize({ 640,360 }))
		{
			geLogLastError();
		}

		threadPool.Start();


		// top
		tri.vertices[0].x = 320;
		tri.vertices[0].y = 130;

		// left
		tri.vertices[1].x = 270;
		tri.vertices[1].y = 230;

		// right
		tri.vertices[2].x = 370;
		tri.vertices[2].y = 230;

	}

	void Shutdown()
	{
		threadPool.Stop();
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
	}

	void rotate(float& x, float& y, float theta) {
		float x_new = (x-320) * cos(theta) - (y-180) * sin(theta);
		float y_new = (x-320) * sin(theta) + (y - 180) * cos(theta);
		x = x_new + 320;
		y = y_new + 180;
	}

	void DrawWireFrame(triangle tri, game::Color color)
	{
		pixelMode.LineClip(
			(int32_t)tri.vertices[0].x, (int32_t)tri.vertices[0].y,
			(int32_t)tri.vertices[1].x, (int32_t)tri.vertices[1].y,
			color);
		pixelMode.LineClip(
			(int32_t)tri.vertices[1].x, (int32_t)tri.vertices[1].y,
			(int32_t)tri.vertices[2].x, (int32_t)tri.vertices[2].y,
			color);
		pixelMode.LineClip(
			(int32_t)tri.vertices[2].x, (int32_t)tri.vertices[2].y,
			(int32_t)tri.vertices[0].x, (int32_t)tri.vertices[0].y,
			color);
		fence++;
	}

	void Render(const float_t msElapsed)
	{
		static float rotation = 0.0f;

		//rotation += (2 * 3.14f / 10.0f) * (msElapsed / 1000.0f);
		geClear(GAME_FRAME_BUFFER_BIT, game::Colors::Blue);

		triangle rotatedTri;
		ZeroMemory(&rotatedTri, sizeof(triangle));
		rotatedTri = tri;

		rotate(rotatedTri.vertices[0].x, rotatedTri.vertices[0].y, rotation);
		rotate(rotatedTri.vertices[1].x, rotatedTri.vertices[1].y, rotation);
		rotate(rotatedTri.vertices[2].x, rotatedTri.vertices[2].y, rotation);


		pixelMode.Clear(game::Colors::Black);

		threadPool.Queue(std::bind(&Game::DrawWireFrame, this, rotatedTri, game::Colors::Red));
		threadPool.Queue(std::bind(&Game::DrawWireFrame, this, tri, game::Colors::White));

		//DrawWireFrame(rotatedTri, game::Colors::Red);
		//DrawWireFrame(tri, game::Colors::White);
		while(fence < 2)
		{ }
		fence = 0;

		//pixelMode.LineClip(
		//	(int32_t)tri.vertices[0].x, (int32_t)tri.vertices[0].y,
		//	(int32_t)tri.vertices[1].x, (int32_t)tri.vertices[1].y,
		//	game::Colors::White);
		//pixelMode.LineClip(
		//	(int32_t)tri.vertices[1].x, (int32_t)tri.vertices[1].y,
		//	(int32_t)tri.vertices[2].x, (int32_t)tri.vertices[2].y,
		//	game::Colors::White);
		//pixelMode.LineClip(
		//	(int32_t)tri.vertices[2].x, (int32_t)tri.vertices[2].y,
		//	(int32_t)tri.vertices[0].x, (int32_t)tri.vertices[0].y,
		//	game::Colors::White);

		pixelMode.TextClip("FPS: " + std::to_string(geGetFramesPerSecond()), 0, 0, game::Colors::White, 2);


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