#if !defined(GAMESOFTWARE3D_H)
#define GAMESOFTWARE3D_H

#include <vector>
#include "GameMath.h"
#include "GameColor.h"
#include "GamePixelMode.h"
#include "GameSoftware3D_Camera3D.h"
#include "GameSoftware3D_Data.h"
#include "GameSoftware3D_Math.h"
#include "GameThreadPool.h"

#define GAME_SOFTWARE3D_STATE_FILL_MODE 0
#define GAME_SOFTWARE3D_STATE_THREADED 1
#define GAME_SOFTWARE3D_WIREFRAME_THICKNESS 2
#define GAME_SOFTWARE3D_TEXTURE 3
#define GAME_SOFTWARE3D_LIGHTING 4

namespace game
{

	class Software3D
	{
	public:
		Software3D();
		~Software3D();
		bool Initialize(PixelMode& pixelMode, const int32_t threads);
		int32_t SetState(const uint32_t state, const int32_t value) noexcept;
		bool SetTexture(const Texture& texture) noexcept;
		bool SetTexture(const RenderTarget& target) noexcept;
		bool SetDefaultTexture() noexcept;
		void Fence(const uint64_t fenceValue) noexcept;
		const Recti TriangleBoundingBox(const Triangle& tri) const noexcept;
		void Render(const std::vector<Triangle>& tris, const Recti& clip) noexcept;
		template<bool wireFrame, bool filled, bool lighting, bool textured>
		void DrawColored(const Triangle& tri, const Recti& clip) noexcept;
		uint32_t NumberOfThreads() const noexcept { return _threadPool.NumberOfThreads(); }
		void ClearDepth(const float_t depth);
		void ClearRenderTarget(const Color& color, const float_t depth);
		void ScreenClip(std::vector<Triangle>& in, ClippingRects& clip, const uint32_t index) const noexcept;
		bool CreateTexture(const uint32_t width, const uint32_t height, Texture& texture) noexcept;
		bool CreateTexture(const Pointi size, Texture& texture) noexcept;
		void DeleteTexture(Texture& texture) noexcept;
		bool CreateRenderTarget(const uint32_t width, const uint32_t height, RenderTarget& target) noexcept;
		bool CreateRenderTarget(const Pointi size, RenderTarget& target) noexcept;
		void DeleteRenderTarget(RenderTarget& target) noexcept;
		bool SetRenderTarget(const RenderTarget& target) noexcept;
		void SetRenderTargetDefault() noexcept;

		void VertexProcessor(game::Mesh& mesh, const game::Matrix4x4f& mvp, std::vector<game::Triangle>& processedTris, Camera3D& camera) const noexcept;
		void RenderMesh(Mesh& mesh, Matrix4x4f& projection, Camera3D& camera, ClippingRects& clip);
		std::vector<game::Triangle> trianglesToRender;
		float_t* depthBuffer;
	private:
		RenderTarget _currentRenderTarget;
		float_t* _clearDepthBuffer[10];
		void _Render(const std::vector<Triangle>& tris, const Recti& clip) noexcept;
		void _GenerateDefaultTexture(uint32_t* buff, const uint32_t w, const uint32_t h);
		std::atomic<uint32_t> _fence;
		bool _multiThreaded;
		bool _usingRenderTarget;
		RenderTarget _defaultRenderTarget;
		Texture _defaultTexture;
		uint32_t _numbuffers;
		uint32_t _currentDepth;
		Texture _currentTexture;
		ThreadPool _threadPool;
		FillMode _fillMode;
		bool _enableTexturing;
		bool _enableLighting;
		PixelMode* _pixelMode;
		uint32_t _totalBufferSize;
	};

	Software3D::Software3D()
	{
		_defaultRenderTarget = {};
		_usingRenderTarget = false;
		_pixelMode = nullptr;
		_fence = 0;
		_currentDepth = 0;
		_totalBufferSize = 0;
		_multiThreaded = false;
		depthBuffer = nullptr;
		_enableTexturing = false;
		_enableLighting = false;
		_numbuffers = 5;
		for (uint32_t i = 0; i < 10; i++)
			_clearDepthBuffer[i] = nullptr;
		_fillMode = FillMode::WireFrameFilled;
		CreateTexture(64, 64, _defaultTexture);
		_GenerateDefaultTexture(_defaultTexture.data, 64, 64);
		SetTexture(_defaultTexture);
		trianglesToRender.reserve(1000);
	}

	Software3D::~Software3D()
	{
		_threadPool.Stop();
		for (uint32_t i = 0; i < _numbuffers; i++)
		{
			if (_clearDepthBuffer[i] != nullptr)
			{
				delete[] _clearDepthBuffer[i];
				_clearDepthBuffer[i] = nullptr;
			}
		}
		DeleteTexture(_defaultTexture);
		depthBuffer = nullptr;
	}



	inline bool Software3D::SetTexture(const Texture& texture) noexcept
	{
		if (texture.data == nullptr)
		{
			SetDefaultTexture(); 
			return false;
		}
		_currentTexture = texture;
		return true;
	}

	inline bool Software3D::SetTexture(const RenderTarget& target) noexcept
	{
		if (target.colorBuffer == nullptr)
		{
			SetDefaultTexture();
			return false;
		}
		if (target.depthBuffer == nullptr)
		{
			SetDefaultTexture();
			return false;
		}
		if (!target.size.width || !target.size.height)
		{
			SetDefaultTexture();
			return false;
		}
		Texture temp;
		temp.data = target.colorBuffer;
		temp.size.width = target.size.width;
		temp.size.height = target.size.height;
		//temp.oneOverSize.width = 1.0f / (float_t)target.size.width;
		//temp.oneOverSize.height = 1.0f / (float_t)target.size.height;
		_currentTexture = temp;
		return true;
	}

	inline bool Software3D::SetDefaultTexture() noexcept
	{
		_currentTexture = _defaultTexture;
		return true;
	}

	inline bool Software3D::CreateTexture(const uint32_t width, const uint32_t height, Texture& texture) noexcept
	{
		if (texture.data != nullptr)
		{
			return false;
		}
		if (!width || !height)
		{
			return false;
		}
		texture.data = new uint32_t[width * height];
		if (texture.data == nullptr)
		{
			return false;
		}
		texture.size.width = width;
		texture.size.height = height;
		//texture.oneOverSize.width = 1.0f / width;
		//texture.oneOverSize.height = 1.0f / height;
		return true;
	}

