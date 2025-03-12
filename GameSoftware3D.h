#if !defined(GAMESOFTWARE3D_H)
#define GAMESOFTWARE3D_H

#include <vector>
#include "GameMath.h"
#include "GameColor.h"
#include "GamePixelMode.h"
#include "GameSoftware3D_Camera3D.h"
#include "GameSoftware3D_Data.h"
#include "GameSoftware3D_Math.h"
#include "GameSoftware3D_Particle.h"
#include "GameSoftware3D_PointSprite.h"
#include "GameThreadPool.h"

namespace game
{

	class Software3D
	{
	public:
		Software3D();
		~Software3D();
		bool Initialize(PixelMode& pixelMode, const int32_t threads);
		int32_t SetState(const uint32_t state, const int32_t value) noexcept;
		int32_t SetState(const uint32_t state, const float_t value) noexcept;
		bool SetTexture(const Texture& texture) noexcept;
		bool SetNormalMap(const Texture& texture) noexcept;
		bool SetTexture(const RenderTarget& target) noexcept;
		bool SetDefaultTexture() noexcept;
		bool LoadTexture(const std::string& file, Texture& texture);
		void Fence(const uint64_t fenceValue) noexcept;
		void TriangleBoundingBox(Triangle& tri) const noexcept;
		void Render(const std::vector<Triangle>& tris, const Recti& clip) noexcept;
		template<bool wireFrame, bool filled, bool lighting, bool textured>
		void Rasterize(const Triangle& tri, const Recti& __restrict clip) noexcept;
		uint32_t NumberOfThreads() const noexcept { return _threadPool.NumberOfThreads(); }
		void ClearDepth(const float_t depth);
		void ClearRenderTarget(const Color& color, const float_t depth) const;
		void ScreenClip(std::vector<Triangle>& in, ClippingRects& clip, const uint32_t index) const noexcept;
		bool CreateTexture(const uint32_t width, const uint32_t height, Texture& texture) noexcept;
		bool CreateTexture(const Pointi size, Texture& texture) noexcept;
		void DeleteTexture(Texture& texture) noexcept;
		bool CreateRenderTarget(const uint32_t width, const uint32_t height, RenderTarget& target) noexcept;
		bool CreateRenderTarget(const Pointi size, RenderTarget& target) noexcept;
		void DeleteRenderTarget(RenderTarget& target) noexcept;
		bool SetRenderTarget(const RenderTarget& target) noexcept;
		void SetRenderTargetDefault() noexcept;
		void SetStateObject(const Software3DStateObject& state) noexcept;

		void VertexProcessor(game::Mesh& mesh, const uint64_t numberOfTris, const game::Matrix4x4f& __restrict mvp, std::vector<game::Triangle>& processedTris, Camera3D& camera) const noexcept;
		void RenderMesh(Mesh& mesh, const uint64_t numberOfTris, Matrix4x4f& projection, Camera3D& camera, ClippingRects& clip) noexcept;
		float_t* depthBuffer;
		std::vector<Light> lights;
	private:
		std::vector<game::Triangle> _trianglesToRender;
		RenderTarget _currentRenderTarget;
		float_t* _clearDepthBuffer[10];
		void _Render(const std::vector<Triangle>& tris, const Recti& __restrict clip) noexcept;
		void _GenerateDefaultTexture(uint32_t* buff, const uint32_t w, const uint32_t h);
		std::atomic<uint32_t> _fence;
		bool _usingRenderTarget;
		RenderTarget _defaultRenderTarget;
		Texture _defaultTexture;
		uint32_t _numbuffers;
		uint32_t _currentDepth;
		Texture _currentTexture;
		Texture _currentNormalMap;
		ThreadPool _threadPool;

		bool _multiThreaded;
		SortingType _sortType;
		FillMode _fillMode;
		bool _enableTexturing;
		bool _enableLighting;
		bool _enableBackFaceCulling;
		bool _enableDepthWrite;
		bool _enableColorTinting;
		LightingType _lightingType;
		bool _enableAlphaTest;
		uint32_t _alphaTestValue;
		bool _enableAlphaBlend;
		float_t _wireFrameThicknessSquared;
		uint32_t _wireFrameColor;

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
		_multiThreaded = true;// false;
		depthBuffer = nullptr;
		_enableDepthWrite = true;
		_enableTexturing = false;
		_enableLighting = false;
		_enableBackFaceCulling = true;
		_lightingType = LightingType::Face;
		_sortType = SortingType::FrontToBack;
		_enableAlphaTest = false;
		_alphaTestValue = 128;
		_enableAlphaBlend = false;
		_numbuffers = 2;
		_enableColorTinting = false;
		_wireFrameThicknessSquared = 1.0f;
		_wireFrameColor = Colors::White.packedABGR;
		for (uint32_t i = 0; i < 10; i++)
			_clearDepthBuffer[i] = nullptr;
		_fillMode = FillMode::WireFrameFilled;
		CreateTexture(1024, 1024, _defaultTexture);
		_GenerateDefaultTexture(_defaultTexture.data, 1024, 1024);
		SetTexture(_defaultTexture);
		_trianglesToRender.reserve(1000);
	}

