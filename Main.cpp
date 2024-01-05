#define GAME_SUPPORT_DIRECTX11
#include "game.h"


// GameSoftwareRenderer.h
struct Triangle
{
	game::Vector3f vertices[3];
	game::Color color[3];
	game::Vector3f faceNormal;
	game::Vector3f normals[3];
	game::Vector2f uvs[3];
};

enum class FillMode
{
	WireFrame,
	FilledColor,
	WireFrameFilled,
	//AffineTextureMapped,
	//WireFrameAffTexture,
	//ProjectionTextureMapped,
	//WireFrameProjTexture,
	None
};
//FillMode& operator++ (RasterMode& rmode, int);
//std::ostream& operator<< (std::ostream& stm, FillMode rmode);
static FillMode& operator++ (FillMode& rmode, int)
{
	rmode = static_cast<FillMode>((int)rmode + 1);
	if (rmode == FillMode::None) rmode = static_cast<FillMode>(0);
	return rmode;
}
static std::ostream& operator<< (std::ostream& stream, FillMode rmode)
{
	switch (rmode)
	{
	case FillMode::WireFrame: return stream << "WireFrame";
	case FillMode::FilledColor: return stream << "Filled Color";
	case FillMode::WireFrameFilled: return stream << "WireFrame Filled";
	//case FillMode::AffineTextureMapped: return stream << "Affine Texture Mapped";
	//case FillMode::WireFrameAffTexture: return stream << "WireFrame Affine Texture Mapped";
	//case FillMode::ProjectionTextureMapped: return stream << "Projection Correct Texture Mapped";
	//case FillMode::WireFrameProjTexture: return stream << "WireFrame Projection Correct Texture Mapped";
	default: return stream << "Unknown Enumerator";
	}
}

// end GameSoftwareRenderer.h

class Game : public game::Engine
{

public:
	game::PixelMode pixelMode;
	Triangle tri;
	game::ThreadPool threadPool;
	std::atomic<uint64_t> fence;
	//size_t fence;

	game::Recti clip[4];
	std::vector<Triangle> tris;
	std::vector<Triangle> clippedTris;

	FillMode state = FillMode::WireFrameFilled;

//#include "Header.h"

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
		attributes.RenderingAPI = game::RenderAPI::DirectX11;
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
		//geSetFrameLock(60);
		if (!pixelMode.Initialize({ 640, 360 }))
		{
			geLogLastError();
		}
		game::Random rnd;

		rnd.NewSeed();

		// Start the threads up
		threadPool.Start();

		// Clockwise vertex winding
		// top
		tri.vertices[0].x = 320;
		tri.vertices[0].y = 130;
		tri.color[0] = game::Colors::Red;

		// right
		tri.vertices[1].x = 370;
		tri.vertices[1].y = 230;
		tri.color[1] = game::Colors::Blue;

		// left
		tri.vertices[2].x = 270;
		tri.vertices[2].y = 230;
		tri.color[2] = game::Colors::Green;

		for (uint32_t i = 0; i < 1000; i++)
		{
			Triangle temp(tri);
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

		if (geKeyboard.WasKeyPressed(geK_W))
		{
			state++;
		}
	}

	void Rotate(float_t& x, float_t& y, const float_t theta)
	{
		float_t x_new = (x - 320) * cos(-theta) - (y - 180) * sin(-theta);
		float_t y_new = (x - 320) * sin(-theta) + (y - 180) * cos(-theta);
		x = x_new + 320.0f;
		y = y_new + 180.0f;
	}

	inline const game::Recti TriangleBoundingBox(const Triangle& tri) const noexcept
	{
		game::Recti boundingBox;

		int32_t sx1 = (int32_t)(tri.vertices[0].x + 0.5f);
		int32_t sx2 = (int32_t)(tri.vertices[1].x + 0.5f);
		int32_t sx3 = (int32_t)(tri.vertices[2].x + 0.5f);
		int32_t sy1 = (int32_t)(tri.vertices[0].y + 0.5f);
		int32_t sy2 = (int32_t)(tri.vertices[1].y + 0.5f);
		int32_t sy3 = (int32_t)(tri.vertices[2].y + 0.5f);

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

		// Evaluate the edge equation for the given point.
		float_t evaluate(const float_t x, const float_t y) const noexcept
		{
			return a * x + b * y + c;
		}

		// Test if the given point is inside the edge.
		bool test(const float_t x, const float_t y) const noexcept
		{
			return test(evaluate(x, y));
		}

		// Test for a given evaluated value.
		bool test(const float_t v) const noexcept
		{
			return (v < 0 || v == 0 && fillRule);
		}

		// Step the equation value v to the x direction.
		float_t stepX(const float_t v) const noexcept
		{
			return v + a;
		}

		// Step the equation value v to the x direction.
		float_t stepX(const float_t v, const float_t stepSize) const noexcept
		{
			return v + a * stepSize;
		}

		// Step the equation value v to the y direction.
		float_t stepY(const float_t v) const noexcept
		{
			return v + b;
		}

		// Step the equation value vto the y direction.
		float_t stepY(const float_t v, const float_t stepSize) const noexcept
		{
			return v + b * stepSize;
		}
	};

