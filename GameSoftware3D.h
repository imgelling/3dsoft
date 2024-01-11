#if !defined(GAMESOFTWARE3D_H)
#define GAMESOFTWARE3D_H

#include <vector>
#include "GameMath.h"
#include "GameColor.h"
#include "GamePixelMode.h"
#include "GameSoftware3D_Data.h"
#include "GameThreadPool.h"

#define GAME_SOFTWARE3D_STATE_FILL_MODE 0
#define GAME_SOFTWARE3D_STATE_THREADED 1

namespace game
{

	class Software3D
	{
	public:
		Software3D();
		~Software3D();
		bool Initialize(uint32_t* frameBuffer, const Pointi& size, const int32_t threads);
		int32_t SetState(const uint32_t state, const int32_t value);
		void Fence(uint64_t fenceValue) noexcept;
		const Recti TriangleBoundingBox(const Triangle& tri) noexcept;
		void Render(std::vector<Triangle>& tris, const Recti& clip);
		template<bool wireFrame, bool filled>
		void DrawColored(const Triangle& tri, const Recti& clip);
		std::atomic<uint32_t> fence;
		uint32_t NumberOfThreads() { return _threadPool.NumberOfThreads(); }
		void ClearDepth();
	private:
		template<bool threaded>
		void _Render(std::vector<Triangle>& tris, const Recti& clip);
		bool _multiThreaded;
		ThreadPool _threadPool;
		uint32_t* _frameBuffer;
		int32_t _frameBufferWidth;
		int32_t _frameBufferHeight;
		float_t* _depth;
		HANDLE _fenceEvent;
		FillMode _FillMode;
	};

	Software3D::Software3D()
	{
		_frameBuffer = nullptr;
		_fenceEvent = nullptr;
		fence = 0;
		_frameBufferWidth = 0;
		_frameBufferHeight = 0;
		_multiThreaded = false;
		_depth = nullptr;
		_FillMode = FillMode::WireFrameFilled;
	}

	Software3D::~Software3D()
	{
		CloseHandle(_fenceEvent);
		_fenceEvent = nullptr;
		_threadPool.Stop();
		delete[] _depth;
		_depth = nullptr;
	}

	inline void Software3D::Fence(uint64_t fenceValue) noexcept
	{
		while (fence < fenceValue) {};
		fence = 0;
	}

	inline void Software3D::ClearDepth()
	{
		std::fill_n(_depth, _frameBufferWidth * _frameBufferHeight, 1000.0f);
	}

	inline int32_t Software3D::SetState(const uint32_t state, const int32_t value)
	{
		if (state == GAME_SOFTWARE3D_STATE_FILL_MODE)
		{
			if ((value < (int32_t)FillMode::WireFrame) || (value >= (int32_t)FillMode::None))
			{
				return false;
			}
			_FillMode = (FillMode)value;
			return true;
		}

		if (state == GAME_SOFTWARE3D_STATE_THREADED)
		{
			if (value >= 0)
			{
				_multiThreaded = true;
				_threadPool.Stop();
				_threadPool.Start(value);
			}
			else
			{
				_multiThreaded = false;
				_threadPool.Stop();
			}
		}

		return false;
	}

	inline bool Software3D::Initialize(uint32_t* frameBuffer, const Pointi& size, const int32_t threads = -1)
	{
		_frameBuffer = frameBuffer;
		fence = 0;
		_frameBufferWidth = size.width;
		_frameBufferHeight = size.height;
		if (threads < 0)
		{
			_multiThreaded = false;
		}
		else
		{
			_multiThreaded = true;
			_threadPool.Start(threads);
		}
		_depth = new float[size.width * size.height];
		return true;
	}

	inline void Software3D::Render(std::vector<Triangle>& tris, const Recti& clip)
	{
		if (_multiThreaded)
		{
			_Render<true>(tris,clip);
		}
		else
		{
			_Render<false>(tris,clip);
		}
	}

	template<bool threaded>
	inline void Software3D::_Render(std::vector<Triangle>& tris, const Recti& clip)
	{
		std::function<void(Triangle)> renderer;

		switch (_FillMode)
		{
		case game::FillMode::WireFrameFilled: renderer = std::bind(&Software3D::DrawColored<true, true>, this, std::placeholders::_1, clip); break;
		case game::FillMode::WireFrame: renderer = std::bind(&Software3D::DrawColored<true, false>, this, std::placeholders::_1, clip); break;
		case game::FillMode::FilledColor: renderer = std::bind(&Software3D::DrawColored<false, true>, this, std::placeholders::_1, clip); break;
		default: break;
		}

		for (uint32_t triangleCount = 0; triangleCount < tris.size(); ++triangleCount)
		{
			if (threaded)
			{
				_threadPool.Queue(std::bind(renderer,(tris[triangleCount])));
			}
			else
			{
				renderer(tris[triangleCount]);
			}
		}
	}

