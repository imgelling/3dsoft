#if !defined(GAMESOFTWARE3D_H)
#define GAMESOFTWARE3D_H

#include "GameMath.h"
#include "GameColor.h"
#include "GamePixelMode.h"

namespace game
{
	// GameSoftwareRenderer.h
	struct Triangle
	{
		game::Vector3f vertices[3];
		game::Vector3f clippedVertices[3];
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

	struct EdgeEquation {
		float_t a;
		float_t b;
		float_t c;
		bool fillRule;

		EdgeEquation(const game::Vector3f& v0, const game::Vector3f& v1)
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
		float_t a;
		float_t b;
		float_t c;

		ParameterEquation(
			const float_t p0,
			const float_t p1,
			const float_t p2,
			const EdgeEquation& e0,
			const EdgeEquation& e1,
			const EdgeEquation& e2,
			float_t area)
		{
			float_t factor = 1.0f / area;

			a = factor * (p0 * e0.a + p1 * e1.a + p2 * e2.a);
			b = factor * (p0 * e0.b + p1 * e1.b + p2 * e2.b);
			c = factor * (p0 * e0.c + p1 * e1.c + p2 * e2.c);
		}

		// Evaluate the parameter equation for the given point.
		float_t evaluate(const float_t x, const float_t y) const noexcept
		{
			return a * x + b * y + c;
		}
	};

	class Software3D
	{
	public:
		Software3D();
		bool Initialize(uint32_t* frameBuffer, const Pointi& size);
		const Recti TriangleBoundingBox(const Triangle& tri) noexcept;
		template<bool wireFrame, bool filled>
		void DrawColored(const Triangle& tri);
	private:
		uint32_t* _frameBuffer;
		int32_t _frameBufferWidth;
		int32_t _frameBufferHeight;
		std::atomic<uint32_t> fence;
	};

	Software3D::Software3D()
	{
		_frameBuffer = nullptr;
		fence = 0;
		_frameBufferWidth = 0;
		_frameBufferHeight = 0;
	}

	inline bool Software3D::Initialize(uint32_t* frameBuffer, const Pointi& size)
	{
		_frameBuffer = frameBuffer;
		fence = 0;
		_frameBufferWidth = size.width;
		_frameBufferHeight = size.height;
		return true;
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



	template<bool wireFrame, bool filled>
	inline void Software3D::DrawColored(const Triangle& tri)
	{
		game::Vector3f v0(tri.vertices[0].x, tri.vertices[0].y, 0.0f);
		game::Vector3f v1(tri.vertices[1].x, tri.vertices[1].y, 0.0f);
		game::Vector3f v2(tri.vertices[2].x, tri.vertices[2].y, 0.0f);

		bool foundTriangle(false);
		uint32_t videoBufferPos(0);
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
		if ((boundingBox.right < 0) || (boundingBox.x > _frameBufferWidth - 1) ||
			(boundingBox.bottom < 0) || (boundingBox.y > _frameBufferHeight - 1))
		{
			fence++;
			return;
		}

		// Partial offscreen
		if (boundingBox.x < 0)
			boundingBox.x = 0;
		if (boundingBox.right > _frameBufferWidth - 1)
			boundingBox.right = _frameBufferWidth - 1;
		if (boundingBox.y < 0)
			boundingBox.y = 0;
		if (boundingBox.bottom > _frameBufferHeight - 1)
			boundingBox.bottom = _frameBufferHeight - 1;

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
		uint32_t* buffer = _frameBuffer;
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
						++videoBufferPos;
						break;
					}
					else
					{
						//pixelMode.videoBuffer[videoBufferPos] = game::Colors::Magenta.packedARGB;
						++videoBufferPos;
						continue;
					}
				}
				if (e1.test(pixelOffset.x, pixelOffset.y))
				{
					if (foundTriangle)
					{
						++videoBufferPos;
						break;
					}
					else
					{
						//pixelMode.videoBuffer[videoBufferPos] = game::Colors::Magenta.packedARGB;
						++videoBufferPos;
						continue;
					}
				}
				if (e2.test(pixelOffset.x, pixelOffset.y))
				{
					if (foundTriangle)
					{
						++videoBufferPos;
						break;
					}
					else
					{
						//pixelMode.videoBuffer[videoBufferPos] = game::Colors::Magenta.packedARGB;
						++videoBufferPos;
						continue;
					}
				}
				foundTriangle = true;

				if (wireFrame)
				{
					auto distanceFromPointToLineSq = [&](const float_t x0, const float_t y0, const float_t yy, const float_t xx, const float_t xyyx, const float_t denominator)
						{
							float_t num = yy * x0 - xx * y0 + xyyx;
							float_t numerator = num * num;
							return (numerator * denominator);
						};
					// Wireframe
					for (uint32_t dist = 0; dist < 3; dist++)
					{
						d[dist] = distanceFromPointToLineSq(pixelOffset.x, pixelOffset.y, yy[dist], xx[dist], xy[dist], denominator[dist]);
					}
					minDistSq = d[0] < d[1] ? (d[0] < d[2] ? d[0] : d[2]) : (d[1] < d[2] ? d[1] : d[2]);
					if (minDistSq < 1)
					{
						buffer[videoBufferPos] = game::Colors::White.packedARGB;
						++videoBufferPos;
						continue;
					}
				}

				// Calculates the color
				if (filled)
				{
					game::Color color(r.evaluate(pixelOffset.x, pixelOffset.y), g.evaluate(pixelOffset.x, pixelOffset.y), b.evaluate(pixelOffset.x, pixelOffset.y), 1.0f);
					buffer[videoBufferPos] = color.packedARGB;
				}
				++videoBufferPos;
			}
			videoBufferPos += videoBufferStride - xLoopCount;
		}
		fence++;
	}

}

#endif