	inline bool Software3D::CreateTexture(const Pointi size, Texture& texture) noexcept
	{
		return CreateTexture(size.width, size.height, texture);
	}

	inline void Software3D::DeleteTexture(Texture& texture) noexcept
	{
		if (texture.data)
		{
			delete[] texture.data;
			texture.data = nullptr;
		}
		texture.size.width = 0;
		texture.size.height = 0;
		//texture.oneOverSize.width = 0;
		//texture.oneOverSize.height = 0;		
	}

	inline void Software3D::_GenerateDefaultTexture(uint32_t* buff, const uint32_t w, const uint32_t h)
	{
		game::Color col1 = game::Colors::Yellow;
		game::Color col2 = game::Colors::Magenta;
		for (uint32_t y = 0; y < h; y++)
		{
			if (y % 2 == 0)
				std::swap(col1, col2);
			for (uint32_t x = 0; x < w; x++)
			{
				if (x % 2 == 0)
					std::swap(col1, col2);
				buff[y * w + x] = col1.packedABGR;
			}
		}
	}

	inline bool Software3D::CreateRenderTarget(const uint32_t width, const uint32_t height, RenderTarget& target) noexcept
	{
		if (target.colorBuffer != nullptr)
		{
			return false;
		}
		if (target.depthBuffer != nullptr)
		{
			return false;
		}
		if (!width || !height)
		{
			return false;
		}
		target.colorBuffer = new uint32_t[width * height];
		if (target.colorBuffer == nullptr)
		{
			return false;
		}
		target.depthBuffer = new float_t[width * height];
		if (target.depthBuffer == nullptr)
		{
			DeleteRenderTarget(target);
			return false;
		}
		target.size.width = width;
		target.size.height = height;
		target.totalBufferSize = width * height;
		target.halfSize.width = width >> 1;
		target.halfSize.height = height >> 1;
		my_PerspectiveFOV2(90.0f, target.size.width / (float_t)target.size.height, 0.1f, 100.0f, target.projection);
		return true;
	}

	inline bool Software3D::SetRenderTarget(const RenderTarget& target) noexcept
	{
		if (!target.colorBuffer || !target.depthBuffer || !target.size.width || !target.size.height)
		{
			return false;
		}
		_usingRenderTarget = true;
		_currentRenderTarget = target;
		return true;
	}

	inline void Software3D::SetRenderTargetDefault() noexcept
	{
		_usingRenderTarget = false;
		_currentRenderTarget = _defaultRenderTarget;
		_currentRenderTarget.colorBuffer = _pixelMode->videoBuffer;// = _defaultRenderTarget;
		_currentRenderTarget.depthBuffer = _clearDepthBuffer[_currentDepth];
		depthBuffer = _clearDepthBuffer[_currentDepth];
	}

	inline bool Software3D::CreateRenderTarget(const Pointi size, RenderTarget& target) noexcept
	{
		CreateRenderTarget(size.width, size.height, target);
	}

	inline void Software3D::DeleteRenderTarget(RenderTarget& target) noexcept
	{
		if (target.colorBuffer)
		{
			delete[] target.colorBuffer;
			target.colorBuffer = nullptr;
		}
		if (target.depthBuffer)
		{
			delete[] target.depthBuffer;
			target.depthBuffer = nullptr;
		}
		target.size.width = 0;
		target.size.height = 0;
	}

	inline void Software3D::Fence(const uint64_t fenceValue) noexcept
	{
		while (_fence < fenceValue) {  };
		_fence = 0;
	}

	inline void Software3D::ClearDepth(const float_t depth)
	{
		if (_multiThreaded)
		{
			_threadPool.Queue(std::bind(std::fill_n<float_t*, uint32_t, float>, _clearDepthBuffer[_currentDepth], _totalBufferSize, depth));
			_currentDepth++;
			if (_currentDepth > _numbuffers - 1) _currentDepth = 0;
			_currentRenderTarget.depthBuffer = _clearDepthBuffer[_currentDepth];
			depthBuffer = _clearDepthBuffer[_currentDepth];
		}
		else
		{
			std::fill_n(_currentRenderTarget.depthBuffer, _totalBufferSize, depth);
			depthBuffer = _clearDepthBuffer[_currentDepth];
		}
		//}
		//else
		//{
		//	std::fill_n(_currentRenderTarget.depthBuffer, _currentRenderTarget.size.width * _currentRenderTarget.size.height, depth);
		//}
		//std::fill(depthBuffer, depthBuffer + _totalBufferSize, depth);
		//FillMemory(depthBuffer, 255, _totalBufferSize*4);
		//memcpy(depthBuffer, clearDepthBuffer, _totalBufferSize * 4);
		//MoveMemory(depthBuffer, clearDepthBuffer, _totalBufferSize * 4);
	}

	inline void Software3D::ClearRenderTarget(const Color& color, const float_t depth)
	{
		std::fill_n(_currentRenderTarget.depthBuffer, _currentRenderTarget.totalBufferSize, depth);
		std::fill_n(_currentRenderTarget.colorBuffer, _currentRenderTarget.totalBufferSize, color.packedABGR);
	}

	inline int32_t Software3D::SetState(const uint32_t state, const int32_t value) noexcept
	{
		if (state == GAME_SOFTWARE3D_STATE_FILL_MODE)
		{
			if ((value < (int32_t)FillMode::WireFrame) || (value >= (int32_t)FillMode::None))
			{
				return false;
			}
			_fillMode = (FillMode)value;
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
			return true;
		}

		return false;
	}

	inline bool Software3D::Initialize(PixelMode& pixelMode, const int32_t threads = -1)
	{
		_pixelMode = &pixelMode;
		_defaultRenderTarget.colorBuffer = _pixelMode->videoBuffer;
		_defaultRenderTarget.size = _pixelMode->GetPixelFrameBufferSize();
		_defaultRenderTarget.halfSize.width = _defaultRenderTarget.size.width >> 1;
		_defaultRenderTarget.halfSize.height = _defaultRenderTarget.size.height >> 1;
		_totalBufferSize = _defaultRenderTarget.size.width * _defaultRenderTarget.size.height;
		if (threads < 0)
		{
			_multiThreaded = false;
		}
		else
		{
			_multiThreaded = true;
			_threadPool.Start(threads);
		}
	
		for (uint32_t i = 0; i < _numbuffers; i++)
		{
			_clearDepthBuffer[i] = new float_t[_totalBufferSize];
			std::fill_n(_clearDepthBuffer[i], _totalBufferSize, 100.0f);
		}
		_currentDepth = 0;
		_defaultRenderTarget.depthBuffer = _clearDepthBuffer[_currentDepth];
		_currentRenderTarget = _defaultRenderTarget;
		return true;
	}

