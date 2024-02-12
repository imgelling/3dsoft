#if !defined(GAMESOFTWARE3D_DATA_H)
#define GAMESOFTWARE3D_DATA_H

#include "GameMath.h"
#include "GameColor.h"

namespace game
{
	enum FillMode
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

	class EdgeEquation 
	{
	public:
		float_t a;
		float_t b;
		float_t c;
		bool fillRule;
		EdgeEquation() { a=(0.0f); b=(0.0f); c=(0.0f); fillRule=(false); }
		EdgeEquation(const game::Vector3f& v0, const game::Vector3f& v1)
		{
			a = v0.y - v1.y;
			b = v1.x - v0.x;
			c = -(a * (v0.x + v1.x) + b * (v0.y + v1.y)) / 2;
			fillRule = a != 0 ? a > 0 : b > 0;
		}

		inline void Set(const game::Vector3f& v0, const game::Vector3f& v1)
		{
			a = v0.y - v1.y;
			b = v1.x - v0.x;
			c = -(a * (v0.x + v1.x) + b * (v0.y + v1.y)) / 2;
			fillRule = a != 0 ? a > 0 : b > 0;
		}

		// Evaluate the edge equation for the given point.
		inline float_t evaluate(const float_t x, const float_t y) const noexcept
		{
			return a * x + b * y + c;
		}

		// Test if the given point is inside the edge.
		bool test(const float_t x, const float_t y) const noexcept
		{
			return test(evaluate(x, y));
		}

		// Test for a given evaluated value.
		inline bool test(const float_t v) const noexcept
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

	struct Triangle
	{
		Vector3f vertices[3];
		Color color[3];
		Vector3f faceNormal;
		Vector3f normals[3];
		Vector2f uvs[3];

		// Pre calc in clip
		EdgeEquation edge0;
		EdgeEquation edge1;
		EdgeEquation edge2;

		// Clipping data
		bool backFaceCulled = false;
		bool edgeCalculated = false;
		bool boundingCalculated = false;
		Recti boundingBox;
		float_t area = 0.0;
	};

	struct Texture
	{
		uint32_t* data = nullptr;
		Pointi size;
		Pointf oneOverSize;
	};

	struct RenderTarget
	{
		uint32_t* colorBuffer = nullptr;
		float_t* depthBuffer = nullptr;
		Pointi size;
		Pointf halfSize;
		uint32_t totalBufferSize = 0;
	};

	struct Mesh
	{
		std::vector<Triangle> tris;
		Matrix4x4f model;
		Matrix4x4f translate;
		Matrix4x4f rotation;
		Matrix4x4f scale;
		inline void SetTranslation(const float_t x, const float_t y, const float_t z) noexcept
		{
			translate.SetTranslation(x, y, z);
		}
		inline void SetScale(const float_t x, const float_t y, const float_t z) noexcept
		{
			scale.SetScale(x, y, z);
		}
		inline void SetRotation(const float_t x, const float_t y, const float_t z) noexcept
		{
			Matrix4x4f rotX;
			Matrix4x4f rotY;
			Matrix4x4f rotZ;
			if (x)
			{
				rotX.SetRotationX(x);
			}
			if (y)
			{
				rotY.SetRotationY(y);
			}
			if (z)
			{
				rotZ.SetRotationZ(z);
			}
			rotation = rotZ * rotY * rotX;
		}
		inline Matrix4x4f CreateModelMatrix() noexcept
		{
			model.SetIdentity();
			return model = translate * rotation * scale;
		}
	};

	struct ParameterEquation 
	{
		float_t a = 0.0f;
		float_t b = 0.0f;
		float_t c = 0.0f;

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
		inline float_t evaluate(const float_t x, const float_t y) const noexcept
		{
			return a * x + b * y + c;
		}
	};

	struct Projection {
		float_t a = 0.0f;
		float_t b = 0.0f;
		float_t c = 0.0f;
		float_t d = 0.0f;
		float_t e = 0.0f;
	};
}


#endif