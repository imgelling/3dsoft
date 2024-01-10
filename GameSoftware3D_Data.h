#if !defined(GAMESOFTWARE3D_DATA_H)
#define GAMESOFTWARE3D_DATA_H

#include "GameMath.h"
#include "GameColor.h"

namespace game
{
	struct Triangle
	{
		game::Vector3f vertices[3];
		//game::Vector3f clippedVertices[3];
		game::Color color[3];
		//game::Vector3f faceNormal;
		//game::Vector3f normals[3];
		//game::Vector2f uvs[3];
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
	static FillMode& operator++ (FillMode& rmode, int32_t)
	{
		rmode = static_cast<FillMode>((int)rmode + 1);
		if (rmode == FillMode::None) rmode = static_cast<FillMode>(0);
		return rmode;
	}
	static std::ostream& operator<< (std::ostream& stream, FillMode mode)
	{
		switch (mode)
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
}


#endif