	inline void Software3D::RenderMesh(Mesh& mesh, Matrix4x4f& projection, Camera3D& camera, ClippingRects& clip)
	{
		VertexProcessor(mesh, projection, trianglesToRender, camera);
		SetTexture(mesh.texture);
		uint64_t fenceCount = 0;
		for (uint32_t c = 0; c < clip.numberOfClipRects; c++)
		{
			ScreenClip(trianglesToRender, clip, c);
			if (!clip.clippedTris[c].size()) continue;
			std::sort(clip.clippedTris[c].begin(), clip.clippedTris[c].end(), [](const game::Triangle& a, const game::Triangle& b)
				{
					float_t az = a.vertices[0].z + a.vertices[1].z + a.vertices[2].z;
					float_t bz = b.vertices[0].z + b.vertices[1].z + b.vertices[2].z;
					return az < bz;
				});
			//pixelMode.Rect(clip[c], game::Colors::Yellow);
			Render(clip.clippedTris[c], clip.clips[c]);
			fenceCount++;
		}
		Fence(fenceCount);
		trianglesToRender.clear();
	}

	inline void Software3D::Render(const std::vector<Triangle>& tris, const Recti& clip) noexcept
	{
		if (!_usingRenderTarget)
		{
			_currentRenderTarget.colorBuffer = _pixelMode->videoBuffer;
		}
		if (_multiThreaded)
		{
			_threadPool.Queue(std::bind(&Software3D::_Render, this, tris,clip));
		}
		else
		{
			_Render(tris,clip);
		}
	}

	inline void Software3D::_Render(const std::vector<Triangle>& __restrict tris, const Recti& __restrict clip) noexcept //445
	{
		std::function<void(Triangle)> renderer;

		switch (_fillMode)
		{
		case game::FillMode::WireFrameFilled: renderer = std::bind(&Software3D::DrawColored<true, true, false, false>, this, std::placeholders::_1, std::cref(clip)); break;
		case game::FillMode::WireFrame: renderer = std::bind(&Software3D::DrawColored<true, false, false, false>, this, std::placeholders::_1, std::cref(clip)); break;
		case game::FillMode::Filled: renderer = std::bind(&Software3D::DrawColored<false, true, false, false>, this, std::placeholders::_1, std::cref(clip)); break;
		default: break;
		}

		uint64_t trisSize = tris.size();
		for (uint32_t triangleCount = 0; triangleCount < trisSize; ++triangleCount)
		{
				renderer(std::cref(tris[triangleCount]));
		}
		_fence++;
	}

	inline const Recti Software3D::TriangleBoundingBox(const Triangle& tri) const noexcept
	{
		Recti boundingBox;

		float_t sx1 = (tri.vertices[0].x);
		float_t sx2 = (tri.vertices[1].x);
		float_t sx3 = (tri.vertices[2].x);
		float_t sy1 = (tri.vertices[0].y);
		float_t sy2 = (tri.vertices[1].y);
		float_t sy3 = (tri.vertices[2].y);

		boundingBox.right = (int32_t)(sx1 > sx2 ? (sx1 > sx3 ? sx1 : sx3) : (sx2 > sx3 ? sx2 : sx3));
		boundingBox.bottom = (int32_t)(sy1 > sy2 ? (sy1 > sy3 ? sy1 : sy3) : (sy2 > sy3 ? sy2 : sy3));
		boundingBox.left = (int32_t)(sx1 < sx2 ? (sx1 < sx3 ? sx1 : sx3) : (sx2 < sx3 ? sx2 : sx3));
		boundingBox.top = (int32_t)(sy1 < sy2 ? (sy1 < sy3 ? sy1 : sy3) : (sy2 < sy3 ? sy2 : sy3));

		return boundingBox;
	}

