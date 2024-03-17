#if !defined(GAMESOFTWARE3D_DATA_H)
#define GAMESOFTWARE3D_DATA_H

#include "GameMath.h"
#include "GameColor.h"

namespace game
{
	enum FillMode
	{
		WireFrame,
		Filled,
		WireFrameFilled,
		//AffineTextureMapped,
		//WireFrameAffTexture,
		//ProjectionTextureMapped,
		//WireFrameProjTexture,
		None
	};

	enum LightingType
	{
		Face,
		Vertex,
		Depth
	};

	enum SortingType
	{
		FrontToBack,
		BackToFront,
		NoSort
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
		case FillMode::Filled: return stream << "Filled";
		case FillMode::WireFrameFilled: return stream << "WireFrame Filled";
			//case FillMode::AffineTextureMapped: return stream << "Affine Texture Mapped";
			//case FillMode::WireFrameAffTexture: return stream << "WireFrame Affine Texture Mapped";
			//case FillMode::ProjectionTextureMapped: return stream << "Projection Correct Texture Mapped";
			//case FillMode::WireFrameProjTexture: return stream << "WireFrame Projection Correct Texture Mapped";
		default: return stream << "Unknown fill type";
		}
	}
	static std::ostream& operator<< (std::ostream& stream, LightingType type)
	{
		switch (type)
		{
		case LightingType::Face: return stream << "Face Lighting";
		case LightingType::Vertex: return stream << "Vertex Lighting";
		case LightingType::Depth: return stream << "Depth Lighting";
		default: return stream << "Unknown lighting type";
		}
	}
	static std::ostream& operator<< (std::ostream& stream, SortingType type)
	{
		switch (type)
		{
		case SortingType::BackToFront: return stream << "Back to Front";
		case SortingType::FrontToBack: return stream << "Front to Back";
		case SortingType::NoSort: return stream << "None";
		default: return stream << "Unknown lighting type";
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
			c = -(a * (v0.x + v1.x) + b * (v0.y + v1.y)) * 0.5f;// / 2;
			fillRule = a != 0 ? a > 0 : b > 0;
		}

		inline void Set(const game::Vector3f& v0, const game::Vector3f& v1)
		{
			a = v0.y - v1.y;
			b = v1.x - v0.x;
			c = -(a * (v0.x + v1.x) + b * (v0.y + v1.y)) * 0.5f;// / 2;
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
		//Pointf oneOverSize;
	};

	struct RenderTarget
	{
		uint32_t* colorBuffer = nullptr;
		float_t* depthBuffer = nullptr;
		Pointi size;
		Pointi halfSize;
		uint32_t totalBufferSize = 0;
		Matrix4x4f projection;
	};

	struct Mesh
	{
		std::vector<Triangle> tris;
		Texture texture;
		Matrix4x4f model;
		Matrix4x4f translate;
		Matrix4x4f rotation;
		Matrix4x4f scale;
		Vector3f centerPoint;
		Matrix4x4f billboard;
		Vector3f position;
		bool SetTexture(const Texture& inTexture) noexcept
		{
			if (texture.data == nullptr)
			{
				return false;
			}
			texture = inTexture;
			return true;
		}
		bool SetTexture(const RenderTarget& target) noexcept
		{
			if (target.colorBuffer == nullptr)
			{
				return false;
			}
			if (target.depthBuffer == nullptr)
			{
				return false;
			}
			if (!target.size.width || !target.size.height)
			{
				return false;
			}
			texture.data = target.colorBuffer;
			texture.size.width = target.size.width;
			texture.size.height = target.size.height;
			//texture.oneOverSize.width = 1.0f / (float_t)target.size.width;
			//texture.oneOverSize.height = 1.0f / (float_t)target.size.height;
			return true;
		}
		inline void SetTranslation(const float_t x, const float_t y, const float_t z) noexcept
		{
			translate.SetTranslation(x, y, z);
			//position = { x,y,z };
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
		inline void GenerateModelMatrix() noexcept
		{
			model.SetIdentity();
			model = translate * rotation * scale;
		}
		inline void GenerateBillboardMatrix(Camera3D& camera) noexcept
		{
			Vector3f look;// = camera.position - position;
			look.x = -(camera.position.x - position.x);
			look.y = (camera.position.y - position.y);
			look.z = (camera.position.z - position.z);
			
			look.Normalize();

			Vector3f right = camera.up.Cross(look);
			right.Normalize();
			Vector3f up = look.Cross(right);
			up.Normalize();

			billboard.SetIdentity();
			billboard.m[0] = right.x;
			billboard.m[4] = right.y;
			billboard.m[8] = right.z;

			billboard.m[1] = 0.0f;// up.x; // locks y rotation
			billboard.m[5] = 1.0f;// up.y;
			billboard.m[9] = 0.0f;// up.z;

			billboard.m[2] =  look.x;
			billboard.m[6] =  look.y;
			billboard.m[10] = look.z;

			billboard.m[12] = (position.x);
			billboard.m[13] = (position.y);
			billboard.m[14] = (position.z);
			

			////billboard.m[15] = 1.0f;
			//Matrix4x4f rotY;
			//rotY.SetRotationY(3.14159f/2.0f);
			//billboard = billboard * translate;
			//SetTranslation(0, 0, 0);
			//billboard = billboard *rotY;
		}

		inline void GenerateBillboardMatrix2(const Camera3D& camera) noexcept
		{
			//Matrix4x4f x;
			//Matrix4x4f y;
			//Matrix4x4f z;
			//Matrix4x4f t;
			billboard.SetIdentity();
			//x.SetRotationX(-camera.rotation.x);
			//y.SetRotationY(camera.rotation.y -3.14159f / 2.0f);
			//z.SetRotationZ(camera.rotation.z);
			//SetTranslation(position.x, position.y, position.z);
			billboard.m[0] = camera.view.m[0];//right.x;
			billboard.m[4] = camera.view.m[1];//right.y;
			billboard.m[8] = camera.view.m[2];//right.z;

			billboard.m[1] = camera.view.m[4];//up.x;
			billboard.m[5] = camera.view.m[5];//up.y;
			billboard.m[9] = camera.view.m[6];//up.z;

			billboard.m[2] = camera.view.m[8];//look.x;
			billboard.m[6] = camera.view.m[9];//look.y;
			billboard.m[10] = camera.view.m[10];//look.z;
			//billboard.m[12] = position.x;// camera.view.m[3];
			//billboard.m[13] = position.y;// camera.view.m[7];
			//billboard.m[14] = position.z;// camera.view.m[11];
			//position = { 0 ,0,0};
			//GenerateModelMatrix();
			//billboard = billboard * translate;
			//SetTranslation(0, 0, 0);
			//billboard = billboard * z * y * x;// *translate;
		}

		inline void GenerateQuad(const Color &color, const Pointf size) noexcept
		{
			Triangle topLeftTri;
			Triangle bottomRightTri;
// ---------- ALL OF THE FOLLOWING CAN BE PRECALC -------------------------------
			//float_t size = 0.5f;  // actually half size
			float_t z = 0;// position.z;
			//position = { 0,0,position.z };
			// tl
			topLeftTri.vertices[0].x = -size.width;// +position.x;
			topLeftTri.vertices[0].y = -size.height;// +position.y;
			topLeftTri.vertices[0].z = z;
			topLeftTri.color[0] = color;
			topLeftTri.uvs[0].u = 0.0f;
			topLeftTri.uvs[0].v = 0.0f;
			topLeftTri.faceNormal.x = 0.0f;
			topLeftTri.faceNormal.y = 0.0f;
			topLeftTri.faceNormal.z = -1.0f;


			// tr
			topLeftTri.vertices[1].x = size.width;// +position.x;
			topLeftTri.vertices[1].y = -size.height;// +position.y;
			topLeftTri.vertices[1].z = z;
			topLeftTri.uvs[1].u = 1.0f;
			topLeftTri.uvs[1].v = 0.0f;
			topLeftTri.color[1] = color;

			// bl
			topLeftTri.vertices[2].x = -size.width;// +position.x;
			topLeftTri.vertices[2].y = size.height;// +position.y;
			topLeftTri.vertices[2].z = z;
			topLeftTri.uvs[2].u = 0.0f;
			topLeftTri.uvs[2].v = 1.0f;
			topLeftTri.color[2] = color;

			// tr
			bottomRightTri.vertices[0].x = size.width;// +position.x;
			bottomRightTri.vertices[0].y = -size.height;// +position.y;
			bottomRightTri.vertices[0].z = z;
			bottomRightTri.color[0] = color;
			bottomRightTri.uvs[0].u = 1.0f;
			bottomRightTri.uvs[0].v = 0.0f;
			bottomRightTri.faceNormal.x = 0.0f;
			bottomRightTri.faceNormal.y = 0.0f;
			bottomRightTri.faceNormal.z = -1.0f;

			// br
			bottomRightTri.vertices[1].x = size.width;// +position.x;
			bottomRightTri.vertices[1].y = size.height;// +position.y;
			bottomRightTri.vertices[1].z = z;
			bottomRightTri.uvs[1].u = 1.0f;
			bottomRightTri.uvs[1].v = 1.0f;
			bottomRightTri.color[1] = color;

			// bl
			bottomRightTri.vertices[2].x = -size.width;// +position.x;
			bottomRightTri.vertices[2].y = size.height;// +position.y;
			bottomRightTri.vertices[2].z = z;
			bottomRightTri.uvs[2].u = 0.0f;
			bottomRightTri.uvs[2].v = 1.0f;
			bottomRightTri.color[2] = color;

			for (uint32_t i = 0; i < 3; i++)
			{
				topLeftTri.normals[i] = { 0.0f,0.0f,-1.0f };
				bottomRightTri.normals[i] = { 0.0f,0.0f,-1.0f };
// ---------------------------------------------------------------------------
				topLeftTri.vertices[i] = topLeftTri.vertices[i] * rotation * billboard;
				bottomRightTri.vertices[i] = bottomRightTri.vertices[i] * rotation * billboard;
			}
			rotation.SetIdentity();
			tris.emplace_back(topLeftTri);
			tris.emplace_back(bottomRightTri);

		}
	};

	struct ParameterEquation 
	{
		float_t a = 0.0f;
		float_t b = 0.0f;
		float_t c = 0.0f;
		uint32_t first = 1;

		//float_t axstep = 0.0f;
		//float_t bystep = 0.0f;

		ParameterEquation(
			const float_t p0,
			const float_t p1,
			const float_t p2,
			const EdgeEquation& e0,
			const EdgeEquation& e1,
			const EdgeEquation& e2,
			const float_t area)
		{

			a = area * (p0 * e0.a + p1 * e1.a + p2 * e2.a);
			b = area * (p0 * e0.b + p1 * e1.b + p2 * e2.b);
			c = area * (p0 * e0.c + p1 * e1.c + p2 * e2.c);
		}

		ParameterEquation()
		{
		}

		void Set(const float_t p0,
			const float_t p1,
			const float_t p2,
			const EdgeEquation& e0,
			const EdgeEquation& e1,
			const EdgeEquation& e2,
			const float_t area)
		{
			a = area * (p0 * e0.a + p1 * e1.a + p2 * e2.a);
			b = area * (p0 * e0.b + p1 * e1.b + p2 * e2.b);
			c = area * (p0 * e0.c + p1 * e1.c + p2 * e2.c);
		}

		// Evaluate the parameter equation for the given point.
		inline void evaluate(const float_t x, const float_t y, float &eval) noexcept
		{
			first = 0;
			eval = a * x + b * y + c;
		}

		inline void stepX(float_t& v) const noexcept
		{
			v = v + a;
		}
	};

	struct Projection {
		float_t a = 0.0f;
		float_t b = 0.0f;
		float_t c = 0.0f;
		float_t d = 0.0f;
		float_t e = 0.0f;
	};

	class ClippingRects
	{
	public:
		uint32_t numberOfClipRects;
		game::Recti* clips;
		std::vector<std::vector<game::Triangle>> clippedTris;
		ClippingRects() noexcept;
		ClippingRects(const uint32_t inNumberOfClipRects);
		~ClippingRects();

		void SetNumberOfClipsRects(const uint32_t inNumberOfClipRects);
		bool GenerateClips(const game::Pointi& size);
	};

	inline ClippingRects::ClippingRects() noexcept
	{
		clips = nullptr;
		numberOfClipRects = 0;
	}

	inline ClippingRects::ClippingRects(const uint32_t inNumberOfClipRects)
	{
		numberOfClipRects = inNumberOfClipRects;
		clips = new game::Recti[numberOfClipRects];
		for (uint32_t clippedTri = 0; clippedTri < numberOfClipRects; clippedTri++)
		{
			std::vector<game::Triangle> newClippedTri;
			clippedTris.emplace_back(newClippedTri);
		}
	}

	inline ClippingRects::~ClippingRects()
	{
		if (clips != nullptr)
		{
			delete[] clips;
			clips = nullptr;
		}
		numberOfClipRects = 0;
	}

	inline void ClippingRects::SetNumberOfClipsRects(const uint32_t inNumberOfClipRects)
	{
		numberOfClipRects = inNumberOfClipRects;
		clippedTris.clear();
		if (clips != nullptr)
		{
			delete[] clips;
		}
		clips = new game::Recti[numberOfClipRects];
		for (uint32_t clippedTri = 0; clippedTri < numberOfClipRects; clippedTri++)
		{
			std::vector<game::Triangle> newClippedTri;
			clippedTris.emplace_back(newClippedTri);
		}
	}

	inline bool ClippingRects::GenerateClips(const game::Pointi& size)
	{
		if ((!size.width) || (!size.height)) return false;

		uint32_t cols = (uint32_t)sqrt(numberOfClipRects);//(int32_t)std::ceil(sqrt(numberOfClips));
		uint32_t rows = (uint32_t)(numberOfClipRects / (float_t)cols);//(int32_t)std::ceil(numberOfClips / (float_t)cols);

		uint32_t colsize = (uint32_t)std::ceil(size.width / (float_t)cols);
		uint32_t rowsize = (uint32_t)std::ceil(size.height / (float_t)rows);

		uint32_t rc = 0;
		uint32_t cc = 0;
		game::Recti* clips2 = clips;
		for (uint32_t row = 0; row < rows; row++)
		{
			rc = 0;
			for (uint32_t col = 0; col < cols; col++)
			{
				uint32_t access = row * cols + col;
				clips2[access].left = (rc) * (colsize);
				clips2[access].right = (clips2[access].left + colsize) - 1;
				if (clips2[access].right > size.width - 1) clips2[access].right = size.width - 1;
				clips2[access].top = cc * (rowsize);
				clips2[access].bottom = clips2[access].top + rowsize - 1;
				if (clips2[access].bottom > size.height - 1) clips2[access].bottom = size.height - 1;
				rc++;
			}
			cc++;
		}
		return true;
	}
}




#endif