	struct ParameterEquation {
		float a;
		float b;
		float c;

		ParameterEquation(
			const float_t p0,
			const float_t p1,
			const float_t p2,
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

		// Evaluate the parameter equation for the given point.
		float evaluate(const float_t x, const float_t y) const noexcept
		{
			return a * x + b * y + c;
		}
	};

	template<bool wireFrame, bool filled>
	void DrawColored(const Triangle& tri)
	{
		game::Vector2f v0(tri.vertices[0].x, tri.vertices[0].y);
		game::Vector2f v1(tri.vertices[1].x, tri.vertices[1].y);
		game::Vector2f v2(tri.vertices[2].x, tri.vertices[2].y);

		bool foundTriangle(false);
		uint32_t videoBufferPos(0);
		uint32_t videoBufferStride(pixelMode.GetPixelFrameBufferSize().x);


		fence++;

		EdgeEquation e0(v1, v2);
		EdgeEquation e1(v2, v0);
		EdgeEquation e2(v0, v1);

		// back face cull
		float area(e0.c + e1.c + e2.c);
		if (area < 0)
		{
			return;
		}

		game::Recti boundingBox = TriangleBoundingBox(tri);
		game::Vector2f pixelOffset;

		// Screen clipping
		// Offscreen completely
		if (boundingBox.right < 0) return;
		if (boundingBox.x > pixelMode.GetPixelFrameBufferSize().width - 1) return;
		if (boundingBox.bottom < 0) return;
		if (boundingBox.y > pixelMode.GetPixelFrameBufferSize().height - 1) return;

		// Partial offscreen
		if (boundingBox.x < 0)
			boundingBox.x = 0;
		if (boundingBox.right > pixelMode.GetPixelFrameBufferSize().width - 1) 
			boundingBox.right = pixelMode.GetPixelFrameBufferSize().width - 1;
		if (boundingBox.y < 0) 
			boundingBox.y = 0;
		if (boundingBox.bottom > pixelMode.GetPixelFrameBufferSize().height - 1) 
			boundingBox.bottom = pixelMode.GetPixelFrameBufferSize().height - 1;

		// Color parameter
		ParameterEquation r(tri.color[0].rf, tri.color[1].rf, tri.color[2].rf, e0, e1, e2, area);
		ParameterEquation g(tri.color[0].gf, tri.color[1].gf, tri.color[2].gf, e0, e1, e2, area);
		ParameterEquation b(tri.color[0].bf, tri.color[1].bf, tri.color[2].bf, e0, e1, e2, area);

		// Wireframe precalcs
		float_t d[3] = {};
		float_t minDistSq = 0.0f;
		float_t yy[3] = {}; //y2 - y1;
		float_t xx[3] = {}; //x2 - x1;
		float_t xy[3] = {}; //x2 * y1 then xy - yx
		float_t yx[3] = {}; //y2 * x1;
		float_t denominator[3] = {};	//1.0f / (xx * xx + yy * yy);
		if (wireFrame)
		{
			yy[0] = tri.vertices[1].y - tri.vertices[0].y;
			xx[0] = tri.vertices[1].x - tri.vertices[0].x;
			xy[0] = tri.vertices[1].x * tri.vertices[0].y;
			yx[0] = tri.vertices[1].y * tri.vertices[0].x;

			yy[1] = tri.vertices[2].y - tri.vertices[1].y;
			xx[1] = tri.vertices[2].x - tri.vertices[1].x;
			xy[1] = tri.vertices[2].x * tri.vertices[1].y;
			yx[1] = tri.vertices[2].y * tri.vertices[1].x;

			yy[2] = tri.vertices[0].y - tri.vertices[2].y;
			xx[2] = tri.vertices[0].x - tri.vertices[2].x;
			xy[2] = tri.vertices[0].x * tri.vertices[2].y;
			yx[2] = tri.vertices[0].y * tri.vertices[2].x;

			xy[0] = xy[0] - yx[0];
			xy[1] = xy[1] - yx[1];
			xy[2] = xy[2] - yx[2];

			denominator[0] = 1.0f / (xx[0] * xx[0] + yy[0] * yy[0]);
			denominator[1] = 1.0f / (xx[1] * xx[1] + yy[1] * yy[1]);
			denominator[2] = 1.0f / (xx[2] * xx[2] + yy[2] * yy[2]);
		}

		// jitters with floats, maybe a pixel with floats for subpixel? It is slower 
		// with just floats no subpixel
		videoBufferPos = (boundingBox.y * videoBufferStride + boundingBox.x);
		uint32_t xLoopCount = 0;
		// added the = last night below
		for (int32_t j = boundingBox.y; j <= boundingBox.bottom; ++j)
		{
			foundTriangle = false;
			xLoopCount = 0;
			for (int32_t i = boundingBox.x; i <= boundingBox.right; ++i)
			{
				xLoopCount++;
				pixelOffset = { i + 0.5f , j + 0.5f };
				
				if (e0.test(pixelOffset.x, pixelOffset.y))
				{					
					if (foundTriangle)
					{
						videoBufferPos++;
						break;
					}
					else
					{
						//pixelMode.Pixel((int)i, (int32_t)j, game::Colors::Pink);
						//pixelMode.videoBuffer[videoBufferPos] = game::Colors::Magenta.packedARGB;
						videoBufferPos++;
						continue;
					}
				}
				if (e1.test(pixelOffset.x, pixelOffset.y))
				{
					if (foundTriangle)
					{
						videoBufferPos++;
						break;
					}
					else
					{
						//pixelMode.Pixel((int)i, (int32_t)j, game::Colors::Pink);
						//pixelMode.videoBuffer[videoBufferPos] = game::Colors::Magenta.packedARGB;
						videoBufferPos++;
						continue;
					}
				}
				if (e2.test(pixelOffset.x, pixelOffset.y))
				{
					if (foundTriangle)
					{
						videoBufferPos++;
						break;
					}
					else
					{
						//pixelMode.Pixel((int)i, (int32_t)j, game::Colors::Pink);
						//pixelMode.videoBuffer[videoBufferPos] = game::Colors::Magenta.packedARGB;
						videoBufferPos++;
						continue;
					}
				}
				foundTriangle = true;

				if (wireFrame)
				{
					// Wireframe
					for (uint32_t dist = 0; dist < 3; dist++)
					{
						d[dist] = distanceFromPointToLineSq(pixelOffset.x, pixelOffset.y, yy[dist], xx[dist], xy[dist], denominator[dist]);
					}
					minDistSq = d[0] < d[1] ? (d[0] < d[2] ? d[0] : d[2]) : (d[1] < d[2] ? d[1] : d[2]);
					if (minDistSq < 4)
					{
						//pixelMode.Pixel(i, j, game::Colors::White);
						pixelMode.videoBuffer[videoBufferPos] = game::Colors::White.packedARGB;
						videoBufferPos++;
						continue;
					}
				}

				// Calculates the color
				if (filled)
				{
					//pixelMode.Pixel(i, j, game::Color(r.evaluate(pixelOffset.x, pixelOffset.y), g.evaluate(pixelOffset.x, pixelOffset.y), b.evaluate(pixelOffset.x, pixelOffset.y), 1.0f));
					game::Color color(r.evaluate(pixelOffset.x, pixelOffset.y), g.evaluate(pixelOffset.x, pixelOffset.y), b.evaluate(pixelOffset.x, pixelOffset.y), 1.0f);
					pixelMode.videoBuffer[videoBufferPos] = color.packedARGB;
				}
				videoBufferPos++;
			}
			videoBufferPos += videoBufferStride - xLoopCount;
		}
	}