	template<bool renderWireFrame, bool renderColor, bool lighting, bool textured>
	inline void Software3D::DrawColored(const Triangle& triangle, const Recti& clip) noexcept
	{
		//game::Vector3f vertex0(triangle.vertices[0].x, triangle.vertices[0].y, 0);
		//game::Vector3f vertex1(triangle.vertices[1].x, triangle.vertices[1].y, 0);
		//game::Vector3f vertex2(triangle.vertices[2].x, triangle.vertices[2].y, 0);

		uint32_t foundTriangle(0);
		uint32_t videoBufferStride(_currentRenderTarget.size.width);

		game::Vector2f pixelOffset;

		Vector3f oneOverW(1.0f / triangle.vertices[0].w, 1.0f / triangle.vertices[1].w, 1.0f / triangle.vertices[2].w);

		// Color parameter	
		//Color colorAtPixel;
		//ParameterEquation rColorParam(triangle.color[0].rf * oneOverW.x, triangle.color[1].rf * oneOverW.y, triangle.color[2].rf * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);
		//ParameterEquation gColorParam(triangle.color[0].gf * oneOverW.x, triangle.color[1].gf * oneOverW.y, triangle.color[2].gf * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);
		//ParameterEquation bColorParam(triangle.color[0].bf * oneOverW.x, triangle.color[1].bf * oneOverW.y, triangle.color[2].bf * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);

		// Depth parameter
		float_t oneOverDepthEval(0.0f);
		ParameterEquation depthParam(oneOverW.x, oneOverW.y, oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);

		// Face normal light pre calc (directional light) can add ambient here
		Vector3f faceNormal(triangle.faceNormal);// (0.0f, 0.0f, 1.0f);
		Vector3f lightNormal(0.0f, 0.0f, 1.0f);  // direction the light is shining to (opposite for y)
		//lightNormal.Normalize();
		//rot += (2 * 3.14f / 10.0f) * (time / 1000.0f);
		//lightNormal = RotateXYZ(lightNormal, 0, time , 0);
		//Color lightColor = Colors::Yellow;
		float_t luminance = 1.0f;// -faceNormal.Dot(lightNormal);// Should have the negative as it is left handed
		luminance = max(0.25f, luminance); //ambient here for face

		// Vertex normal parameters (directional light)
		ParameterEquation vnx(triangle.normals[0].x * oneOverW.x, triangle.normals[1].x * oneOverW.y, triangle.normals[2].x * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);
		ParameterEquation vny(triangle.normals[0].y * oneOverW.x, triangle.normals[1].y * oneOverW.y, triangle.normals[2].y * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);
		ParameterEquation vnz(triangle.normals[0].z * oneOverW.x, triangle.normals[1].z * oneOverW.y, triangle.normals[2].z * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);
		

		// Texture parameters
		ParameterEquation uParam(triangle.uvs[0].u * oneOverW.x, triangle.uvs[1].u * oneOverW.y, triangle.uvs[2].u * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);
		ParameterEquation vParam(triangle.uvs[0].v * oneOverW.x, triangle.uvs[1].v * oneOverW.y, triangle.uvs[2].v * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);

		// Wireframe precalcs
		float_t d[3] = {};
		float_t minDistSq(0.0f);
		float_t yy[3] = {}; //y2 - y1;
		float_t xx[3] = {}; //x2 - x1;
		float_t xy[3] = {}; //x2 * y1 then xy - yx
		float_t yx[3] = {}; //y2 * x1;
		float_t denominator[3] = {};	//1.0f / (xx * xx + yy * yy);
		if (renderWireFrame)
		{
			yy[0] = triangle.vertices[1].y - triangle.vertices[0].y;
			xx[0] = triangle.vertices[1].x - triangle.vertices[0].x;
			xy[0] = triangle.vertices[1].x * triangle.vertices[0].y;
			yx[0] = triangle.vertices[1].y * triangle.vertices[0].x;

			yy[1] = triangle.vertices[2].y - triangle.vertices[1].y;
			xx[1] = triangle.vertices[2].x - triangle.vertices[1].x;
			xy[1] = triangle.vertices[2].x * triangle.vertices[1].y;
			yx[1] = triangle.vertices[2].y * triangle.vertices[1].x;

			yy[2] = triangle.vertices[0].y - triangle.vertices[2].y;
			xx[2] = triangle.vertices[0].x - triangle.vertices[2].x;
			xy[2] = triangle.vertices[0].x * triangle.vertices[2].y;
			yx[2] = triangle.vertices[0].y * triangle.vertices[2].x;

			xy[0] = xy[0] - yx[0];
			xy[1] = xy[1] - yx[1];
			xy[2] = xy[2] - yx[2];

			denominator[0] = 1.0f / (xx[0] * xx[0] + yy[0] * yy[0]);
			denominator[1] = 1.0f / (xx[1] * xx[1] + yy[1] * yy[1]);
			denominator[2] = 1.0f / (xx[2] * xx[2] + yy[2] * yy[2]);
		}

		uint32_t* colorBuffer = _currentRenderTarget.colorBuffer + (triangle.boundingBox.top * videoBufferStride + triangle.boundingBox.left);
		float_t* depthBufferPtr = _currentRenderTarget.depthBuffer + (triangle.boundingBox.top * videoBufferStride + triangle.boundingBox.left);
		uint32_t xLoopCount = 0;
		for (int32_t j = triangle.boundingBox.top; j <= triangle.boundingBox.bottom; ++j)
		{
			xLoopCount = 0;
			//if ((j % 2 == 0))  // cheap scanline effect
			//{
			//	colorBuffer += videoBufferStride - xLoopCount;
			//	depthBufferPtr += videoBufferStride - xLoopCount;
			//	continue;
			//}
			foundTriangle = 0;// foundTriangle^ foundTriangle;
			for (int32_t i = triangle.boundingBox.left; i <= triangle.boundingBox.right; ++i)
			{
				++xLoopCount;
				//if (i % 2 == 0)  // cheap scanline effect
				//{
				//	colorBuffer++;// = videoBufferStride - xLoopCount;
				//	depthBufferPtr++;// += videoBufferStride - xLoopCount;
				//	continue;
				//}
				pixelOffset = { i + 0.5f , j + 0.5f };

				if (triangle.edge0.test(pixelOffset.x, pixelOffset.y))
				{
					++colorBuffer;
					++depthBufferPtr;
					if (foundTriangle)
					{
						break;
					}
					else
					{
						continue;
					}
				}
				if (triangle.edge1.test(pixelOffset.x, pixelOffset.y))
				{
					++colorBuffer;
					++depthBufferPtr;
					if (foundTriangle)
					{
						break;
					}
					else
					{
						continue;
					}
				}
				if (triangle.edge2.test(pixelOffset.x, pixelOffset.y))
				{
					++colorBuffer;
					++depthBufferPtr;
					if (foundTriangle)
					{
						break;
					}
					else
					{
						continue;
					}
				}
				foundTriangle = 1;

				// depth buffer test
				oneOverDepthEval = 1.0f / (depthParam.evaluate(pixelOffset.x, pixelOffset.y));
				if (oneOverDepthEval+0.00001f < *depthBufferPtr)
				{

					*depthBufferPtr = oneOverDepthEval;
				}
				else
				{
					++colorBuffer;
					++depthBufferPtr;
					continue;
				}
				
				// Wireframe
				if (renderWireFrame)
				{
					auto distanceFromPointToLineSq = [&](const float_t x0, const float_t y0, const float_t yy, const float_t xx, const float_t xyyx, const float_t denominator)
						{
							float_t numerator = yy * x0 - xx * y0 + xyyx;
							numerator = numerator * numerator;
							return (numerator * denominator);
						};
					for (uint32_t dist = 0; dist < 3; dist++)
					{
						d[dist] = distanceFromPointToLineSq(pixelOffset.x, pixelOffset.y, yy[dist], xx[dist], xy[dist], denominator[dist]);
					}
					minDistSq = d[0] < d[1] ? (d[0] < d[2] ? d[0] : d[2]) : (d[1] < d[2] ? d[1] : d[2]);
					if (minDistSq < 1.0f)
					{
						*colorBuffer = game::Colors::White.packedARGB;
						++colorBuffer;
						++depthBufferPtr;
						continue;
					}
					//else
					//{
					//	if (!renderColor)
					//	{
					//		*colorBuffer = game::Colors::Black.packedARGB;
					//	}
					//}
				}

				// Color filled
				
				if (renderColor)
				{
					// No lighting
					//colorAtPixel.Set(r.evaluate(pixelOffset.x, pixelOffset.y) * dd, g.evaluate(pixelOffset.x, pixelOffset.y) * dd, b.evaluate(pixelOffset.x, pixelOffset.y) * dd, 1.0f);
					
					// original 1/(1/w)
					float_t pre = oneOverDepthEval;
					
					// Depth based lighting color
					//luminance = oneOverDepthEval + 1.0f;
					//luminance = 1.0f / luminance;
					//luminance += 0.3f; // simulate ambient 
					//luminance = min(luminance, 1.0f);

					// Vertex normal lighting
					//Vector3f vertexNormalEval(vnx.evaluate(pixelOffset.x, pixelOffset.y)*pre, vny.evaluate(pixelOffset.x, pixelOffset.y)*pre, vnz.evaluate(pixelOffset.x, pixelOffset.y)*pre);
					//luminance = -vertexNormalEval.Dot(lightNormal);
					//luminance = max(0.0f, luminance);// < 0.0f ? 0.0f : lum;

					// Face and vertex normal lighting amibient, needs calc once for face, every pixel for vertex
					//float_t luminanceAmbient(luminance + 0.25f);
					//luminanceAmbient = min(luminanceAmbient, 1.0f);

					//// Colored light
					//float rp = rColorParam.evaluate(pixelOffset.x, pixelOffset.y) * pre;
					//float gp = gColorParam.evaluate(pixelOffset.x, pixelOffset.y) * pre;
					//float bp = bColorParam.evaluate(pixelOffset.x, pixelOffset.y) * pre;

					//rp = min(((luminance * lightColor.rf) + (rp)) / 2.0f, 1.0f);
					//gp = min(((luminance * lightColor.gf) + (gp)) / 2.0f, 1.0f);
					//bp = min(((luminance * lightColor.bf) + (bp)) / 2.0f, 1.0f);
					//colorAtPixel.Set(rp, gp, bp, 1.0f);

					// Common to all lighting except colored and texture
					//float_t rd = min(rColorParam.evaluate(pixelOffset.x, pixelOffset.y) * pre, 1.0f) * luminanceAmbient;
					//float_t gd = min(gColorParam.evaluate(pixelOffset.x, pixelOffset.y) * pre, 1.0f) * luminanceAmbient;
					//float_t bd = min(bColorParam.evaluate(pixelOffset.x, pixelOffset.y) * pre, 1.0f) * luminanceAmbient;
					//colorAtPixel.Set(rd, gd, bd, 1.0f);
					//*colorBuffer = colorAtPixel.packedABGR;
					
					// texture stuff
					float_t up = uParam.evaluate(pixelOffset.x, pixelOffset.y) * pre;
					float_t vp = vParam.evaluate(pixelOffset.x, pixelOffset.y) * pre;
					// calculate texture lookup
					up = min(up, 1.0f);
					vp = min(vp, 1.0f);
					// changed to unsigned 02/05
					uint32_t tx = max((int32_t)(up * (_currentTexture.size.width-1) + 0.5f), 0);	// -1 fix texture seams at max texW and texH
					uint32_t ty = max((int32_t)(vp * (_currentTexture.size.height-1) + 0.5f), 0);

					// texture lighting
					uint32_t color = _currentTexture.data[ty * _currentTexture.size.width + tx];
					uint32_t rc = (color >> 0) & 0xFF;  
					uint32_t gc = (color >> 8) & 0xFF;
					uint32_t bc = (color >> 16) & 0xFF;
					rc = (uint32_t)(rc * luminance);//Ambient);
					gc = (uint32_t)(gc * luminance);//Ambient);
					bc = (uint32_t)(bc * luminance);//Ambient); 
					//color = ((0xFF << 24) | (bc << 16) | (gc << 8) | (rc)); //7.25

					*colorBuffer = ((0xFF << 24) | (bc << 16) | (gc << 8) | (rc));// colorAtPixel.packedABGR; 
				}
				++colorBuffer;
				++depthBufferPtr;
			}
			colorBuffer += videoBufferStride - xLoopCount;
			depthBufferPtr += videoBufferStride - xLoopCount;
		}
	}