	Software3D::~Software3D()
	{
		_threadPool.Stop();
		for (uint32_t i = 0; i < _numbuffers; i++)
		{
			if (_clearDepthBuffer[i] != nullptr)
			{
				_aligned_free(_clearDepthBuffer[i]);
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

	inline bool Software3D::SetNormalMap(const Texture& texture) noexcept
	{
		if (texture.data == nullptr)
		{
			_currentNormalMap = _defaultTexture;
			return false;
		}
		_currentNormalMap = texture;
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
		temp.sizeMinusOne.width = (float_t)temp.size.width - 1.05f;  // needs to match create texture
		temp.sizeMinusOne.height = (float_t)temp.size.height - 1.05f;
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
		texture.data = (uint32_t*)_aligned_malloc((size_t)width * height * sizeof(float_t), 16); //new uint32_t[width * height];
		if (texture.data == nullptr)
		{
			return false;
		}
		texture.size.width = width;
		texture.size.height = height;
		texture.sizeMinusOne.width  = (float_t)width - 1.05f;
		texture.sizeMinusOne.height = (float_t)height - 1.05f;
		return true;
	}

	inline bool Software3D::CreateTexture(const Pointi size, Texture& texture) noexcept
	{
		return CreateTexture(size.width, size.height, texture);
	}

	inline void Software3D::DeleteTexture(Texture& texture) noexcept
	{
		if (texture.data != nullptr)
		{
			_aligned_free(texture.data);
			texture.data = nullptr;
		}
		texture.size.width = 0;
		texture.size.height = 0;
	}

	inline void Software3D::_GenerateDefaultTexture(uint32_t* buff, const uint32_t w, const uint32_t h)
	{
		game::Color col1 = game::Colors::Yellow;
		//game::Color col2 = game::Colors::Magenta;
		for (uint32_t y = 0; y < h; ++y)
		{
			//if (y % 2 == 0)
				//std::swap(col1, col2);
			for (uint32_t x = 0; x < w; ++x)
			{
				//if (x % 2 == 0)
					//std::swap(col1, col2);
				col1.Set(x ^ y, x ^ y, x ^ y, 255);
				buff[y * w + x] = col1.packedABGR;
			}
		}
	}

	inline bool Software3D::LoadTexture(const std::string& file, Texture& texture)
	{
		game::ImageLoader imageloader;   
		uint32_t t = 0;
		uint32_t texw = 0;
		uint32_t texh = 0;
		uint32_t* temp = (uint32_t*)imageloader.Load(file.c_str(), texw, texh, t);
		if (temp == nullptr)
		{
			std::cout << "Could not load texture \"" << file << "\" and default texture will be used!\n";
			return false;
		}
		else
		{
			texture.data = (uint32_t*)_aligned_malloc((size_t)texw * texh * sizeof(uint32_t), 16); //new uint32_t[texw * texh];
			if (texture.data != nullptr)
			{
				memcpy(texture.data, temp, (size_t)texw * texh * 4);
				texture.size.width = texw;
				texture.size.height = texh;
				texture.sizeMinusOne.width = (float_t)texture.size.width - 1.05f;  // needs to match create texture
				texture.sizeMinusOne.height = (float_t)texture.size.height - 1.05f;
			}
		}
		return true;
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
		_currentRenderTarget.colorBuffer = _pixelMode->videoBuffer;
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

	inline void Software3D::SetStateObject(const Software3DStateObject& state) noexcept
	{
		_enableAlphaBlend = state.alphaBlend;					// Do alpha blending
		_enableAlphaTest = state.alphaTest;						// Do alpha testing
		_alphaTestValue = state.alphaTestValue;					// Minimal value to render while alpha testing
		_enableBackFaceCulling = state.backFaceCulling;			// Do back face culling
		_enableColorTinting = state.colorTinting;				// Do color tiniting of textures
		_enableDepthWrite = state.depthWrite;					// Do depth writing
		_fillMode = state.fillMode;								// How to fill the triangle or not
		_enableLighting = state.lighting;						// Do lighting
		_lightingType = state.lightingType;						// Type of lighting
		//_multiThreaded = state.multiThreaded;					// Do pipeline multithreading
		_sortType = state.sortType;								// How to sort the triangles or not
		_enableTexturing = state.texturing;						// Do texturing
		_wireFrameColor = state.wireFrameColor;					// Color of the wireframe
		_wireFrameThicknessSquared = state.wireFrameThicknessSquared;	// Thickness of wireframe squared
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

	inline void Software3D::ClearRenderTarget(const Color& color, const float_t depth) const
	{
		std::fill_n(_currentRenderTarget.depthBuffer, _currentRenderTarget.totalBufferSize, depth);
		std::fill_n(_currentRenderTarget.colorBuffer, _currentRenderTarget.totalBufferSize, color.packedABGR);
	}

	inline int32_t Software3D::SetState(const uint32_t state, const float_t value) noexcept
	{
		switch (state)
		{
		case GAME_SOFTWARE3D_WIREFRAME_THICKNESS:
		{
			_wireFrameThicknessSquared = value * value;
			return true;
			break;
		}
		}
		return false;
	}

	inline int32_t Software3D::SetState(const uint32_t state, const int32_t value) noexcept
	{
		switch (state)
		{
		case GAME_SOFTWARE3D_FILL_MODE:
		{
			if ((value < (int32_t)FillMode::WireFrame) || (value >= (int32_t)FillMode::None))
			{
				return false;
			}
			_fillMode = (FillMode)value;
			return true;
		}
		case GAME_SOFTWARE3D_THREADED:
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
		case GAME_SOFTWARE3D_LIGHTING:
		{
			_enableLighting = value;
			return true;
		}
		case GAME_SOFTWARE3D_LIGHTING_TYPE:
		{
			if (value < LightingType::Face) return false;
			if (value > LightingType::Depth) return false;

			_lightingType = (LightingType)value;
			return true;
		}
		case GAME_SOFTWARE3D_TEXTURE:
		{
			_enableTexturing = value;
			return true;
		}
		case GAME_SOFTWARE3D_ALPHA_TEST:
		{
			_enableAlphaTest = value;
			return true;
		}
		case GAME_SOFTWARE3D_ALPHA_TEST_VALUE:
		{
			if (value < 0) return false;
			if (value > 255) return false;
			_alphaTestValue = value;
			return true;
		}
		case GAME_SOFTWARE3D_BACKFACECULL:
		{
			_enableBackFaceCulling = value;
			return true;
		}
		case GAME_SOFTWARE3D_DEPTH_WRITE:
		{
			_enableDepthWrite = value;
			return true;
		}
		case GAME_SOFTWARE3D_SORT:
		{
			if ((value < SortingType::BackToFront) && (value > SortingType::NoSort))
				return false;

			_sortType = (SortingType)value;
			return true;
		}
		case GAME_SOFTWARE3D_ALPHA_BLEND:
		{
			_enableAlphaBlend = value;
			return true;
		}
		case GAME_SOFTWARE3D_COLOR_TINTING:
		{
			_enableColorTinting = value;
			return true;
		}
		case GAME_SOFTWARE3D_WIREFRAME_COLOR:
		{
			_wireFrameColor = value;
			return true;
		}
		case GAME_SOFTWARE3D_WIREFRAME_THICKNESS:
		{
			_wireFrameThicknessSquared = (float_t)(value * value);
			return true;
		}
		default: 
		{
			return false;
		}
		}
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
			_clearDepthBuffer[i] = (float_t*)_aligned_malloc(_totalBufferSize * sizeof(float_t), 16);
			std::fill_n(_clearDepthBuffer[i], _totalBufferSize, 100.0f);
		}
		_currentDepth = 0;
		_defaultRenderTarget.depthBuffer = _clearDepthBuffer[_currentDepth];
		_currentRenderTarget = _defaultRenderTarget;
		return true;
	}

	inline void Software3D::RenderMesh(Mesh& mesh, const uint64_t numberOfTris, Matrix4x4f& projection, Camera3D& camera, ClippingRects& clip) noexcept
	{
		VertexProcessor(mesh, numberOfTris, projection, _trianglesToRender, camera);
		SetTexture(mesh.texture);
		SetNormalMap(mesh.normalMap);
		uint64_t fenceCount = {};

		// needs to sort length(cam.pos - vertex) 
		auto backToFront = [](const game::Triangle& a, const game::Triangle& b)
			{
				float_t az = a.vertices[0].z + a.vertices[1].z + a.vertices[2].z;
				float_t bz = b.vertices[0].z + b.vertices[1].z + b.vertices[2].z;
				return az > bz;
			};
		auto frontToBack = [](const game::Triangle& a, const game::Triangle& b)
			{
				float_t az = a.vertices[0].z + a.vertices[1].z + a.vertices[2].z;
				float_t bz = b.vertices[0].z + b.vertices[1].z + b.vertices[2].z;
				return az < bz;
			};

		const uint32_t num = clip.numberOfClipRects;
		for (uint32_t c = 0; c < num; c++)
		{
			ScreenClip(_trianglesToRender, clip, c);
			if (!clip.clippedTris[c].size()) continue;
			switch (_sortType)
			{
			case SortingType::BackToFront: std::sort(clip.clippedTris[c].begin(), clip.clippedTris[c].end(), backToFront); break;
			case SortingType::FrontToBack: std::sort(clip.clippedTris[c].begin(), clip.clippedTris[c].end(), frontToBack); break;
			case SortingType::NoSort:
			default: break;
			}
			
			//_pixelMode->Rect(clip.clips[c], game::Colors::Yellow);
			Render(clip.clippedTris[c], clip.clips[c]);
			fenceCount++;
		}
		Fence(fenceCount);
		_trianglesToRender.clear();
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

	inline void Software3D::_Render(const std::vector<Triangle>& tris, const Recti& __restrict clip) noexcept
	{
		std::function<void(Triangle)> renderer;
		std::function<void(Triangle, Recti)> renderer2 = std::bind(&Software3D::Rasterize<true, true, true, true>, this, std::placeholders::_1, std::placeholders::_2);;
														
		// lit variants
		// alpha blend variants
		// alpha test variants
		// tint variants

		if (_enableLighting && _enableTexturing)
		{
			switch (_fillMode)
			{
			case game::FillMode::WireFrameFilled: renderer = std::bind(&Software3D::Rasterize<true, true, true, true>, this, std::placeholders::_1, std::cref(clip)); break;
			case game::FillMode::WireFrame: renderer = std::bind(&Software3D::Rasterize<true, false, true, true>, this, std::placeholders::_1, std::cref(clip)); break;
			case game::FillMode::Filled: renderer = std::bind(&Software3D::Rasterize<false, true, true, true>, this, std::placeholders::_1, std::cref(clip)); break;
			default: break;
			}
		}
		else if (!_enableLighting && _enableTexturing)
		{
			switch (_fillMode)
			{
			case game::FillMode::WireFrameFilled: renderer = std::bind(&Software3D::Rasterize<true, true, false, true>, this, std::placeholders::_1, std::cref(clip)); break;
			case game::FillMode::WireFrame: renderer = std::bind(&Software3D::Rasterize<true, false, false, true>, this, std::placeholders::_1, std::cref(clip)); break;
			case game::FillMode::Filled: renderer = std::bind(&Software3D::Rasterize<false, true, false, true>, this, std::placeholders::_1, std::cref(clip)); break;
			default: break;
			}
		}
		else if (_enableLighting && !_enableTexturing)
		{
			switch (_fillMode)
			{
			case game::FillMode::WireFrameFilled: renderer = std::bind(&Software3D::Rasterize<true, true, true, false>, this, std::placeholders::_1, std::cref(clip)); break;
			case game::FillMode::WireFrame: renderer = std::bind(&Software3D::Rasterize<true, false, true, false>, this, std::placeholders::_1, std::cref(clip)); break;
			case game::FillMode::Filled: renderer = std::bind(&Software3D::Rasterize<false, true, true, false>, this, std::placeholders::_1, std::cref(clip)); break;
			default: break;
			}
		}
		else if (!_enableLighting && !_enableTexturing)
		{
			switch (_fillMode)
			{
			case game::FillMode::WireFrameFilled: renderer = std::bind(&Software3D::Rasterize<true, true, false, false>, this, std::placeholders::_1, std::cref(clip)); break;
			case game::FillMode::WireFrame: renderer = std::bind(&Software3D::Rasterize<true, false, false, false>, this, std::placeholders::_1, std::cref(clip)); break;
			case game::FillMode::Filled: renderer = std::bind(&Software3D::Rasterize<false, true, false, false>, this, std::placeholders::_1, std::cref(clip)); break;
			default: break;
			}
		}

		const uint64_t trisSize = tris.size();
		for (uint32_t triangleCount = 0; triangleCount < trisSize; ++triangleCount)
		{
				renderer(std::cref(tris[triangleCount]));
		}
		_fence++;
	}

	inline void Software3D::TriangleBoundingBox(Triangle& tri) const noexcept
	{
		const float_t sx1 = (tri.vertices[0].x);
		const float_t sx2 = (tri.vertices[1].x);
		const float_t sx3 = (tri.vertices[2].x);
		const float_t sy1 = (tri.vertices[0].y);
		const float_t sy2 = (tri.vertices[1].y);
		const float_t sy3 = (tri.vertices[2].y);

		tri.boundingBox.right = (int32_t)(sx1 > sx2 ? (sx1 > sx3 ? sx1 : sx3) : (sx2 > sx3 ? sx2 : sx3));
		tri.boundingBox.bottom = (int32_t)(sy1 > sy2 ? (sy1 > sy3 ? sy1 : sy3) : (sy2 > sy3 ? sy2 : sy3));
		tri.boundingBox.left = (int32_t)(sx1 < sx2 ? (sx1 < sx3 ? sx1 : sx3) : (sx2 < sx3 ? sx2 : sx3));
		tri.boundingBox.top = (int32_t)(sy1 < sy2 ? (sy1 < sy3 ? sy1 : sy3) : (sy2 < sy3 ? sy2 : sy3));
	}

	template<bool renderWireFrame, bool filled, bool lighting, bool textured>
	inline void Software3D::Rasterize(const Triangle& triangle, const Recti& __restrict clip) noexcept
	{
		uint32_t foundTriangle = {};
		const uint32_t videoBufferStride(_currentRenderTarget.size.width);

		game::Vector2f pixelOffset;

		const Vector3f oneOverW(1.0f / triangle.vertices[0].w, 1.0f / triangle.vertices[1].w, 1.0f / triangle.vertices[2].w);

		// Color parameter	
		Color colorAtPixel;
		ParameterEquation rColorParam;
		ParameterEquation gColorParam;
		ParameterEquation bColorParam;
		ParameterEquation aColorParam;
		if (filled || _enableColorTinting)
		{
			rColorParam.Set(triangle.color[0].rf * oneOverW.x, triangle.color[1].rf * oneOverW.y, triangle.color[2].rf * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);
			gColorParam.Set(triangle.color[0].gf * oneOverW.x, triangle.color[1].gf * oneOverW.y, triangle.color[2].gf * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);
			bColorParam.Set(triangle.color[0].bf * oneOverW.x, triangle.color[1].bf * oneOverW.y, triangle.color[2].bf * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);
			aColorParam.Set(triangle.color[0].af * oneOverW.x, triangle.color[1].af * oneOverW.y, triangle.color[2].af * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);
		}

		// Depth parameter 
		float_t oneOverDepthEval = {};
		ParameterEquation depthParam(oneOverW.x, oneOverW.y, oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);

		// Face normal light pre calc (directional light) can add ambient here
		float_t intensity = {};
		Vector3f lightNormal(0.0f, 0.0f, 1.0f);  // direction the light is shining to (opposite for y)
		if (lighting)
		{
			//Vector3f faceNormal(triangle.faceNormal);// (0.0f, 0.0f, 1.0f);
			//lightNormal.Normalize();
			//faceNormal.Normalize();

			if (_lightingType == LightingType::Face)
			{
				intensity = -lightNormal.Dot(triangle.faceNormal); // Should have the negative as it is left handed
				intensity = max(0.25f, intensity); //ambient here for face
			}
		}

		// Lighting parameters
		ParameterEquation vnx;
		ParameterEquation vny;
		ParameterEquation vnz;
		ParameterEquation pixelPosParam[3];
		if (lighting) // needs only vertex and point
		{
			vnx.Set(triangle.normals[0].x * oneOverW.x, triangle.normals[1].x * oneOverW.y, triangle.normals[2].x * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);
			vny.Set(triangle.normals[0].y * oneOverW.x, triangle.normals[1].y * oneOverW.y, triangle.normals[2].y * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);
			vnz.Set(triangle.normals[0].z * oneOverW.x, triangle.normals[1].z * oneOverW.y, triangle.normals[2].z * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);
			if (_lightingType == LightingType::Point)
			{
				pixelPosParam[0].Set(triangle.pixelPos[0].x * oneOverW.x, triangle.pixelPos[1].x * oneOverW.y, triangle.pixelPos[2].x * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);
				pixelPosParam[1].Set(triangle.pixelPos[0].y * oneOverW.x, triangle.pixelPos[1].y * oneOverW.y, triangle.pixelPos[2].y * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);
				pixelPosParam[2].Set(triangle.pixelPos[0].z * oneOverW.x, triangle.pixelPos[1].z * oneOverW.y, triangle.pixelPos[2].z * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);
			}
		}


		// Texture parameters
		ParameterEquation uParam;
		ParameterEquation vParam;
		if (textured)
		{
			uParam.Set(triangle.uvs[0].u * oneOverW.x, triangle.uvs[1].u * oneOverW.y, triangle.uvs[2].u * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);
			vParam.Set(triangle.uvs[0].v * oneOverW.x, triangle.uvs[1].v * oneOverW.y, triangle.uvs[2].v * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);
		}

		// Wireframe precalcs
		float_t d[3] = {};
		float_t minDistSq = {};;
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

		const int32_t bbtop = triangle.boundingBox.top;
		const int32_t bbbot = triangle.boundingBox.bottom;

		const int32_t bbleft = triangle.boundingBox.left;
		const int32_t bbright = triangle.boundingBox.right;

		uint32_t* colorBuffer = _currentRenderTarget.colorBuffer + (bbtop * videoBufferStride + bbleft);
		float_t* depthBufferPtr = _currentRenderTarget.depthBuffer + (bbtop * videoBufferStride + bbleft);
		uint32_t xLoopCount = {};

		// test
		float_t dEval = {};
		float_t uEval = {};
		float_t vEval = {};
		float_t rEval = {};
		float_t gEval = {};
		float_t bEval = {};
		float_t aEval = {};
		float_t nXEval = {};
		float_t nYEval = {};
		float_t nZEval = {};
		float_t pixelPosEvalX = {};
		float_t pixelPosEvalY = {};
		float_t pixelPosEvalZ = {};

		

		float_t  rSource = {};
		float_t  gSource = {};
		float_t  bSource = {};
		float_t  aSource = {};

		uint32_t tx = {};
		uint32_t ty = {};
		float_t upDiv = {};
		float_t vpDiv = {};

		//uint32_t color = 0;
		uint32_t rc = {};
		uint32_t gc = {};
		uint32_t bc = {};
		uint32_t ac = {};

		float_t rDest {};
		float_t gDest {};
		float_t bDest {};
		float_t aDest {};
		float_t aFinal(1.0f);

		Vector3f pixPos = {};
		Vector3f lightDir = {};
		Vector3f lightDist = {};
		float_t ad{};
		float_t attLinear{};
		float_t attQuadratic{};



		uint32_t destColor = {};

		Vector3f vertexNormalEval;

		float_t e0 = {};
		float_t e1 = {};
		float_t e2 = {};
		uint32_t firstTestOfLine(1);



		pixelOffset.y = (float_t)bbtop - 0.5f;
		for (int32_t j = bbtop; j <= bbbot; ++j)
		{
			xLoopCount = {};
			pixelOffset.y += 1;
			//if ((j % 2 == 0))  // cheap scanline effect
			//{
			//	colorBuffer += videoBufferStride - xLoopCount;
			//	depthBufferPtr += videoBufferStride - xLoopCount;
			//	continue;
			//}
			foundTriangle = {};

			firstTestOfLine = 1;
			pixelOffset.x = (float_t)bbleft - 0.5f;
			for (int32_t i = bbleft; i <= bbright; ++i)
			{
				++xLoopCount;
				pixelOffset.x += 1;// = i + 0.5f;
				if (!firstTestOfLine)
				{
					triangle.edge0.stepX(e0);
					triangle.edge1.stepX(e1);
					triangle.edge2.stepX(e2);
				}
				else
				{
					triangle.edge0.evaluate(pixelOffset.x, pixelOffset.y, e0);
					triangle.edge1.evaluate(pixelOffset.x, pixelOffset.y, e1);
					triangle.edge2.evaluate(pixelOffset.x, pixelOffset.y, e2);
					firstTestOfLine = {};
				}

				if (triangle.edge0.test(e0))
				{
					++colorBuffer;
					++depthBufferPtr;

					if (!foundTriangle)
					{
						continue;
					}
					else
					{
						break;
					}
				}
				if (triangle.edge1.test(e1))
				{
					++colorBuffer;
					++depthBufferPtr;
					if (!foundTriangle)
					{
						continue;
					}
					else
					{
						break;
					}
				}
				if (triangle.edge2.test(e2))
				{
					++colorBuffer;
					++depthBufferPtr;
					if (!foundTriangle)
					{
						continue;
					}
					else
					{
						break;
					}
				}

				// If we got here, we found the triangle for this scanline
				foundTriangle = 1;

				// depth buffer test
				if (!depthParam.first)
				{
					depthParam.stepX(dEval);
				}
				else
				{
					depthParam.evaluate(pixelOffset.x, pixelOffset.y, dEval);
				}			
				oneOverDepthEval = 1.0f / dEval;
				// Insert depth testing here, can block out all code below
				if (oneOverDepthEval < *depthBufferPtr)  // add depth testing
				{
					if (textured)
					{
						if (!_enableAlphaTest && _enableDepthWrite)
						{
							*depthBufferPtr = oneOverDepthEval;
						}
					}
					if (!textured && _enableDepthWrite)
					{
						*depthBufferPtr = oneOverDepthEval;
					}
				}
				else
				{
					if (filled)
					{
						if (textured)
						{
							uParam.stepX(uEval);
							vParam.stepX(vEval);
						}
						if (lighting)
						{
							if ((_lightingType == LightingType::Vertex) || (_lightingType == LightingType::Point))
							{
								vnx.stepX(nXEval);
								vny.stepX(nYEval);
								vnz.stepX(nZEval);
								pixelPosParam[0].stepX(pixelPosEvalX);
								pixelPosParam[1].stepX(pixelPosEvalY);
								pixelPosParam[2].stepX(pixelPosEvalZ);
							}
						}
						if (!textured || _enableColorTinting)
						{
							rColorParam.stepX(rEval);
							gColorParam.stepX(gEval);
							bColorParam.stepX(bEval);
							aColorParam.stepX(aEval);
						}
						++colorBuffer;
						++depthBufferPtr;
						continue;
					}
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
					if (textured && !filled)
					{
						if (_enableAlphaTest && _enableDepthWrite)
						{
							*depthBufferPtr = oneOverDepthEval;
						}
					}
					for (uint32_t dist = 0; dist < 3; dist++)
					{
						d[dist] = distanceFromPointToLineSq(pixelOffset.x, pixelOffset.y, yy[dist], xx[dist], xy[dist], denominator[dist]);
					}
					minDistSq = d[0] < d[1] ? (d[0] < d[2] ? d[0] : d[2]) : (d[1] < d[2] ? d[1] : d[2]);
					if (minDistSq < _wireFrameThicknessSquared)
					{
						if (textured)
						{
							if (_enableAlphaTest && _enableDepthWrite)
							{
								*depthBufferPtr = oneOverDepthEval;
							}
						}
						*colorBuffer = _wireFrameColor;// 0xFFFFFFFF;// game::Colors::White.packedARGB;

						++colorBuffer;
						++depthBufferPtr;
						continue;
					}
				}
				
				if (filled) 
				{
					if (lighting)
					{
						if (_lightingType == LightingType::Vertex)
						{
							// Vertex normal lighting
							if (!vnx.first)
							{
								vnx.stepX(nXEval);
								vny.stepX(nYEval);
								vnz.stepX(nZEval);
							}
							else
							{
								vnx.evaluate(pixelOffset.x, pixelOffset.y, nXEval);
								vny.evaluate(pixelOffset.x, pixelOffset.y, nYEval);
								vnz.evaluate(pixelOffset.x, pixelOffset.y, nZEval);
							}
							vertexNormalEval.x = nXEval * oneOverDepthEval;
							vertexNormalEval.y = nYEval * oneOverDepthEval;
							vertexNormalEval.z = nZEval * oneOverDepthEval;

							intensity = -vertexNormalEval.Dot(lightNormal);
							intensity += 0.25f; //ambient
							intensity = max(0.25f, intensity);
							intensity = min(intensity, 1.0f);
						}
						else if ((_lightingType == LightingType::Point))
						{
							if (vnx.first == 0)
							{
								vnx.stepX(nXEval);
								vny.stepX(nYEval);
								vnz.stepX(nZEval);
								pixelPosParam[0].stepX(pixelPosEvalX);
								pixelPosParam[1].stepX(pixelPosEvalY);
								pixelPosParam[2].stepX(pixelPosEvalZ);
							}
							else
							{
								//vnx.evaluate(pixelOffset.x, pixelOffset.y, nXEval);
								//vny.evaluate(pixelOffset.x, pixelOffset.y, nYEval);
								//vnz.evaluate(pixelOffset.x, pixelOffset.y, nZEval);
								pixelPosParam[0].evaluate(pixelOffset.x, pixelOffset.y, pixelPosEvalX);
								pixelPosParam[1].evaluate(pixelOffset.x, pixelOffset.y, pixelPosEvalY);
								pixelPosParam[2].evaluate(pixelOffset.x, pixelOffset.y, pixelPosEvalZ);
							}
							vertexNormalEval.x = nXEval * oneOverDepthEval;
							vertexNormalEval.y = nYEval * oneOverDepthEval;
							vertexNormalEval.z = nZEval * oneOverDepthEval;

							// normal mapping stuff
							if (!uParam.first)
							{
								uParam.stepX(uEval);
								vParam.stepX(vEval);
							}
							else
							{
								uParam.evaluate(pixelOffset.x, pixelOffset.y, uEval);
								vParam.evaluate(pixelOffset.x, pixelOffset.y, vEval);
							}
							upDiv = uEval * oneOverDepthEval;
							vpDiv = vEval * oneOverDepthEval;
							// calculate texture lookup
							upDiv = min(upDiv, 1.0f); //clamp
							vpDiv = min(vpDiv, 1.0f); //clamp

							upDiv = max(upDiv, 0.0f);  // something is causing a negative value
							vpDiv = max(vpDiv, 0.0f);  // so these are here

							tx = max((uint32_t)(upDiv * (_currentNormalMap.sizeMinusOne.width) + 0.5f), 0);	// -1 fix texture seams at max texW and texH
							ty = max((uint32_t)(vpDiv * (_currentNormalMap.sizeMinusOne.height) + 0.5f), 0);
							uint32_t normPack = _currentNormalMap.data[ty * _currentTexture.size.width + tx];


							vertexNormalEval.x = (((normPack >> 0) & 0xFF) * (1.0f / 255.0f)) * 2.0f - 1.0f;
							vertexNormalEval.y = (((normPack >> 8) & 0xFF) * (1.0f / 255.0f)) * 2.0f - 1.0f;
							vertexNormalEval.z = -(((normPack >> 16) & 0xFF) * (1.0f / 255.0f)) * 2.0f - 1.0f;
							vertexNormalEval.Normalize(); // needs? yes
							// --- end normal mapping stuff

							// how to add lights
							//vec3 lightDir = normalize(light.position - fragPos);
							//// diffuse shading
							//float diff = max(dot(normal, lightDir), 0.0);
							//// specular shading
							//vec3 reflectDir = reflect(-lightDir, normal);
							// ret reflect(i, n)
							// v = i - 2 * n * dot(i n) 
							//float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
							//// attenuation
							//float distance = length(light.position - fragPos);
							//float attenuation = 1.0 / (light.constant + light.linear * distance +
							//	light.quadratic * (distance * distance));
							//// combine results
							//vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords)); <------------
							//vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
							//vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
							//ambient *= attenuation;
							//diffuse *= attenuation;
							//specular *= attenuation;
							//return (ambient + diffuse + specular); <-----------------

							// Get direction of pixel to light
							pixPos = { pixelPosEvalX * oneOverDepthEval,pixelPosEvalY * oneOverDepthEval,pixelPosEvalZ * oneOverDepthEval };
							lightDir = pixPos - lights[0].position; // swapped instead of negating
							intensity = 0.25f;  // ambient
							const float_t ld2 = lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z;
							if (ld2 < lights[0].radius)
							{
								lightDist = lightDir;
								lightDir.Normalize();

								// Diffuse calc
								intensity = -vertexNormalEval.Dot(lightDir);
								if (intensity > 0)
								{
									intensity += 0.25f; // ambient
									// falloff
									//diffuseLighting *= ((length(lightDir) * length(lightDir)) / dot(light.Position - Input.WorldPosition, light.Position - Input.WorldPosition));
									//                   lightNormal * lightNormal / (lightpos-pixelpos.dot(lightpos-pixelpos) this last part is squaring
									// Attenuation calc 
									//ad = lightDist.x* lightDist.x + lightDist.y * lightDist.y + lightDist.z * lightDist.z;//Mag2();// (lights[0].position - pixPos).Dot(lights[0].position - pixPos);// e.Mag();
									attLinear = lights[0].attenuation.linear * ld2;// ad;
									attQuadratic = lights[0].attenuation.quadratic * ld2 * ld2;// ad* ad;
									//ad = lights[0].attenuation.constant + attLin + attExp;

									//intensity *= pow(1.0f / ad,2.2f);
									//intensity *= 1.0f / ad;
									intensity *= 1.0f / (lights[0].attenuation.constant + attLinear + attQuadratic);
									//std::cout << lightDist.Mag2() << "\n";
								}
								intensity = max(intensity, 0.25f); // ambient
								intensity = min(intensity, 1.0f);
							}
					
						}
						else if (_lightingType == LightingType::Depth)
						{
							//Depth based lighting color
							intensity = oneOverDepthEval + 1.0f;
							intensity = 1.0f / intensity;
							intensity += 0.25f; // simulate ambient ?
							intensity = min(intensity, 1.0f);
						}
					}

					// Just colored
					if (!textured)
					{
						if (!rColorParam.first)
						{
							rColorParam.stepX(rEval);
							gColorParam.stepX(gEval);
							bColorParam.stepX(bEval);
							aColorParam.stepX(aEval);
						}
						else
						{
							rColorParam.evaluate(pixelOffset.x, pixelOffset.y, rEval);
							gColorParam.evaluate(pixelOffset.x, pixelOffset.y, gEval);
							bColorParam.evaluate(pixelOffset.x, pixelOffset.y, bEval);
							aColorParam.evaluate(pixelOffset.x, pixelOffset.y, aEval);
						}

						rSource = min(rEval * oneOverDepthEval, 1.0f);
						gSource = min(gEval * oneOverDepthEval, 1.0f);
						bSource = min(bEval * oneOverDepthEval, 1.0f);
						aSource = min(aEval * oneOverDepthEval, 1.0f);

						if (lighting)
						{
							rSource = (rSource + lights[0].diffuse.rf) * 0.5f * intensity; // light color
							gSource = (gSource + lights[0].diffuse.gf) * 0.5f * intensity;
							bSource = (bSource + lights[0].diffuse.bf) * 0.5f * intensity;
						}

						// alpha blending
						if (_enableAlphaBlend) 
						{
							destColor = *colorBuffer;
							// extract dest						
							rDest = ((destColor >> 0) & 0xFF) * (1.0f / 255.0f);
							gDest = ((destColor >> 8) & 0xFF) * (1.0f / 255.0f);
							bDest = ((destColor >> 16) & 0xFF) * (1.0f / 255.0f);
							aDest = ((destColor >> 24) & 0xFF) * (1.0f / 255.0f);
							aFinal = 1.0f - (1.0f - aSource) * (1.0f - aDest);
							
							// fg.R * fg.A / r.A + bg.R * bg.A * (1 - fg.A) / r.A;
							float_t adnewa = aSource / aFinal;
							float_t da1minadnewa = aDest * (1.0f - aSource) / aFinal;
							//rd = rd * ad / newa + dr * da * (1.0f - ad) / newa;
							//gd = gd * ad / newa + dg * da * (1.0f - ad) / newa;
							//bd = bd * ad / newa + db * da * (1.0f - ad) / newa;
							rSource = rSource * adnewa + rDest * da1minadnewa;
							gSource = gSource * adnewa + gDest * da1minadnewa;
							bSource = bSource * adnewa + bDest * da1minadnewa;
						}
						*colorBuffer = (uint32_t(aFinal * 255) << 24) | (uint32_t(bSource * 255) << 16) | (uint32_t(gSource * 255) << 8) | uint32_t(rSource * 255); 
						//colorAtPixel.packedABGR;
					}

					
					if (textured)
					{
						// test for normal mapping
						//if (!lighting)
						if (!lighting || (_lightingType != LightingType::Point))
						{
							if (!uParam.first)
							{
								uParam.stepX(uEval);
								vParam.stepX(vEval);
							}
							else
							{
								uParam.evaluate(pixelOffset.x, pixelOffset.y, uEval);
								vParam.evaluate(pixelOffset.x, pixelOffset.y, vEval);
							}
							upDiv = uEval * oneOverDepthEval;
							vpDiv = vEval * oneOverDepthEval;
							// calculate texture lookup
							upDiv = min(upDiv, 1.0f); //clamp
							vpDiv = min(vpDiv, 1.0f); //clamp

							upDiv = max(upDiv, 0.0f);  // something is causing a negative value
							vpDiv = max(vpDiv, 0.0f);  // so these are here

						}
						tx = max((uint32_t)(upDiv * (_currentTexture.sizeMinusOne.width) + 0.5f), 0);	// -1 fix texture seams at max texW and texH
						ty = max((uint32_t)(vpDiv * (_currentTexture.sizeMinusOne.height) + 0.5f), 0);
						colorAtPixel.packedABGR = _currentTexture.data[ty * _currentTexture.size.width + tx];

						rSource = ((colorAtPixel.packedABGR >> 0) & 0xFF) * (1.0f / 255.0f);
						gSource = ((colorAtPixel.packedABGR >> 8) & 0xFF) * (1.0f / 255.0f);
						bSource = ((colorAtPixel.packedABGR >> 16) & 0xFF) * (1.0f / 255.0f);
						aSource = ((colorAtPixel.packedABGR >> 24) & 0xFF) * (1.0f / 255.0f);

						if (_enableAlphaTest)
						{							
							if (aSource * 255 < _alphaTestValue)
							{
								++colorBuffer;
								++depthBufferPtr;
								rColorParam.stepX(rEval);
								gColorParam.stepX(gEval);
								bColorParam.stepX(bEval);
								aColorParam.stepX(aEval);
								continue;
							}
							else
							{
								if (_enableDepthWrite)
								{
									*depthBufferPtr = oneOverDepthEval;
								}

							}
						}

						if (_enableColorTinting)
						{
							if (!rColorParam.first)
							{
								rColorParam.stepX(rEval);
								gColorParam.stepX(gEval);
								bColorParam.stepX(bEval);
								aColorParam.stepX(aEval);
							}
							else
							{
								rColorParam.evaluate(pixelOffset.x, pixelOffset.y, rEval);
								gColorParam.evaluate(pixelOffset.x, pixelOffset.y, gEval);
								bColorParam.evaluate(pixelOffset.x, pixelOffset.y, bEval);
								aColorParam.evaluate(pixelOffset.x, pixelOffset.y, aEval);
							}
							// Apply color tint
							rSource = ((rSource * min(rEval * oneOverDepthEval, 1.0f)));
							gSource = ((gSource * min(gEval * oneOverDepthEval, 1.0f)));
							bSource = ((bSource * min(bEval * oneOverDepthEval, 1.0f)));
							aSource = ((aSource * min(aEval * oneOverDepthEval, 1.0f)));
						}

						// texture lighting
						if (lighting) // this may need to go to end
						{
							rSource = (rSource + lights[0].diffuse.rf) * 0.5f; // color light
							gSource = (gSource + lights[0].diffuse.gf) * 0.5f;
							bSource = (bSource + lights[0].diffuse.bf) * 0.5f;
							rSource *= intensity;
							gSource *= intensity;
							bSource *= intensity;
						}

						if (_enableAlphaBlend) 
						{
							destColor = *colorBuffer;
							// extract dest
							rDest = ((destColor >> 0) & 0xFF) * (1.0f / 255.0f);
							gDest = ((destColor >> 8) & 0xFF) * (1.0f / 255.0f);
							bDest = ((destColor >> 16) & 0xFF) * (1.0f / 255.0f);
							aDest = ((destColor >> 24) & 0xFF) * (1.0f / 255.0f);

							aFinal = 1.0f - (1.0f - aSource) * (1.0f - aDest);
							// fg.R * fg.A / r.A + bg.R * bg.A * (1 - fg.A) / r.A;
							float_t adnewa = aSource / aFinal;
							float_t da1minadnewa = aDest * (1.0f - aSource) / aFinal;
							//rd = rd * ad / newa + dr * da * (1.0f - ad) / newa;
							//gd = gd * ad / newa + dg * da * (1.0f - ad) / newa;
							//bd = bd * ad / newa + db * da * (1.0f - ad) / newa;
							rSource = rSource * adnewa + rDest * da1minadnewa;
							gSource = gSource * adnewa + gDest * da1minadnewa;
							bSource = bSource * adnewa + bDest * da1minadnewa;
						}
						//rSource = pow(rSource, 1.0f / 2.2f);
						//gSource = pow(gSource, 1.0f / 2.2f);
						//bSource = pow(bSource, 1.0f / 2.2f);
						*colorBuffer = (uint32_t(aFinal * 255)<< 24) | (uint32_t(bSource * 255) << 16) | (uint32_t(gSource * 255) << 8) | uint32_t(rSource * 255);// colorAtPixel.packedABGR;// color;// _currentTexture.data[ty * _currentTexture.size.width + tx];
					}
				}
				++colorBuffer;
				++depthBufferPtr;
			}
			depthParam.first = 1;
			if (filled || _enableColorTinting)
			{
				rColorParam.first = 1;
				//gColorParam.first = 1;
				//bColorParam.first = 1;
				//aColorParam.first = 1;
			}
			if (textured)
			{
				uParam.first = 1;
				//vParam.first = 1;
			}
			if (lighting)
			{
				vnx.first = 1;
				//vny.first = 1;
				//vnz.first = 1;
				//pixelPosParam[0].first = 1;
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
		const uint64_t inSize = in.size();
		for (uint32_t tri = 0; tri < inSize; ++tri)
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

			if (!in[tri].boundingCalculated)
			{
				TriangleBoundingBox(in[tri]);
				in[tri].boundingCalculated = true;
			}
			// Screen clipping
			// Offscreen completely
			if ((in[tri].boundingBox.right < 0) || (in[tri].boundingBox.left > _defaultRenderTarget.size.width-1) ||
				(in[tri].boundingBox.bottom < 0) || (in[tri].boundingBox.top > _defaultRenderTarget.size.height-1))
			{
				in[tri].backFaceCulled = true;
				continue;
			}

			// Outside clipping rect completely // by itself 336
			if ((in[tri].boundingBox.right < clip.clips[index].left) || (in[tri].boundingBox.left > clip.clips[index].right) ||
				(in[tri].boundingBox.bottom < clip.clips[index].top) || (in[tri].boundingBox.top > clip.clips[index].bottom))
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
					if (_enableBackFaceCulling)
					{
						in[tri].backFaceCulled = true;
						continue;
					}
					else
					{
						// Back face culling off, need to change winding
						std::swap(in[tri].vertices[1], in[tri].vertices[0]);
						std::swap(in[tri].normals[1], in[tri].normals[0]);
						std::swap(in[tri].uvs[1], in[tri].uvs[0]);
						std::swap(in[tri].color[1], in[tri].color[0]);
						in[tri].edge0.Set(in[tri].vertices[1], in[tri].vertices[2]);
						in[tri].edge1.Set(in[tri].vertices[2], in[tri].vertices[0]);
						in[tri].edge2.Set(in[tri].vertices[0], in[tri].vertices[1]);
						in[tri].area = 1.0f / (in[tri].edge0.c + in[tri].edge1.c + in[tri].edge2.c);
					}
				}
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
		const uint32_t meshSize = (uint32_t)mesh.tris.size();
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


	inline void Vector3MultMatrix4x4(const Vector3f & __restrict vector, const Matrix4x4f& __restrict mat, Vector3f& __restrict out) noexcept
	{
		//Vector3f ret;
		out.x = vector.x * mat.m[0] + vector.y * mat.m[4] + vector.z * mat.m[8] + vector.w * mat.m[12];
		out.y = vector.x * mat.m[1] + vector.y * mat.m[5] + vector.z * mat.m[9] + vector.w * mat.m[13];
		out.z = vector.x * mat.m[2] + vector.y * mat.m[6] + vector.z * mat.m[10] + vector.w * mat.m[14];
		out.w = vector.x * mat.m[3] + vector.y * mat.m[7] + vector.z * mat.m[11] + vector.w * mat.m[15];
		//vector = ret;
	}

	inline void Software3D::VertexProcessor(game::Mesh& mesh, const uint64_t numberOfTris, const game::Matrix4x4f& __restrict mvp, std::vector<game::Triangle>& processedTris, Camera3D& camera) const noexcept
	{
		mesh.GenerateModelMatrix();
		Matrix4x4f mvpCopy(mvp);
		mvpCopy = mvpCopy * mesh.model;
		game::Triangle workingTriangle;


		// Point light stuff
		if (_enableLighting && (_lightingType == LightingType::Point))
		{
			for (uint32_t i = 0; i < numberOfTris; ++i)
			{
				mesh.tris[i].pixelPos[0] = mesh.tris[i].vertices[0] * mesh.model;
				mesh.tris[i].pixelPos[1] = mesh.tris[i].vertices[1] * mesh.model;
				mesh.tris[i].pixelPos[2] = mesh.tris[i].vertices[2] * mesh.model;
			}
		}



		game::Triangle newClippedTris[2];
		uint32_t numberTrisGenerated = {};
		//int culled = 0;
		uint32_t changeWinding = {};

		Vector3f cameraRay; 
		for (uint32_t i = 0; i < numberOfTris; i++)
		{
			changeWinding = {};
			workingTriangle = mesh.tris[i];

			// Backface cull before clip, there ARE artifacts when scaling non uniformly or if scale is negative
			//workingTriangle.faceNormal = workingTriangle.faceNormal * mesh.rotation;
			Vector3MultMatrix4x4(mesh.tris[i].faceNormal, mesh.rotation, workingTriangle.faceNormal);
			//cameraRay = (workingTriangle.vertices[0] * mesh.model) - camera.position;
			Vector3MultMatrix4x4(mesh.tris[i].vertices[0], mesh.model, cameraRay);
			cameraRay -= camera.position;  

			if (workingTriangle.faceNormal.Dot(cameraRay) >= 0.0f)
			{
				if (_enableBackFaceCulling)
				{
					//culled++;
					continue;
				}
				else
				{
					changeWinding = 1;
				}
			} 



			//workingTriangle.vertices[0] = (workingTriangle.vertices[0] * mvpCopy);
			//workingTriangle.vertices[1] = (workingTriangle.vertices[1] * mvpCopy);
			//workingTriangle.vertices[2] = (workingTriangle.vertices[2] * mvpCopy);
			Vector3MultMatrix4x4(mesh.tris[i].vertices[0], mvpCopy, workingTriangle.vertices[0]);
			Vector3MultMatrix4x4(mesh.tris[i].vertices[1], mvpCopy, workingTriangle.vertices[1]);
			Vector3MultMatrix4x4(mesh.tris[i].vertices[2], mvpCopy, workingTriangle.vertices[2]);

			//workingTriangle.normals[0] = workingTriangle.normals[0] * mesh.rotation;
			//workingTriangle.normals[1] = workingTriangle.normals[1] * mesh.rotation;
			//workingTriangle.normals[2] = workingTriangle.normals[2] * mesh.rotation;
			Vector3MultMatrix4x4(mesh.tris[i].normals[0], mesh.rotation, workingTriangle.normals[0]);
			Vector3MultMatrix4x4(mesh.tris[i].normals[1], mesh.rotation, workingTriangle.normals[1]);
			Vector3MultMatrix4x4(mesh.tris[i].normals[2], mesh.rotation, workingTriangle.normals[2]);

			// Color based on normals
			//Color normC;
			//workingTriangle.faceNormal.Normalize();
			//normC.Set(workingTriangle.faceNormal.x * 0.5f + 0.5f, workingTriangle.faceNormal.y * 0.5f + 0.5f, workingTriangle.faceNormal.z * 0.5f + 0.5f, 1.0f);
			//workingTriangle.color[0] = normC;
			//workingTriangle.color[1] = normC;
			//workingTriangle.color[2] = normC;


			if ((workingTriangle.vertices[0].z < 0.1) ||
				(workingTriangle.vertices[1].z < 0.1) ||
				(workingTriangle.vertices[2].z < 0.1))
			{
				numberTrisGenerated = ClipAgainstNearZ(workingTriangle, newClippedTris[0], newClippedTris[1]);
				for (uint32_t tri = 0; tri < numberTrisGenerated; ++tri)
				{
					// Perspective divide
					newClippedTris[tri].vertices[0] /= newClippedTris[tri].vertices[0].w;
					newClippedTris[tri].vertices[1] /= newClippedTris[tri].vertices[1].w;
					newClippedTris[tri].vertices[2] /= newClippedTris[tri].vertices[2].w;

					ScaleToScreen(newClippedTris[tri], _currentRenderTarget.halfSize);

					// New triangles can be generated "backwards"
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
				if (changeWinding)
				{
					std::swap(workingTriangle.vertices[1], workingTriangle.vertices[0]);
					std::swap(workingTriangle.normals[1], workingTriangle.normals[0]);
					std::swap(workingTriangle.uvs[1], workingTriangle.uvs[0]);
					std::swap(workingTriangle.color[1], workingTriangle.color[0]);
				}
				// Perspective Divide
				workingTriangle.vertices[0] /= workingTriangle.vertices[0].w;
				workingTriangle.vertices[1] /= workingTriangle.vertices[1].w;
				workingTriangle.vertices[2] /= workingTriangle.vertices[2].w;


				ScaleToScreen(workingTriangle, _currentRenderTarget.halfSize);
				processedTris.emplace_back(workingTriangle);
			}
		}
		//std::cout << culled << "\n";
	}


	// Insert vertices in the order of your triangle
	inline Vector3f GenerateFaceNormal(game::Vector3f& __restrict A, game::Vector3f& __restrict B, game::Vector3f& __restrict C) noexcept
	{
		const Vector3f AC = C - A; // Vector from A to C
		const Vector3f AB = B - A; // Vector from A to B
		Vector3f N = AC.Cross(AB); // Cross product of AB and AC
		N.Normalize();
		return N;
	}

	// Insert vertices in the order of your triangle
	inline void GenerateFaceNormal(game::Vector3f& __restrict A, game::Vector3f& __restrict B, game::Vector3f& __restrict C, game::Vector3f& __restrict out) noexcept
	{
		const Vector3f AC = C - A; // Vector from A to C
		const Vector3f AB = B - A; // Vector from A to B
		out = AC.Cross(AB); // Cross product of AB and AC
		out.Normalize();
	}

	// Function to create a UV sphere
	// Slices needs to be even number for uvs to map correctly
	inline void GenerateUVSphere(game::Mesh& mesh, const uint32_t stacks, const uint32_t slices, const game::Vector3f& __restrict pos, const game::Color color)
	{
		mesh.tris.clear();
		game::Vector3f v1, v2, v3, v4;
		const float_t invStacks = 1.0f / (float_t)stacks * 3.14159f;
		const float_t invSlice = 1.0f / (float_t)slices * 2.0f * 3.14159f;
		game::Triangle tri;

		tri.color[0] = color;
		tri.color[1] = color;
		tri.color[2] = color;


		for (uint32_t t = 0; t < stacks; ++t)
		{
			const float_t theta1 = static_cast<float>(t) * invStacks;
			const float_t theta2 = static_cast<float>(t + 1) * invStacks;
			const float_t cosTheta1 = cos(theta1);
			const float_t sinTheta1 = sin(theta1);
			const float_t cosTheta2 = cos(theta2);
			const float_t sinTheta2 = sin(theta2);

			for (uint32_t p = 0; p < slices; ++p)
			{
				const float_t phi1 = static_cast<float>(p) * invSlice;
				const float_t phi2 = static_cast<float>(p + 1) * invSlice;
				const float_t cosPhi1 = cos(phi1);
				const float_t sinPhi1 = sin(phi1);
				const float_t cosPhi2 = cos(phi2);
				const float_t sinPhi2 = sin(phi2);


				// Calculate vertex positions for each quad				
				v1.x = sinTheta1 * cosPhi1;
				v1.z = sinTheta1 * sinPhi1;
				v1.y = cosTheta1;

				v2.x = sinTheta1 * cosPhi2;
				v2.z = sinTheta1 * sinPhi2;
				v2.y = cosTheta1;

				v3.x = sinTheta2 * cosPhi1;
				v3.z = sinTheta2 * sinPhi1;
				v3.y = cosTheta2;

				v4.x = sinTheta2 * cosPhi2;
				v4.z = sinTheta2 * sinPhi2;
				v4.y = cosTheta2;

				// UV mapping equation from
				// https://en.wikipedia.org/wiki/UV_mapping

				game::Color color;

				// First triangle of quad
				tri.vertices[0] = v1 + pos; //bl
				tri.normals[0] = v1;
				tri.uvs[0].u = atan2f(v1.z, v1.x) / (2.0f * 3.14159f) + 0.5f;
				tri.uvs[0].v = asin(v1.y) / 3.14159f + 0.5f;
				//color.Set(tri.uvs[0].u, tri.uvs[0].u, tri.uvs[0].u, 255);
				//tri.color[0] = color;

				tri.vertices[1] = v3 + pos; // tl
				tri.normals[1] = v3;
				tri.uvs[1].u = atan2f(v3.z, v3.x) / (2.0f * 3.14159f) + 0.5f;
				tri.uvs[1].v = asin(v3.y) / 3.14159f + 0.5f;
				//color.Set(tri.uvs[1].u, tri.uvs[1].u, tri.uvs[1].u, 255);
				//tri.color[1] = color;

				tri.vertices[2] = v2 + pos; //br
				tri.normals[2] = v2;
				tri.uvs[2].u = atan2f(v2.z, v2.x) / (2.0f * 3.14159f) + 0.5f;
				tri.uvs[2].v = asin(v2.y) / 3.14159f + 0.5f;
				//color.Set(tri.uvs[2].u, tri.uvs[2].u, tri.uvs[2].u, 255);
				//tri.color[2] = color;

				// This may be cutting off part of texture??
				if (tri.uvs[0].u == 1.0f)
				{
					tri.uvs[0].u = 0.0f;
					//color.Set(tri.uvs[0].u, tri.uvs[0].u, tri.uvs[0].u, 255);
					//tri.color[0] = color;
					tri.uvs[1].u = 0.0f;
					//color.Set(tri.uvs[1].u, tri.uvs[1].u, tri.uvs[1].u, 255);
					//tri.color[1] = color;
				}

				GenerateFaceNormal(v1, v3, v2, tri.faceNormal);

				mesh.tris.emplace_back(tri);
				if ((t != stacks - 1) || (t == 0))
				{
					// Second triangle of quad
					tri.vertices[0] = v2 + pos; //br
					tri.normals[0] = v2;
					tri.uvs[0].u = atan2f(v2.z, v2.x) / (2.0f * 3.14159f) + 0.5f;
					tri.uvs[0].v = asin(v2.y) / 3.14159f + 0.5f;
					//color.Set(tri.uvs[0].u, tri.uvs[0].u, tri.uvs[0].u, 255);
					//tri.color[0] = color;

					tri.vertices[1] = v3 + pos; // tl
					tri.normals[1] = v3;
					tri.uvs[1].u = atan2f(v3.z, v3.x) / (2.0f * 3.14159f) + 0.5f;
					tri.uvs[1].v = asin(v3.y) / 3.14159f + 0.5f;
					//color.Set(tri.uvs[1].u, tri.uvs[1].u, tri.uvs[1].u, 255);
					//tri.color[1] = color;

					tri.vertices[2] = v4 + pos; //tr
					tri.normals[2] = v4;
					tri.uvs[2].u = atan2f(v4.z, v4.x) / (2.0f * 3.14159f) + 0.5f;
					tri.uvs[2].v = asin(v4.y) / 3.14159f + 0.5f;
					//color.Set(tri.uvs[2].u, tri.uvs[2].u, tri.uvs[2].u, 255);
					//tri.color[2] = color;


					if (tri.uvs[1].u > tri.uvs[2].u)
					{
						tri.uvs[1].u = 0.0f;
						//color.Set(tri.uvs[1].u, tri.uvs[1].u, tri.uvs[1].u, 255);
						//tri.color[1] = color;
					}

					GenerateFaceNormal(v2, v3, v4, tri.faceNormal);

					mesh.tris.emplace_back(tri);
				}

			}
		}
	}
	
	// Subdivides a plane by resolution x resolution, texture is stretched across the plane
	inline void GeneratePlane(game::Mesh& mesh, const game::Vector3f& __restrict pos, const uint32_t resolution, const game::Color color)
	{
		mesh.tris.clear();
		static game::Vector3f oldNorm;
		const float_t subdivisionSize = 1.0f / (float)resolution;

		const float z = pos.z;
		const float size = 0.5f * subdivisionSize;
		const game::Vector3f normal = { 0,0,-1 };
		game::Triangle topLeftTri;
		game::Triangle bottomRightTri;
		const game::Vector3f t = { size * (resolution - 1),size * (resolution - 1),0 };

		for (float_t y = 0; y < 1.0f; y += subdivisionSize)
		{
			for (float_t x = 0; x < 1.0f; x += subdivisionSize)
			{

				// tl
				topLeftTri.vertices[0].x = -size + x + pos.x;
				topLeftTri.vertices[0].y = -size + y + pos.y;
				topLeftTri.vertices[0].z = z;
				topLeftTri.color[0] = game::Colors::White;
				topLeftTri.uvs[0].u = x;// 0.0f;
				topLeftTri.uvs[0].v = y;// 0.0f;
				topLeftTri.faceNormal = normal;
				topLeftTri.normals[0] = normal;

				// tr
				topLeftTri.vertices[1].x = size + x + pos.x;
				topLeftTri.vertices[1].y = -size + y + pos.y;
				topLeftTri.vertices[1].z = z;
				topLeftTri.uvs[1].u = x + subdivisionSize; // 1.0f
				topLeftTri.uvs[1].v = y;// 0.0f;
				topLeftTri.color[1] = game::Colors::White; // game::Colors::Green;
				topLeftTri.normals[1] = normal;

				// bl
				topLeftTri.vertices[2].x = -size + x + pos.x;
				topLeftTri.vertices[2].y = size + y + pos.y;
				topLeftTri.vertices[2].z = z;
				topLeftTri.uvs[2].u = x;// 0.0f;
				topLeftTri.uvs[2].v = y + subdivisionSize;// 1.0f;
				topLeftTri.color[2] = game::Colors::White; //game::Colors::Blue;
				topLeftTri.normals[2] = normal;



				// tr
				bottomRightTri.vertices[0].x = size + x + pos.x;
				bottomRightTri.vertices[0].y = -size + y + pos.y;
				bottomRightTri.vertices[0].z = z;
				bottomRightTri.uvs[0].u = x + subdivisionSize;// 1.0f;
				bottomRightTri.uvs[0].v = y;// 0.0f;
				bottomRightTri.color[0] = game::Colors::White; //game::Colors::Green;
				bottomRightTri.normals[0] = normal;
				bottomRightTri.faceNormal = normal;

				// br
				bottomRightTri.vertices[1].x = size + x + pos.x;
				bottomRightTri.vertices[1].y = size + y + pos.y;
				bottomRightTri.vertices[1].z = z;
				bottomRightTri.uvs[1].u = x + subdivisionSize;// 1.0f;
				bottomRightTri.uvs[1].v = y + subdivisionSize;// 1.0f;
				bottomRightTri.color[1] = game::Colors::White;
				bottomRightTri.normals[1] = normal;

				// bl
				bottomRightTri.vertices[2].x = -size + x + pos.x;
				bottomRightTri.vertices[2].y = size + y + pos.y;
				bottomRightTri.vertices[2].z = z;
				bottomRightTri.uvs[2].u = x;// 0.0f;
				bottomRightTri.uvs[2].v = y + subdivisionSize;// 1.0f;
				bottomRightTri.color[2] = game::Colors::White; //game::Colors::Blue;
				bottomRightTri.normals[2] = normal;

				for (int e = 0; e < 3; ++e)
				{

					topLeftTri.vertices[e] = topLeftTri.vertices[e] - t;
					bottomRightTri.vertices[e] = bottomRightTri.vertices[e] - t;
				}


				mesh.tris.emplace_back(topLeftTri);
				mesh.tris.emplace_back(bottomRightTri);
			}
		}
	}

	inline void GenerateCube(game::Mesh& mesh, const game::Vector3f& __restrict pos, const game::Color& color)  noexcept
	{
		mesh.tris.clear();
		const float_t size = 0.5f;
		game::Vector3f ftl;
		game::Vector3f ftr;
		game::Vector3f fbl;
		game::Vector3f fbr;

		game::Vector3f btl;
		game::Vector3f btr;
		game::Vector3f bbl;
		game::Vector3f bbr;

		// Front
		ftl.x = -size;
		ftl.y = -size;
		ftl.z = -size;
		ftl += pos;

		ftr.x = size;
		ftr.y = -size;
		ftr.z = -size;
		ftr += pos;

		fbl.x = -size;
		fbl.y = size;
		fbl.z = -size;
		fbl += pos;

		fbr.x = size;
		fbr.y = size;
		fbr.z = -size;
		fbr += pos;

		// Back
		btl.x = -size;
		btl.y = -size;
		btl.z = size;
		btl += pos;

		btr.x = size;
		btr.y = -size;
		btr.z = size;
		btr += pos;

		bbl.x = -size;
		bbl.y = size;
		bbl.z = size;
		bbl += pos;

		bbr.x = size;
		bbr.y = size;
		bbr.z = size;
		bbr += pos;

		game::Triangle f;
		f.color[0] = color;
		f.color[1] = color;
		f.color[2] = color;


		// Front
		f.vertices[0] = ftl;
		f.uvs[0].u = 0.0f;
		f.uvs[0].v = 0.0f;
		f.vertices[1] = ftr;
		f.uvs[1].u = 1.0f;
		f.uvs[1].v = 0.0f;
		f.vertices[2] = fbl;
		f.uvs[2].u = 0.0f;
		f.uvs[2].v = 1.0f;
		f.faceNormal = { 0.0f,0.0f,-1.0f };
		f.normals[0] = f.faceNormal;
		f.normals[1] = f.faceNormal;
		f.normals[2] = f.faceNormal;
		mesh.tris.emplace_back(f);

		f.vertices[0] = ftr;
		f.uvs[0].u = 1.0f;
		f.uvs[0].v = 0.0f;
		f.vertices[1] = fbr;
		f.uvs[1].u = 1.0f;
		f.uvs[1].v = 1.0f;
		f.vertices[2] = fbl;
		f.uvs[2].u = 0.0f;
		f.uvs[2].v = 1.0f;
		//f.faceNormal = { 0.0f,0.0f,-1.0f };
		mesh.tris.emplace_back(f);

		// Back
		f.vertices[0] = btr;
		f.uvs[0].u = 0.0f;
		f.uvs[0].v = 0.0f;
		f.vertices[1] = btl;
		f.uvs[1].u = 1.0f;
		f.uvs[1].v = 0.0f;
		f.vertices[2] = bbl;
		f.uvs[2].u = 1.0f;
		f.uvs[2].v = 1.0f;
		f.faceNormal = { 0.0f,0.0f,1.0f };
		f.normals[0] = f.faceNormal;
		f.normals[1] = f.faceNormal;
		f.normals[2] = f.faceNormal;
		mesh.tris.emplace_back(f);

		f.vertices[0] = bbr;
		f.uvs[0].u = 0.0f;
		f.uvs[0].v = 1.0f;
		f.vertices[1] = btr;
		f.uvs[1].u = 0.0f;
		f.uvs[1].v = 0.0f;
		f.vertices[2] = bbl;
		f.uvs[2].u = 1.0f;
		f.uvs[2].v = 1.0f;
		//f.faceNormal = { 0.0f,0.0f,1.0f };
		mesh.tris.emplace_back(f);

		// Left
		f.vertices[0] = btl;
		f.uvs[0].u = 0.0f;
		f.uvs[0].v = 0.0f;
		f.vertices[1] = ftl;
		f.uvs[1].u = 1.0f;
		f.uvs[1].v = 0.0f;
		f.vertices[2] = bbl;
		f.uvs[2].u = 0.0f;
		f.uvs[2].v = 1.0f;
		f.faceNormal = { -1.0f,0.0f,0.0f };
		f.normals[0] = f.faceNormal;
		f.normals[1] = f.faceNormal;
		f.normals[2] = f.faceNormal;
		mesh.tris.emplace_back(f);

		f.vertices[0] = ftl;
		f.uvs[0].u = 1.0f;
		f.uvs[0].v = 0.0f;
		f.vertices[1] = fbl;
		f.uvs[1].u = 1.0f;
		f.uvs[1].v = 1.0f;
		f.vertices[2] = bbl;
		f.uvs[2].u = 0.0f;
		f.uvs[2].v = 1.0f;
		//f.faceNormal = { -1.0f,0.0f,0.0f };
		mesh.tris.emplace_back(f);

		// Right
		f.vertices[0] = ftr;
		f.uvs[0].u = 0.0f;
		f.uvs[0].v = 0.0f;
		f.vertices[1] = btr;
		f.uvs[1].u = 1.0f;
		f.uvs[1].v = 0.0f;
		f.vertices[2] = bbr;
		f.uvs[2].u = 1.0f;
		f.uvs[2].v = 1.0f;
		f.faceNormal = { 1.0f,0.0f,0.0f };
		f.normals[0] = f.faceNormal;
		f.normals[1] = f.faceNormal;
		f.normals[2] = f.faceNormal;
		mesh.tris.emplace_back(f);

		f.vertices[0] = ftr;
		f.uvs[0].u = 0.0f;
		f.uvs[0].v = 0.0f;
		f.vertices[1] = bbr;
		f.uvs[1].u = 1.0f;
		f.uvs[1].v = 1.0f;
		f.vertices[2] = fbr;
		f.uvs[2].u = 0.0f;
		f.uvs[2].v = 1.0f;
		//f.faceNormal = { 1.0f,0.0f,0.0f };
		mesh.tris.emplace_back(f);

		// Top
		f.vertices[0] = ftl;
		f.uvs[0].u = 0.0f;
		f.uvs[0].v = 1.0f;
		f.vertices[1] = btl;
		f.uvs[1].u = 0.0f;
		f.uvs[1].v = 0.0f;
		f.vertices[2] = btr;
		f.uvs[2].u = 1.0f;
		f.uvs[2].v = 0.0f;
		f.faceNormal = { 0.0f,-1.0f,0.0f };
		f.normals[0] = f.faceNormal;
		f.normals[1] = f.faceNormal;
		f.normals[2] = f.faceNormal;
		mesh.tris.emplace_back(f);

		f.vertices[0] = ftr;
		f.uvs[0].u = 1.0f;
		f.uvs[0].v = 1.0f;
		f.vertices[1] = ftl;
		f.uvs[1].u = 0.0f;
		f.uvs[1].v = 1.0f;
		f.vertices[2] = btr;
		f.uvs[2].u = 1.0f;
		f.uvs[2].v = 0.0f;
		//f.faceNormal = { 0.0f,-1.0f,0.0f };
		mesh.tris.emplace_back(f);

		// Bottom
		f.vertices[0] = fbl;
		f.uvs[0].u = 0.0f;
		f.uvs[0].v = 0.0f;
		f.vertices[1] = fbr;
		f.uvs[1].u = 1.0f;
		f.uvs[1].v = 0.0f;
		f.vertices[2] = bbl;
		f.uvs[2].u = 0.0f;
		f.uvs[2].v = 1.0f;
		f.faceNormal = { 0.0f,1.0f,0.0f };
		f.normals[0] = f.faceNormal;
		f.normals[1] = f.faceNormal;
		f.normals[2] = f.faceNormal;
		mesh.tris.emplace_back(f);

		f.vertices[0] = fbr;
		f.uvs[0].u = 1.0f;
		f.uvs[0].v = 0.0f;
		f.vertices[1] = bbr;
		f.uvs[1].u = 1.0f;
		f.uvs[1].v = 1.0f;
		f.vertices[2] = bbl;
		f.uvs[2].u = 0.0f;
		f.uvs[2].v = 1.0f;
		//f.faceNormal = { 0.0f,1.0f,0.0f };
		mesh.tris.emplace_back(f);
	}

	// Segments need to be even for uv seam fix to work
	void GenerateCylinder(game::Mesh& mesh, const float_t topRadius, const float_t bottomRadius, const uint32_t segments, const float_t height, const game::Vector3f& __restrict pos, const game::Color color)
	{
		mesh.tris.clear();
		// Generate cone
		game::Triangle tri;
		tri.color[0] = color;
		tri.color[1] = color;
		tri.color[2] = color;

		const float invSeg = 1.0f / segments;
		const game::Vector3f center = pos;
		const game::Vector3f up = { 0,-height,0 };// * height;
		game::Vector3f dir;
		game::Color c;

		game::Vector3f topCenter = up;
		game::Vector3f botCenter = { 0,height,0 };
		for (uint32_t seg = 0; seg < segments; ++seg)
		{
			if (bottomRadius > 0.0f)
			{
				// Generate Vertices
				// bl
				tri.vertices[0].x = bottomRadius * cos((seg * invSeg) * (2.0f * 3.14159f));
				tri.vertices[0].z = bottomRadius * sin((seg * invSeg) * (2.0f * 3.14159f));
				tri.vertices[0].y = height;

				// tr
				tri.vertices[1].x = topRadius * cos(((seg + 1) * invSeg) * (2.0f * 3.14159f));
				tri.vertices[1].y = -height;
				tri.vertices[1].z = topRadius * sin(((seg + 1) * invSeg) * (2.0f * 3.14159f));

				// br
				tri.vertices[2].x = bottomRadius * cos(((seg + 1) * invSeg) * (2.0f * 3.14159f));
				tri.vertices[2].z = bottomRadius * sin(((seg + 1) * invSeg) * (2.0f * 3.14159f));
				tri.vertices[2].y = height;


				//Calculate UVs
				// U
				tri.uvs[0].u = (seg * invSeg);
				tri.uvs[1].u = ((seg + 1) * invSeg);
				tri.uvs[2].u = ((seg + 1) * invSeg);

				// V
				tri.uvs[0].v = 1;
				tri.uvs[2].v = 1;
				tri.uvs[1].v = 0;

				// Changing the position before uv gen will mess it up
				tri.vertices[0] += pos;
				tri.vertices[1] += pos;
				tri.vertices[2] += pos;

				game::GenerateFaceNormal(tri.vertices[0], tri.vertices[1], tri.vertices[2], tri.faceNormal);

				// Calculate vertex normals			
				dir = tri.vertices[0] - center;
				dir.Normalize();
				tri.normals[0] = dir + up;
				tri.normals[0].Normalize();

				dir = tri.vertices[1] - center;
				tri.normals[1] = dir + up;
				tri.normals[1].Normalize();

				dir = tri.vertices[2] - center;
				tri.normals[2] = dir + up;
				tri.normals[2].Normalize();


				mesh.tris.emplace_back(tri);


				// Generate bottom circle
				// bl
				tri.vertices[0].x = bottomRadius * cos((seg * invSeg) * (2.0f * 3.14159f));
				tri.vertices[0].z = bottomRadius * sin((seg * invSeg) * (2.0f * 3.14159f));
				tri.vertices[0].y = height;

				// tr
				tri.vertices[2] = botCenter;

				// br
				tri.vertices[1].x = bottomRadius * cos(((seg + 1) * invSeg) * (2.0f * 3.14159f));
				tri.vertices[1].z = bottomRadius * sin(((seg + 1) * invSeg) * (2.0f * 3.14159f));
				tri.vertices[1].y = height;

				//std::swap(tri.uvs[2], tri.uvs[1]);
				for (uint32_t i = 0; i < 3; i++)
				{
					tri.uvs[i].u = (tri.vertices[i].x) / bottomRadius * 0.5f + 0.5f;
					tri.uvs[i].v = (tri.vertices[i].z) / bottomRadius * 0.5f + 0.5f;

				}

				tri.faceNormal = { 0,1,0 };
				tri.normals[0] = tri.faceNormal;
				tri.normals[1] = tri.faceNormal;
				tri.normals[2] = tri.faceNormal;

				tri.vertices[0] += pos;
				tri.vertices[1] += pos;
				tri.vertices[2] += pos;


				mesh.tris.emplace_back(tri);

			}

			if (topRadius > 0.0f)
			{
				// Generate Vertices
				// bl
				tri.vertices[0].x = bottomRadius * cos((seg * invSeg) * (2.0f * 3.14159f));
				tri.vertices[0].z = bottomRadius * sin((seg * invSeg) * (2.0f * 3.14159f));
				tri.vertices[0].y = height;

				// tl
				tri.vertices[1].x = topRadius * cos(((seg)*invSeg) * (2.0f * 3.14159f));
				tri.vertices[1].z = topRadius * sin(((seg)*invSeg) * (2.0f * 3.14159f));
				tri.vertices[1].y = -height;

				// tr
				tri.vertices[2].x = topRadius * cos(((seg + 1) * invSeg) * (2.0f * 3.14159f));
				tri.vertices[2].z = topRadius * sin(((seg + 1) * invSeg) * (2.0f * 3.14159f));
				tri.vertices[2].y = -height;

				// Calculate UVs
				// U
				tri.uvs[0].u = (seg * invSeg);
				tri.uvs[1].u = (seg * invSeg);
				tri.uvs[2].u = ((seg + 1) * invSeg);
				// V
				tri.uvs[0].v = 1;
				tri.uvs[1].v = 0;
				tri.uvs[2].v = 0;

				// Changing the position before uv gen will mess it up
				tri.vertices[0] += pos;
				tri.vertices[1] += pos;
				tri.vertices[2] += pos;

				game::GenerateFaceNormal(tri.vertices[0], tri.vertices[1], tri.vertices[2], tri.faceNormal);

				// Calculate vertex normals			
				dir = tri.vertices[0] - center;
				dir.Normalize();
				tri.normals[0] = dir + up;
				tri.normals[0].Normalize();

				dir = tri.vertices[1] - center;
				tri.normals[1] = dir + up;
				tri.normals[1].Normalize();

				dir = tri.vertices[2] - center;
				tri.normals[2] = dir + up;
				tri.normals[2].Normalize();


				mesh.tris.emplace_back(tri);

				// Generate top circle
				tri.vertices[0] = topCenter;

				// tl // had to flip 2 and 1
				tri.vertices[2].x = topRadius * cos(((seg)*invSeg) * (2.0f * 3.14159f));
				tri.vertices[2].z = topRadius * sin(((seg)*invSeg) * (2.0f * 3.14159f));
				tri.vertices[2].y = -height;

				// tr
				tri.vertices[1].x = topRadius * cos(((seg + 1) * invSeg) * (2.0f * 3.14159f));
				tri.vertices[1].z = topRadius * sin(((seg + 1) * invSeg) * (2.0f * 3.14159f));
				tri.vertices[1].y = -height;

				for (uint32_t i = 0; i < 3; i++)
				{
					tri.uvs[i].u = (tri.vertices[i].x) / topRadius * 0.5f + 0.5f;
					tri.uvs[i].v = (tri.vertices[i].z) / topRadius * 0.5f + 0.5f;
				}

				tri.faceNormal = { 0, -1, 0 };
				tri.normals[0] = tri.faceNormal;
				tri.normals[1] = tri.faceNormal;
				tri.normals[2] = tri.faceNormal;

				tri.vertices[0] += pos;
				tri.vertices[1] += pos;
				tri.vertices[2] += pos;

				mesh.tris.emplace_back(tri);
			}
		}

		// For cones only
		if ((topRadius == 0) || (bottomRadius == 0))
		{
			// UV mapping from https://stackoverflow.com/questions/35141677/opengl-c-generating-uv-coordinates
			float_t min_X = (float)INFINITE;
			float_t max_X = (float)INFINITE * -1;
			float_t min_Y = (float)INFINITE;
			float_t max_Y = (float)INFINITE * -1;

			for (game::Triangle& vertex : mesh.tris)
			{
				for (int i = 0; i < 3; i++)
				{
					min_X = min(min_X, vertex.vertices[i].x);
					min_Y = min(min_Y, vertex.vertices[i].z);
					max_X = max(max_X, vertex.vertices[i].x);
					max_Y = max(max_Y, vertex.vertices[i].z);
				}
			}

			float_t k_X = 1.0f / (max_X - min_X);
			float_t k_Y = 1.0f / (max_Y - min_Y);

			for (game::Triangle& vertex : mesh.tris)
			{
				for (int i = 0; i < 3; i++)
				{
					vertex.uvs[i].u = (vertex.vertices[i].x - min_X) * k_X;
					vertex.uvs[i].v = (vertex.vertices[i].z - min_Y) * k_Y;
				}
			}
		}

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