	inline float_t distanceFromPointToLineSq(const float_t x0, const float_t y0, const float_t yy, const float_t xx, const float_t xyyx, const float_t denominator)
	{
		float_t num = yy * x0 - xx * y0 + xyyx;
		float_t numerator = num * num;
		return (numerator * denominator);
	}

	void Render(const float_t msElapsed)
	{
		static float_t rotation = 0.0f;

		rotation += (2 * 3.14f / 10.0f) * (msElapsed / 1000.0f);
		geClear(GAME_FRAME_BUFFER_BIT, game::Colors::Blue);

		fence = 0;
		Triangle rotatedTri(tri);

		Rotate(rotatedTri.vertices[0].x, rotatedTri.vertices[0].y, rotation);
		Rotate(rotatedTri.vertices[1].x, rotatedTri.vertices[1].y, rotation);
		Rotate(rotatedTri.vertices[2].x, rotatedTri.vertices[2].y, rotation);


		pixelMode.Clear(game::Colors::Black);
		//threadPool.Queue(std::bind(&Game::DrawColored<false,true>, this, std::ref(tri)));
		//DrawWireFrame(tri, game::Colors::White);

		//for (int s = 0; s < tris.size(); s++)
		//{
		//	threadPool.Queue(std::bind(&Game::DrawColored<true,true>, this, std::ref(tris[s])));
		//}

		//while(fence < tris.size())
		//{
		//	//std::cout << fence << "  != " << tris.size()*4 -1 << "\n";
		//}
		//Game::DrawColored<w, true>(rotatedTri);
		switch (state)
		{
		case FillMode::WireFrameFilled: DrawColored<true, true>(rotatedTri); break;
		case FillMode::WireFrame: DrawColored<true, false>(rotatedTri); break;
		case FillMode::FilledColor: DrawColored<false, true>(rotatedTri); break;
		default: break;
		}
		//DrawColored<w,true>(rotatedTri);
		//DrawColored<false,true>(rotatedTri);
		//DrawColoredBlock(rotatedTri);

		pixelMode.TextClip("FPS: " + std::to_string(geGetFramesPerSecond()), 0, 0, game::Colors::Yellow, 1);
		std::stringstream ss;
		ss << "Fill Mode: " << state;
		pixelMode.TextClip(ss.str(), 0, 10, game::Colors::Yellow, 1);


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