	// Screen clipping and some precalculations 
	inline void Software3D::ScreenClip(std::vector<Triangle>& in, ClippingRects& clip, const uint32_t index) const noexcept
	{
		clip.clippedTris[index].clear();
		Triangle outTri;
		uint64_t inSize = in.size();
		for (uint32_t tri = 0; tri < inSize; tri++)
		{
			//Triangle outTri(in[tri]);
			if (in[tri].backFaceCulled) continue; // was backface culled before

			// Far Z clip
			if ((in[tri].vertices[0].w > 100.0f) ||
				(in[tri].vertices[1].w > 100.0f) ||
				(in[tri].vertices[2].w > 100.0f))
			{
				continue;
			}

			// Calculate edges and backface cull
			if (!in[tri].edgeCalculated)
			{
				in[tri].edge0.Set(in[tri].vertices[1], in[tri].vertices[2]);
				in[tri].edge1.Set(in[tri].vertices[2], in[tri].vertices[0]);
				in[tri].edge2.Set(in[tri].vertices[0], in[tri].vertices[1]);
				in[tri].edgeCalculated = true;
				in[tri].area = 1.0f /  (in[tri].edge0.c + in[tri].edge1.c + in[tri].edge2.c);
				if (in[tri].area < 0)
				{
					in[tri].backFaceCulled = true;
					continue;
				}
			}
			if (!in[tri].boundingCalculated)
			{
				in[tri].boundingBox = TriangleBoundingBox(in[tri]);
				in[tri].boundingCalculated = true;
			}
			// Screen clipping
			// Offscreen completely
			if ((in[tri].boundingBox.right < clip.clips[index].left) || (in[tri].boundingBox.left > clip.clips[index].right) ||
				(in[tri].boundingBox.bottom < clip.clips[index].top) || (in[tri].boundingBox.top > clip.clips[index].bottom))
			{
				continue;
			}


			// This needs copied as the bounding box modification 
			// will only apply to this clip
			outTri = in[tri];

			// Partial offscreen
			if (outTri.boundingBox.left < clip.clips[index].left)
				outTri.boundingBox.left = clip.clips[index].left;
			if (outTri.boundingBox.right > clip.clips[index].right)
				outTri.boundingBox.right = clip.clips[index].right;
			if (outTri.boundingBox.top < clip.clips[index].top)
				outTri.boundingBox.top = clip.clips[index].top;
			if (outTri.boundingBox.bottom > clip.clips[index].bottom)
				outTri.boundingBox.bottom = clip.clips[index].bottom;

			clip.clippedTris[index].emplace_back(outTri);
		}
	}

