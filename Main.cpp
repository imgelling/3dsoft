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

	game::Recti clip[4];
	std::vector<Triangle> tris;
	std::vector<Triangle> clippedTris;

#include "Header.h"

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

		//tl
		clip[0].x = 0;
		clip[0].y = 0;
		clip[0].right = 640 / 2 - 1;
		clip[0].bottom = 360 / 2 - 1;

		// tr
		clip[1].x = 640 / 2 - 1;
		clip[1].y = 0;
		clip[1].right = 640 - 1;
		clip[1].bottom = 360 / 2 - 1;

		// bl
		clip[2].x = 0;
		clip[2].y = 360 / 2 - 1;
		clip[2].right = 640 / 2 - 1;
		clip[2].bottom = 360 - 1;

		// br
		clip[3].x = 640 / 2 -1;
		clip[3].y = 360 / 2 - 1;
		clip[3].right = 640 - 1;
		clip[3].bottom = 360 - 1;
		

	}

	void LoadContent()
	{
		if (!pixelMode.Initialize({ 640,360 }))
		{
			geLogLastError();
		}
		game::Random rnd;

		rnd.NewSeed();

		// Start the threads up
		threadPool.Start(4);

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

		for (int i = 0; i < 1000; i++)
		{
			Triangle temp(tri);
			for (int v = 0; v < 3; v++)
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
		float_t x_new = (x-320) * cos(-theta) - (y - 180) * sin(-theta);
		float_t y_new = (x-320) * sin(-theta) + (y - 180) * cos(-theta);
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

	inline const game::Rectf TriangleBoundingBox(const Triangle& tri)
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

	struct EdgeEquation {
		float_t a;
		float_t b;
		float_t c;
		bool fillRule;

		EdgeEquation(const game::Vector2f& v0, const game::Vector2f& v1)
		{
			a = v0.y - v1.y;
			b = v1.x - v0.x;
			c = -(a * (v0.x + v1.x) + b * (v0.y + v1.y)) / 2;
			fillRule = a != 0 ? a > 0 : b > 0;
		}

		/// Evaluate the edge equation for the given point.
		float evaluate(float x, float y) const
		{
			return a * x + b * y + c;
		}

		/// Test if the given point is inside the edge.
		bool test(float x, float y) const
		{
			return test(evaluate(x, y));
		}

		/// Test for a given evaluated value.
		bool test(float v) const
		{
			return (v < 0 || v == 0 && fillRule);
		}

		/// Step the equation value v to the x direction.
		float stepX(float v) const
		{
			return v + a;
		}

		/// Step the equation value v to the x direction.
		float stepX(float v, float stepSize) const
		{
			return v + a * stepSize;
		}

		/// Step the equation value v to the y direction.
		float stepY(float v) const
		{
			return v + b;
		}

		/// Step the equation value vto the y direction.
		float stepY(float v, float stepSize) const
		{
			return v + b * stepSize;
		}
	};

	struct ParameterEquation {
		float a;
		float b;
		float c;

		ParameterEquation(
			float p0,
			float p1,
			float p2,
			const EdgeEquation& e0,
			const EdgeEquation& e1,
			const EdgeEquation& e2,
			float area)
		{
			float factor = 1.0f / area;

			a = factor * (p0 * e0.a + p1 * e1.a + p2 * e2.a);
			b = factor * (p0 * e0.b + p1 * e1.b + p2 * e2.b);
			c = factor * (p0 * e0.c + p1 * e1.c + p2 * e2.c);
		}

		/// Evaluate the parameter equation for the given point.
		float evaluate(float x, float y) const
		{
			return a * x + b * y + c;
		}
	};

	void DrawColored(const Triangle& tri)
	{
		game::Vector2f v0(tri.vertices[0].x, tri.vertices[0].y);
		game::Vector2f v1(tri.vertices[1].x, tri.vertices[1].y);
		game::Vector2f v2(tri.vertices[2].x, tri.vertices[2].y);

		// test optimization
		bool foundTriangle = false;

		EdgeEquation e0(v1, v2);
		EdgeEquation e1(v2, v0);
		EdgeEquation e2(v0, v1);

		// back face cull
		float area = (e0.c + e1.c + e2.c);
		if (area < 0)
		{
			fence++;
			return;
		}

		float a0 = 0.0f;
		float a1 = 0.0f;
		float a2 = 0.0f;
		game::Rectf boundingBox = TriangleBoundingBox(tri);
		game::Vector2f p;

		ParameterEquation r(tri.vertices[0].r, tri.vertices[1].r, tri.vertices[2].r, e0, e1, e2, area);
		ParameterEquation g(tri.vertices[0].g, tri.vertices[1].g, tri.vertices[2].g, e0, e1, e2, area);
		ParameterEquation b(tri.vertices[0].b, tri.vertices[1].b, tri.vertices[2].b, e0, e1, e2, area);

		// Wireframe precalcs
		float_t d[3] = {}; 
		float_t minDistSq = 0.0f;
		float_t yy[3] = {};
		float_t xx[3] = {};
		float_t xy[3] = {};
		float_t yx[3] = {};
		yy[0] = tri.vertices[1].y - tri.vertices[0].y; //y2 - y1;
		xx[0] = tri.vertices[1].x - tri.vertices[0].x; //x2 - x1;
		xy[0] = tri.vertices[1].x * tri.vertices[0].y; //x2 * y1;
		yx[0] = tri.vertices[1].y * tri.vertices[0].x; //y2 * x1;

		yy[1] = tri.vertices[2].y - tri.vertices[1].y; //y2 - y1;
		xx[1] = tri.vertices[2].x - tri.vertices[1].x; //x2 - x1;
		xy[1] = tri.vertices[2].x * tri.vertices[1].y; //x2 * y1;
		yx[1] = tri.vertices[2].y * tri.vertices[1].x; //y2 * x1;

		yy[2] = tri.vertices[0].y - tri.vertices[2].y; //y2 - y1;
		xx[2] = tri.vertices[0].x - tri.vertices[2].x; //x2 - x1;
		xy[2] = tri.vertices[0].x * tri.vertices[2].y; //x2 * y1;
		yx[2] = tri.vertices[0].y * tri.vertices[2].x; //y2 * x1;

		xy[0] = xy[0] - yx[0];
		xy[1] = xy[1] - yx[1];
		xy[2] = xy[2] - yx[2];

		for (int32_t j = (int32_t)boundingBox.y; j < (int32_t)boundingBox.bottom; ++j)
		{
			foundTriangle = false;
			for (int32_t i = (int32_t)boundingBox.x; i < (int32_t)boundingBox.right; ++i)
			{
				p = { i + 0.5f , j + 0.5f };

				a0 = e0.evaluate(p.x, p.y);
				if (e0.test(a0))  // >= for clockwise triangles  <= counter
				{
					if (foundTriangle)
					{
						break;
					}
					else
					{
						//pixelMode.Pixel((int)i, (int32_t)j, game::Colors::Pink);
						continue;
					}
				}
				a1 = e1.evaluate(p.x, p.y);
				if (e1.test(a1))
				{
					if (foundTriangle)
					{
						break;
					}
					else
					{
						//pixelMode.Pixel((int32_t)i, (int32_t)j, game::Colors::Pink);
						continue;
					}
				}
				a2 = e2.evaluate(p.x, p.y);
				if (e2.test(a2))
				{
					if (foundTriangle)
					{
						break;
					}
					else
					{
						//pixelMode.Pixel((int32_t)i, (int32_t)j, game::Colors::Pink);
						continue;
					}
				}
				foundTriangle = true;

				// Wireframe
				for (uint32_t dis = 0; dis < 3; dis++)
				{
					d[dis] = distanceFromPointToLineSq(p.x, p.y, yy[dis], xx[dis], xy[dis]);
				}
				minDistSq = d[0];
				if (d[1] < minDistSq) minDistSq = d[1];
				if (d[2] < minDistSq) minDistSq = d[2];
				if (minDistSq < 4)
				{
					pixelMode.Pixel(i, j, game::Colors::White);
					continue;
				}

				// Calculates the color
				pixelMode.Pixel(i, j, game::Color(r.evaluate(p.x, p.y), g.evaluate(p.x, p.y), b.evaluate(p.x, p.y), 1.0f));
			}
		}
		fence++;
	}

	float distanceFromPointToLineSq(const float x0, const float y0, const float yy, const float xx, const float xyyx)
	{
		float num = yy * x0 - xx * y0 + xyyx;
		float numerator = num * num;
		float denominator = xx * xx + yy * yy;
		return (numerator / denominator);
	}

	void ClipTriangle(Triangle tri)
	{

	}

	void Render(const float_t msElapsed)
	{
		static float rotation = 0.0f;

		rotation += (2 * 3.14f / 10.0f) * (msElapsed / 1000.0f);
		geClear(GAME_FRAME_BUFFER_BIT, game::Colors::Blue);

		fence = 0;
		Triangle rotatedTri(tri);
		//ZeroMemory(&rotatedTri, sizeof(triangle));
		//rotatedTri = tri;

		//rotation = 3.14f / 3.0f;
		Rotate(rotatedTri.vertices[0].x, rotatedTri.vertices[0].y, rotation);
		Rotate(rotatedTri.vertices[1].x, rotatedTri.vertices[1].y, rotation);
		Rotate(rotatedTri.vertices[2].x, rotatedTri.vertices[2].y, rotation);


		pixelMode.Clear(game::Colors::Black);

		//threadPool.Queue(std::bind(&Game::DrawWireFrame, this, rotatedTri, game::Colors::Red));
		//threadPool.Queue(std::bind(&Game::DrawWireFrame, this, tri, game::Colors::White));

		//DrawWireFrame(rotatedTri, game::Colors::Red);
		//DrawWireFrame(tri, game::Colors::White);

		//for (int s = 0; s < tris.size(); s++)
		//{
		//	for (int t = 0; t < 4; t++)
		//	{
		//		threadPool.Queue(std::bind(&Game::DrawColored, this, std::ref(tris[s])));
		//	}
		//}
	
		////threadPool.Queue(std::bind(&Game::DrawColored, this, std::ref(tri)));
		//while(fence < tris.size() * 4)
		//{
		//	//std::cout << fence << "  != " << tris.size()*4 -1 << "\n";
		//}
		rotation = 0;
		DrawColored(rotatedTri);

		//for (int i = 0; i < 4; i++)
		//DrawColored(rotatedTri,clip[i]);
		//pixelMode.RectClip(clip[1], game::Colors::White);
		//DrawWireFrame(rotatedTri, game::Colors::White);

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