	inline const Recti Software3D::TriangleBoundingBox(const Triangle& tri) noexcept
	{
		Recti boundingBox;

		int32_t sx1 = (int32_t)(tri.vertices[0].x);// + 0.5f);
		int32_t sx2 = (int32_t)(tri.vertices[1].x);// + 0.5f);
		int32_t sx3 = (int32_t)(tri.vertices[2].x);// + 0.5f);
		int32_t sy1 = (int32_t)(tri.vertices[0].y);// + 0.5f);
		int32_t sy2 = (int32_t)(tri.vertices[1].y);// + 0.5f);
		int32_t sy3 = (int32_t)(tri.vertices[2].y);// + 0.5f);

		boundingBox.right = sx1 > sx2 ? (sx1 > sx3 ? sx1 : sx3) : (sx2 > sx3 ? sx2 : sx3);
		boundingBox.bottom = sy1 > sy2 ? (sy1 > sy3 ? sy1 : sy3) : (sy2 > sy3 ? sy2 : sy3);
		boundingBox.x = sx1 < sx2 ? (sx1 < sx3 ? sx1 : sx3) : (sx2 < sx3 ? sx2 : sx3);
		boundingBox.y = sy1 < sy2 ? (sy1 < sy3 ? sy1 : sy3) : (sy2 < sy3 ? sy2 : sy3);

		return boundingBox;
	}