	void ConvertBlenderToThis(game::Mesh& mesh)
	{
		uint32_t meshSize = (uint32_t)mesh.tris.size();
		for (uint32_t tri = 0; tri < meshSize; tri++)
		{
			std::swap(mesh.tris[tri].vertices[0].y, mesh.tris[tri].vertices[0].z);
			std::swap(mesh.tris[tri].vertices[1].y, mesh.tris[tri].vertices[1].z);
			std::swap(mesh.tris[tri].vertices[2].y, mesh.tris[tri].vertices[2].z);

			std::swap(mesh.tris[tri].normals[0].y, mesh.tris[tri].normals[0].z);
			std::swap(mesh.tris[tri].normals[1].y, mesh.tris[tri].normals[1].z);
			std::swap(mesh.tris[tri].normals[2].y, mesh.tris[tri].normals[2].z);

			std::swap(mesh.tris[tri].faceNormal.y, mesh.tris[tri].faceNormal.z);

			mesh.tris[tri].uvs[0].v = 1.0f - mesh.tris[tri].uvs[0].v;
			mesh.tris[tri].uvs[1].v = 1.0f - mesh.tris[tri].uvs[1].v;
			mesh.tris[tri].uvs[2].v = 1.0f - mesh.tris[tri].uvs[2].v;
		}
	}

	bool LoadObj(std::string file, game::Mesh& mesh)
	{
		std::ifstream f(file.c_str());
		std::vector<game::Vector3f> verts;
		std::vector<game::Vector3f> norms;
		std::vector<float> vcount;
		std::vector<game::Vector3i> fcount;
		std::vector<game::Vector3f> vnorms;

		std::vector<game::Vector2f> uvs;
		game::Vector3f vert;
		bool hasNormals = false;
		bool hasUVs = false;
		char line[256] = {};

		// Check to see if file has normals
		if (f.is_open())
		{
			while (!f.eof())
			{
				f.getline(line, 256);
				if (line[0] == 'v' && line[1] == 'n') hasNormals = true;
				if (line[0] == 'v' && line[1] == 't') hasUVs = true;
			}
		}
		// Reset file
		f.clear();
		f.seekg(0);
		hasNormals = false;

		// Parse the file
		if (f.is_open())
		{
			uint8_t junk = 0;
			uint32_t p1 = 0, p2 = 0, p3 = 0;
			uint32_t n1 = 0, n2 = 0, n3 = 0;
			uint32_t uv1 = 0, uv2 = 0, uv3 = 0;
			while (!f.eof())
			{
				junk = 0;
				f.getline(line, 256);
				std::stringstream ss;
				ss << line;

				if (line[0] == 'v')
				{
					if (line[1] == 'n') // Vertex normals
					{
						ss >> junk >> junk >> vert.x >> vert.y >> vert.z;
						//vert.z = -vert.z;
						norms.emplace_back(vert);
						continue;
					}
					else if (line[1] == 't')  // texture uvs
					{
						ss >> junk >> junk >> vert.x >> vert.y;
						uvs.emplace_back(game::Vector2f(vert.x, vert.y));
						continue;
					}
					else
					{
						ss >> junk >> vert.x >> vert.y >> vert.z;
						verts.emplace_back(vert);
						// start counting verts
						vcount.emplace_back(1.0f);
						// if it has no normals make temporary ones
						if (!hasNormals)
						{
							norms.emplace_back(game::Vector3f(0, 0, 0));
						}
						continue;
					}
				}
				if (line[0] == 'f')
				{
					if (hasUVs && hasNormals)
					{
						ss >> junk >> p1 >> junk >> uv1 >> junk >> n1;
						ss >> p2 >> junk >> uv2 >> junk >> n2;
						ss >> p3 >> junk >> uv3 >> junk >> n3;
					}
					else if (hasNormals)
					{
						ss >> junk >> p1 >> junk >> junk >> n1;
						ss >> p2 >> junk >> junk >> n2;
						ss >> p3 >> junk >> junk >> n3;
					}
					else if (hasUVs)
					{
						// may have to get rid of the ns as junk
						ss >> junk >> p1 >> junk >> uv1 >> junk >> n1;
						ss >> p2 >> junk >> uv2 >> junk >> n2;
						ss >> p3 >> junk >> uv3 >> junk >> n3;
					}
					else
					{
						ss >> junk >> p1 >> p2 >> p3;
					}
					game::Triangle tri;
					// Vertices
					tri.vertices[0] = verts[(size_t)p1 - 1];
					tri.vertices[1] = verts[(size_t)p2 - 1];
					tri.vertices[2] = verts[(size_t)p3 - 1];
					// UV (texture) coords
					if (hasUVs)
					{
						tri.uvs[0] = uvs[(size_t)uv1 - 1];// Vector2d(uvs[uv1 - 1].x, uvs[uv1 - 1].y);
						tri.uvs[1] = uvs[(size_t)uv2 - 1];// Vector2d(uvs[uv2 - 1].x, uvs[uv2 - 1].y);
						tri.uvs[2] = uvs[(size_t)uv3 - 1];// Vector2d(uvs[uv3 - 1].x, uvs[uv3 - 1].y);
					}
					else
					{
						tri.uvs[0];// = game::Vector2f(0, 0);
						tri.uvs[1];// = game::Vector2f(0, 0);
						tri.uvs[2];// = game::Vector2f(0, 0);
					}


					// count the vertices
					if (!hasNormals)
					{
						vcount[(size_t)p1 - 1]++;
						vcount[(size_t)p2 - 1]++;
						vcount[(size_t)p3 - 1]++;
						game::Vector3i t;
						t.x = p1 - 1;
						t.y = p2 - 1;
						t.z = p3 - 1;
						fcount.emplace_back(t);
					}


					game::Vector3f a, b;
					// Calculate the face normal of the triangle
					a = tri.vertices[1] - tri.vertices[0];
					b = tri.vertices[2] - tri.vertices[0];
					// this was changed to make face normals work with gamelib2
					//tri.faceNormal = b.Cross(a);
					tri.faceNormal = a.Cross(b);  // orig


					if (hasNormals)
					{
						// Add the face normal to the vertex normals
						tri.faceNormal.Normalize();
						tri.normals[0] = norms[(size_t)n1 - 1];// * -1.0f;
						tri.normals[1] = norms[(size_t)n2 - 1];// * -1.0f;
						tri.normals[2] = norms[(size_t)n3 - 1];// * -1.0f;
						tri.normals[0].Normalize();
						tri.normals[1].Normalize();
						tri.normals[2].Normalize();
					}
					else
					{
						// Sum the normals
						norms[(size_t)p1 - 1] += tri.faceNormal;// *-1.0f;
						norms[(size_t)p2 - 1] += tri.faceNormal;// *-1.0f;
						norms[(size_t)p3 - 1] += tri.faceNormal;// *-1.0f;
						tri.faceNormal.Normalize();

					}

					mesh.tris.emplace_back(tri);

					continue;
				}
			}
			uint64_t meshSize = mesh.tris.size();
			if (!hasNormals)
			{
				for (int i = 0; i < meshSize; i++)
				{
					mesh.tris[i].normals[0] = norms[fcount[i].x] / vcount[fcount[i].x];
					mesh.tris[i].normals[1] = norms[fcount[i].y] / vcount[fcount[i].y];
					mesh.tris[i].normals[2] = norms[fcount[i].z] / vcount[fcount[i].z];
					mesh.tris[i].normals[0].Normalize();
					mesh.tris[i].normals[1].Normalize();
					mesh.tris[i].normals[2].Normalize();
				}
			}
			ConvertBlenderToThis(mesh);
			for (int i = 0; i < meshSize; i++)
			{
				mesh.tris[i].color[0] = game::Colors::White;
				mesh.tris[i].color[1] = game::Colors::White;
				mesh.tris[i].color[2] = game::Colors::White;
				mesh.centerPoint += mesh.tris[i].vertices[0];
				mesh.centerPoint += mesh.tris[i].vertices[1];
				mesh.centerPoint += mesh.tris[i].vertices[2];
			}
			mesh.centerPoint = mesh.centerPoint / ((float_t)mesh.tris.size() * 3.0f);	

			return true;
		}
		else return false;

	}

