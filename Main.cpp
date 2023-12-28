#include "game.h"

struct Vertex
{
	float_t x, y, z;
	float_t r, g, b;
};

struct Triangle
{
	Vertex vertices[3];
	float_t averageZ;
};

class Game : public game::Engine
{

public:
	game::PixelMode pixelMode;
	Triangle tri;
	game::ThreadPool threadPool;
	std::atomic<uint64_t> fence;

	Game() : game::Engine()
	{
		ZeroMemory(&tri, sizeof(Triangle));
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

		// Start the threads up
		threadPool.Start();

		// Clockwise vertex winding
		// top
		tri.vertices[0].x = 320;
		tri.vertices[0].y = 130;
		tri.vertices[0].r = 1.0f;

		// right
		tri.vertices[1].x = 370;
		tri.vertices[1].y = 230;
		tri.vertices[1].b = 1.0f;

		// left
		tri.vertices[2].x = 270;
		tri.vertices[2].y = 230;
		tri.vertices[2].g = 1.0f;



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

	void Rotate(float& x, float& y, float theta) {
		float_t x_new = (x-320) * cos(theta) - (y-180) * sin(theta);
		float_t y_new = (x-320) * sin(theta) + (y - 180) * cos(theta);
		x = x_new + 320;
		y = y_new + 180;
	}

	void DrawWireFrame(const Triangle &tri, const game::Color &color)
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

	// Returns the signed area of the triangle formed by
	// points a,b,c with clockwise winding
	inline float_t edgeFunctionCW(const game::Vector2f& a, const game::Vector2f& b, const game::Vector2f& c)
	{
		return -((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x));
	}

	// Returns the signed area of the triangle formed by
	// points a,b,c with counter (anti) clockwise winding
	inline float_t edgeFunctionCCW(const game::Vector2f& a, const game::Vector2f& b, const game::Vector2f& c)
	{
		return ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x));
	}

	inline game::Rectf TriangleBoundingBox(const Triangle& tri)
	{
		game::Rectf boundingBox;

		float_t sx1 = tri.vertices[0].x;
		float_t sx2 = tri.vertices[1].x;
		float_t sx3 = tri.vertices[2].x;
		float_t sy1 = tri.vertices[0].y;
		float_t sy2 = tri.vertices[1].y;
		float_t sy3 = tri.vertices[2].y;

		boundingBox.right = sx1 > sx2 ? (sx1 > sx3 ? sx1 : sx3) : (sx2 > sx3 ? sx2 : sx3);
		boundingBox.bottom = sy1 > sy2 ? (sy1 > sy3 ? sy1 : sy3) : (sy2 > sy3 ? sy2 : sy3);
		boundingBox.x = sx1 < sx2 ? (sx1 < sx3 ? sx1 : sx3) : (sx2 < sx3 ? sx2 : sx3);
		boundingBox.y = sy1 < sy2 ? (sy1 < sy3 ? sy1 : sy3) : (sy2 < sy3 ? sy2 : sy3);

		return boundingBox;
	}

	void DrawColored(const Triangle& tri)
	{
		game::Vector2f v0(tri.vertices[0].x, tri.vertices[0].y);
		game::Vector2f v1(tri.vertices[1].x, tri.vertices[1].y);
		game::Vector2f v2(tri.vertices[2].x, tri.vertices[2].y);

		game::Rectf boundingBox = TriangleBoundingBox(tri);

		float_t area = edgeFunctionCW(v0, v1, v2);
		float_t oneOverArea = 1.0f / area;
		float_t w0(0.0f);
		float_t w1 = 0.0f;
		float_t w2 = 0.0f;
		float_t r = 0.0f;
		float_t g = 0.0f;
		float_t b = 0.0f;
		game::Vector2f p;


		for (int32_t j = (int32_t)boundingBox.y; j < (int32_t)boundingBox.bottom; ++j) {
			for (int32_t i = (int32_t)boundingBox.x; i < (int32_t)boundingBox.right; ++i) {
				p = { i + 0.5f, j + 0.5f };

				w0 = edgeFunctionCW(v1, v2, p);
				if (w0 <= 0) continue;  // >= for clockwise triangles  <= counter
				w1 = edgeFunctionCW(v2, v0, p);
				if (w1 <= 0) continue;
				w2 = edgeFunctionCW(v0, v1, p);
				if (w2 <= 0) continue;

				w0 *= oneOverArea;
				w1 *= oneOverArea;
				w2 *= oneOverArea;

				// Calculates the color
				r = w0 * tri.vertices[0].r + w1 * tri.vertices[1].r + w2 * tri.vertices[2].r;
				g = w0 * tri.vertices[0].g + w1 * tri.vertices[1].g + w2 * tri.vertices[2].g;
				b = w0 * tri.vertices[0].b + w1 * tri.vertices[1].b + w2 * tri.vertices[2].b;
				pixelMode.Pixel(i, j, game::Color(r, g, b, 1.0f));

			}
		}
		fence++;
	}

	/*
	* // Does it pass the top-left rule?
Vec2f v0 = { ... };
Vec2f v1 = { ... };
Vec2f v2 = { ... };

float w0 = edgeFunction(v1, v2, p); 
float w1 = edgeFunction(v2, v0, p); 
float w2 = edgeFunction(v0, v1, p); 

Vec2f edge0 = v2 - v1;
Vec2f edge1 = v0 - v2;
Vec2f edge2 = v1 - v0;

bool overlaps = true;

// If the point is on the edge, test if it is a top or left edge, 
// otherwise test if  the edge function is positive
overlaps &= (w0 == 0 ? ((edge0.y == 0 && edge0.x > 0) ||  edge0.y > 0) : (w0 > 0));
overlaps &= (w1 == 0 ? ((edge1.y == 0 && edge1.x > 0) ||  edge1.y > 0) : (w1 > 0));
overlaps &= (w1 == 0 ? ((edge2.y == 0 && edge2.x > 0) ||  edge2.y > 0) : (w2 > 0));

if (overlaps) {
    // pixel overlap the triangle
    ...draw it
}
	*/

	void Render(const float_t msElapsed)
	{
		static float rotation = 0.0f;

		rotation += (2 * 3.14f / 10.0f) * (msElapsed / 1000.0f);
		geClear(GAME_FRAME_BUFFER_BIT, game::Colors::Blue);

		fence = 0;
		Triangle rotatedTri;
		//ZeroMemory(&rotatedTri, sizeof(triangle));
		rotatedTri = tri;

		Rotate(rotatedTri.vertices[0].x, rotatedTri.vertices[0].y, rotation);
		Rotate(rotatedTri.vertices[1].x, rotatedTri.vertices[1].y, rotation);
		Rotate(rotatedTri.vertices[2].x, rotatedTri.vertices[2].y, rotation);


		pixelMode.Clear(game::Colors::Black);

		//threadPool.Queue(std::bind(&Game::DrawWireFrame, this, rotatedTri, game::Colors::Red));
		//threadPool.Queue(std::bind(&Game::DrawWireFrame, this, tri, game::Colors::White));

		//DrawWireFrame(rotatedTri, game::Colors::Red);
		//DrawWireFrame(tri, game::Colors::White);
		
		threadPool.Queue(std::bind(&Game::DrawColored, this, std::ref(rotatedTri)));
		//threadPool.Queue(std::bind(&Game::DrawColored, this, std::ref(tri)));
		while(fence < 1)
		{ }


		DrawWireFrame(rotatedTri, game::Colors::White);
		//DrawColored(rotatedTri);

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