	template<bool wireFrame, bool color>
	inline void Software3D::DrawColored(const Triangle& tri, const Recti& clip)
	{
		game::Vector3f v0(tri.vertices[0].x, tri.vertices[0].y, tri.vertices[0].z);
		game::Vector3f v1(tri.vertices[1].x, tri.vertices[1].y, tri.vertices[1].z);
		game::Vector3f v2(tri.vertices[2].x, tri.vertices[2].y, tri.vertices[2].z);

		bool foundTriangle(false);
		uint32_t videoBufferStride(_frameBufferWidth);

		EdgeEquation e0(v1, v2);
		EdgeEquation e1(v2, v0);
		EdgeEquation e2(v0, v1);

		// back face cull
		float area(e0.c + e1.c + e2.c);
		if (area < 0)
		{
			fence++;
			return;
		}

		game::Recti boundingBox = TriangleBoundingBox(tri);
		game::Vector2f pixelOffset;

		// Screen clipping
		// Offscreen completely
		if ((boundingBox.right < clip.x) || (boundingBox.x > clip.right) ||
			(boundingBox.bottom < clip.y) || (boundingBox.y > clip.bottom))
		{
			fence++;
			return;
		}

		// Partial offscreen
		if (boundingBox.x < clip.x)
			boundingBox.x = clip.x;
		if (boundingBox.right > clip.right)
			boundingBox.right = clip.right;
		if (boundingBox.y < clip.y)
			boundingBox.y = clip.y;
		if (boundingBox.bottom > clip.bottom)
			boundingBox.bottom = clip.bottom;

		// Color parameter
		Color colorAtPixel;
		ParameterEquation r(tri.color[0].rf/tri.vertices[0].w, tri.color[1].rf/tri.vertices[1].w, tri.color[2].rf/tri.vertices[2].w, e0, e1, e2, area);
		ParameterEquation g(tri.color[0].gf/tri.vertices[0].w, tri.color[1].gf/tri.vertices[1].w, tri.color[2].gf/tri.vertices[2].w, e0, e1, e2, area);
		ParameterEquation b(tri.color[0].bf/tri.vertices[0].w, tri.color[1].bf/tri.vertices[1].w, tri.color[2].bf/tri.vertices[2].w, e0, e1, e2, area);

		// zclip
		if ((tri.vertices[0].z < 0.1f) ||
			(tri.vertices[1].z < 0.1f) ||
			(tri.vertices[2].z < 0.1f))
		{
			fence++;
			return;
		}
		
		//// Depth test
		//float xd = 1.0f / tri.vertices[0].z;// / 2.0f;
		//float yd = 1.0f / tri.vertices[1].z;// / 2.0f;
		//float zd = 1.0f / tri.vertices[2].z;// / 2.0f;
		float xd = 1.0f / tri.vertices[0].w;// / 2.0f;
		float yd = 1.0f / tri.vertices[1].w;// / 2.0f;
		float zd = 1.0f / tri.vertices[2].w;// / 2.0f;
		ParameterEquation depth(xd, yd, zd, e0, e1, e2, area);

		// Wireframe precalcs
		float_t d[3] = {};
		float_t minDistSq(0.0f);
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
		uint32_t* buffer = _frameBuffer + (boundingBox.y * videoBufferStride + boundingBox.x);
		uint32_t xLoopCount = 0;
		// added the = last night below
		for (int32_t j = boundingBox.y; j <= boundingBox.bottom; ++j)
		{
			foundTriangle = false;
			xLoopCount = 0;
			for (int32_t i = boundingBox.x; i <= boundingBox.right; ++i)
			{
				++xLoopCount;
				pixelOffset = { i + 0.5f , j + 0.5f };

				if (e0.test(pixelOffset.x, pixelOffset.y))
				{
					if (foundTriangle)
					{
						++buffer;
						break;
					}
					else
					{
						//pixelMode.videoBuffer[videoBufferPos] = game::Colors::Magenta.packedARGB;
						++buffer;
						continue;
					}
				}
				if (e1.test(pixelOffset.x, pixelOffset.y))
				{
					if (foundTriangle)
					{
						++buffer;
						break;
					}
					else
					{
						//pixelMode.videoBuffer[videoBufferPos] = game::Colors::Magenta.packedARGB;
						++buffer;
						continue;
					}
				}
				if (e2.test(pixelOffset.x, pixelOffset.y))
				{
					if (foundTriangle)
					{
						++buffer;
						break;
					}
					else
					{
						//pixelMode.videoBuffer[videoBufferPos] = game::Colors::Magenta.packedARGB;
						++buffer;
						continue;
					}
				}
				foundTriangle = true;

				// depth buffer test
				float dd = depth.evaluate(pixelOffset.x, pixelOffset.y);
				//std::cout << dd << "\n";
				if (1.0f / dd <= _depth[j * _frameBufferWidth + i])
				{
					_depth[j * _frameBufferWidth + i] = 1.0f / dd;
				}
				else
				{
					++buffer;
					continue;
				}
				
				// Wireframe
				if (wireFrame)
				{
					auto distanceFromPointToLineSq = [&](const float_t x0, const float_t y0, const float_t yy, const float_t xx, const float_t xyyx, const float_t denominator)
						{
							float_t num = yy * x0 - xx * y0 + xyyx;
							float_t numerator = num * num;
							return (numerator * denominator);
						};

					for (uint32_t dist = 0; dist < 3; dist++)
					{
						d[dist] = distanceFromPointToLineSq(pixelOffset.x, pixelOffset.y, yy[dist], xx[dist], xy[dist], denominator[dist]);
					}
					minDistSq = d[0] < d[1] ? (d[0] < d[2] ? d[0] : d[2]) : (d[1] < d[2] ? d[1] : d[2]);
					if (minDistSq < 1)
					{
						*buffer = game::Colors::White.packedARGB;
						++buffer;
						continue;
					}
				}

				// Color filled
				if (color)
				{
					//colorAtPixel.Set(r.evaluate(pixelOffset.x, pixelOffset.y), g.evaluate(pixelOffset.x, pixelOffset.y), b.evaluate(pixelOffset.x, pixelOffset.y), 1.0f);
					// depth test
					float pre = dd;
					dd /= 2.5f;
					if (dd > 1.0f) dd = 1.0f;
					//dd = 1.0f - dd;
					float_t rd = min(r.evaluate(pixelOffset.x, pixelOffset.y) / pre, 1.0f) *dd;
					float_t gd = min(g.evaluate(pixelOffset.x, pixelOffset.y) / pre, 1.0f) *dd;
					float_t bd = min(b.evaluate(pixelOffset.x, pixelOffset.y) / pre, 1.0f) *dd;
					colorAtPixel.Set(rd, gd, bd, 1.0f);
					*buffer = colorAtPixel.packedARGB;
				}
				++buffer;
			}
			buffer += videoBufferStride - xLoopCount;
		}
		fence++;
	}

}

#endif