	inline void Software3D::VertexProcessor(game::Mesh& mesh, const game::Matrix4x4f& mvp, std::vector<game::Triangle>& processedTris, Camera3D& camera) const noexcept
	{
		mesh.GenerateModelMatrix();
		Matrix4x4f mvpCopy(mvp);
		mvpCopy = mvpCopy * mesh.model;
		game::Triangle workingTriangle;


		game::Triangle newClippedTris[2];
		uint32_t numberTrisGenerated = 0;
		uint64_t meshSize = mesh.tris.size();
		//int culled = 0;

		Vector3f cameraRay;
		for (uint32_t i = 0; i < meshSize; i++)
		{
			workingTriangle = mesh.tris[i];

			// Backface cull before clip, there ARE artifacts when scaling non uniformly or if scale is negative
			workingTriangle.faceNormal = workingTriangle.faceNormal * mesh.rotation;
			cameraRay = (workingTriangle.vertices[0] * mesh.model) - camera.position;
			if (workingTriangle.faceNormal.Dot(cameraRay) >= 0.0f)
			{
				//culled++;
				continue;
			}

			workingTriangle.vertices[0] = (mesh.tris[i].vertices[0] * mvpCopy);
			workingTriangle.vertices[1] = (mesh.tris[i].vertices[1] * mvpCopy);
			workingTriangle.vertices[2] = (mesh.tris[i].vertices[2] * mvpCopy);

			workingTriangle.normals[0] = workingTriangle.normals[0] * mesh.rotation;
			workingTriangle.normals[1] = workingTriangle.normals[1] * mesh.rotation;
			workingTriangle.normals[2] = workingTriangle.normals[2] * mesh.rotation;

			if ((workingTriangle.vertices[0].z < 0.0) ||
				(workingTriangle.vertices[1].z < 0.0) ||
				(workingTriangle.vertices[2].z < 0.0))
			{
				numberTrisGenerated = ClipAgainstNearZ(workingTriangle, newClippedTris[0], newClippedTris[1]);
				for (uint32_t tri = 0; tri < numberTrisGenerated; ++tri)
				{
					//PerspectiveDivide(newClippedTris[tri]);
					newClippedTris[tri].vertices[0] /= newClippedTris[tri].vertices[0].w;
					newClippedTris[tri].vertices[1] /= newClippedTris[tri].vertices[1].w;
					newClippedTris[tri].vertices[2] /= newClippedTris[tri].vertices[2].w;
					ScaleToScreen(newClippedTris[tri], _currentRenderTarget.halfSize);
					if (CheckWinding(newClippedTris[tri].vertices[0], newClippedTris[tri].vertices[1], newClippedTris[tri].vertices[2]) < 0)
					{
						std::swap(newClippedTris[tri].vertices[1], newClippedTris[tri].vertices[0]);
						std::swap(newClippedTris[tri].normals[1], newClippedTris[tri].normals[0]);
						std::swap(newClippedTris[tri].uvs[1], newClippedTris[tri].uvs[0]);
						std::swap(newClippedTris[tri].color[1], newClippedTris[tri].color[0]);
					}
					processedTris.emplace_back(newClippedTris[tri]);
				}
			}
			else
			{
				//PerspectiveDivide(workingTriangle);
				workingTriangle.vertices[0] /= workingTriangle.vertices[0].w;
				workingTriangle.vertices[1] /= workingTriangle.vertices[1].w;
				workingTriangle.vertices[2] /= workingTriangle.vertices[2].w;


				ScaleToScreen(workingTriangle, _currentRenderTarget.halfSize);
				processedTris.emplace_back(workingTriangle);
			}
		}
		//std::cout << culled << "\n";

	}
}

static void testmy_PerspectiveFOV(const float_t fov, const float_t aspect, const float_t nearz, const float_t farz)
{
	float_t D2R = 3.14159f / 180.0f;
	float_t halfFOV = tan((D2R * fov) / 2.0f);
	float_t yScale = 1.0f / halfFOV;
	float_t xScale = 1.0f / (aspect * halfFOV);
	float_t m[] = {
		xScale, 0,      0,                                         0,
		0,      yScale, 0,                                         0,
		0,      0,      (farz + nearz) / (farz - nearz),    	 1.0f,
		0,      0,		-(2.0f * farz * nearz) / (farz - nearz),    0
	};
	//proj.a = xScale;
	//proj.b = yScale;
	//proj.c = farz / nearmfar;
	//proj.d = 1.0f;
	//proj.e = -(nearz * farz) / nearmfar;
	//memcpy(&mret, m, sizeof(float_t) * 16);
	int e = 0;
	game::Vector3f N(0.0f, 0.0f, 0.1f);// glm::vec4 N = Projection * glm::vec4(0.f, 0.f, Near, 1.f);
	game::Vector3f F(0.0f, 0.0f, 100.0f); //glm::vec4 F = Projection * glm::vec4(0.f, 0.f, Far, 1.f);
	game::Vector3f ret;
	//ret.x = (N.x * m[0] + N.y * m[4] + N.z * m[8] + N.w * m[12]);
	//ret.y = (N.x * m[1] + N.y * m[5] + N.z * m[9] + N.w * m[13]);
	//ret.z = (N.x * m[2] + N.y * m[6] + N.z * m[10] + N.w * m[14]);
	//ret.w = (N.x * m[3] + N.y * m[7] + N.z * m[11] + N.w * m[15]);
	ret.x = (N.x * m[0]);// +N.y * m[4] + N.z * m[8] + N.w * m[12]);
	ret.y = N.y * m[5];// (N.x * m[1] + N.y * m[5] + N.z * m[9] + N.w * m[13]);
	ret.z = /*(N.x * m[2] + N.y * m[6] + */(N.z * m[10] + N.w * m[14]);
	ret.w = /*(N.x * m[3] + N.y * m[7] + */ N.z * m[11];// +N.w * m[15]);
	ret /= ret.w;
	e += (ret.z != -1.0f);
	std::cout << "\nFOV -1 to +1\n";
	std::cout << "Nz = " << ret.z << "\n";
	std::cout << "error = " << e << "\n";


	N = F;
	//ret.x = (N.x * m[0] + N.y * m[4] + N.z * m[8] + N.w * m[12]);
	//ret.y = (N.x * m[1] + N.y * m[5] + N.z * m[9] + N.w * m[13]);
	//ret.z = (N.x * m[2] + N.y * m[6] + N.z * m[10] + N.w * m[14]);
	//ret.w = (N.x * m[3] + N.y * m[7] + N.z * m[11] + N.w * m[15]);
	ret.x = (N.x * m[0]);// +N.y * m[4] + N.z * m[8] + N.w * m[12]);
	ret.y = N.y * m[5];// (N.x * m[1] + N.y * m[5] + N.z * m[9] + N.w * m[13]);
	ret.z = /*(N.x * m[2] + N.y * m[6] + */(N.z * m[10] + N.w * m[14]);
	ret.w = /*(N.x * m[3] + N.y * m[7] + */ N.z * m[11];// +N.w * m[15]);
	ret /= ret.w;
	e += (ret.z != 1.0f);
	std::cout << "Fz = " << ret.z << "\n";
	std::cout << "error = " << e << "\n";

}

static void testmy_PerspectiveFOV2(const float_t fov, const float_t aspect, const float_t nearz, const float_t farz)
{
	float_t D2R = 3.14159f / 180.0f;
	float_t halfFOV = tan((D2R * fov) / 2.0f);
	float_t yScale = 1.0f / halfFOV;
	float_t xScale = 1.0f / (aspect * halfFOV);
	float_t m[] = {
		xScale, 0,      0,                           0,
		0,      yScale, 0,                           0,
		0,      0,      farz / (farz - nearz),			 1,
		0,      0,		-(nearz * farz) / (farz - nearz),	 0
	};
	//proj.a = xScale;
	//proj.b = yScale;
	//proj.c = farz / nearmfar;
	//proj.d = 1.0f;
	//proj.e = -(nearz * farz) / nearmfar;
	//memcpy(&mret, m, sizeof(float_t) * 16);
	int e = 0;
	game::Vector3f N(0.0f, 0.0f, 0.1f);// glm::vec4 N = Projection * glm::vec4(0.f, 0.f, Near, 1.f);
	game::Vector3f F(0.0f, 0.0f, 100.0f); //glm::vec4 F = Projection * glm::vec4(0.f, 0.f, Far, 1.f);
	game::Vector3f ret;
	//ret.x = (N.x * m[0] + N.y * m[4] + N.z * m[8] + N.w * m[12]);
	//ret.y = (N.x * m[1] + N.y * m[5] + N.z * m[9] + N.w * m[13]);
	//ret.z = (N.x * m[2] + N.y * m[6] + N.z * m[10] + N.w * m[14]);
	//ret.w = (N.x * m[3] + N.y * m[7] + N.z * m[11] + N.w * m[15]);
	ret.x = (N.x * m[0]);// +N.y * m[4] + N.z * m[8] + N.w * m[12]);
	ret.y = N.y * m[5];// (N.x * m[1] + N.y * m[5] + N.z * m[9] + N.w * m[13]);
	ret.z = /*(N.x * m[2] + N.y * m[6] + */(N.z * m[10] + N.w * m[14]);
	ret.w = /*(N.x * m[3] + N.y * m[7] + */ N.z * m[11];// +N.w * m[15]);
	ret /= ret.w;
	e += (ret.z != 0);
	std::cout << "\nFOV2 0 to +1\n";
	std::cout << "Nz = " << ret.z << "\n";
	std::cout << "error = " << e << "\n";


	N = F;
	//ret.x = (N.x * m[0] + N.y * m[4] + N.z * m[8] + N.w * m[12]);
	//ret.y = (N.x * m[1] + N.y * m[5] + N.z * m[9] + N.w * m[13]);
	//ret.z = (N.x * m[2] + N.y * m[6] + N.z * m[10] + N.w * m[14]);
	//ret.w = (N.x * m[3] + N.y * m[7] + N.z * m[11] + N.w * m[15]);
	ret.x = (N.x * m[0]);// +N.y * m[4] + N.z * m[8] + N.w * m[12]);
	ret.y = N.y * m[5];// (N.x * m[1] + N.y * m[5] + N.z * m[9] + N.w * m[13]);
	ret.z = /*(N.x * m[2] + N.y * m[6] + */(N.z * m[10] + N.w * m[14]);
	ret.w = /*(N.x * m[3] + N.y * m[7] + */ N.z * m[11];// +N.w * m[15]);
	ret /= ret.w;
	e += (ret.z != 1);
	std::cout << "Fz = " << ret.z << "\n";
	std::cout << "error = " << e << "\n";

}



#endif