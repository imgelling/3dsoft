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

namespace game
{

	class Software3D
	{
	public:
		Software3D();
		~Software3D();
		bool Initialize(uint32_t* colorBuffer, const Pointi& colorBufferSize, const int32_t threads);
		int32_t SetState(const uint32_t state, const int32_t value);
		bool SetTexture(const Texture& texture) noexcept;
		bool SetDefaultTexture() noexcept;
		void Fence(const uint64_t fenceValue) noexcept;
		const Recti TriangleBoundingBox(const Triangle& tri) const noexcept;
		void Render(const std::vector<Triangle>& tris, const Recti& clip) noexcept;
		template<bool wireFrame, bool filled>
		void DrawColored(const Triangle& tri, const Recti& clip) noexcept;
		std::atomic<uint32_t> fence;
		uint32_t NumberOfThreads() const noexcept { return _threadPool.NumberOfThreads(); }
		// Must be called, sets the current frame buffer (for now)
		void ClearDepth(const float_t depth);
		void ScreenClip(std::vector<Triangle>& in, const Recti clip, std::vector<Triangle>& out) const noexcept;
		bool CreateTexture(const uint32_t width, const uint32_t height, Texture& texture) noexcept;
		bool CreateTexture(const Pointi size, Texture& texture) noexcept;
		void DeleteTexture(Texture& texture) noexcept;
		bool CreateRenderTarget(const uint32_t width, const uint32_t height, RenderTarget& target) noexcept;
		bool CreateRenderTarget(const Pointi size, RenderTarget& target) noexcept;
		void DeleteRenderTarget(RenderTarget& target) noexcept;
		bool SetRenderTarget(const RenderTarget& target) noexcept;
		void SetRenderTargetDefault() noexcept;

		float_t* depthBuffer;
		float_t* clearDepthBuffer[10];
		//uint32_t* renderTarget;
		RenderTarget _currentRenderTarget;
	private:
		void _Render(const std::vector<Triangle>& tris, const Recti& clip) noexcept;
		void _GenerateDefaultTexture(uint32_t* buff, const uint32_t w, const uint32_t h);
		bool _multiThreaded;
		bool _usingRenderTarget;
		RenderTarget _defaultRenderTarget;
		Texture _defaultTexture;
		uint32_t _numbuffers;
		uint32_t _currentDepth;
		Texture _currentTexture;
		ThreadPool _threadPool;
		FillMode _FillMode;

		uint32_t _colorBufferStride;
		uint32_t _totalBufferSize;
	};

	Software3D::Software3D()
	{
		_defaultRenderTarget = {};
		_usingRenderTarget = false;
		//renderTarget = nullptr;
		fence = 0;
		_currentDepth = 0;
		_colorBufferStride = 0;
		_totalBufferSize = 0;
		_multiThreaded = false;
		depthBuffer = nullptr;
		_numbuffers = 5;
		for (uint32_t i = 0; i < 10; i++)
			clearDepthBuffer[i] = nullptr;
		_FillMode = FillMode::WireFrameFilled;
		CreateTexture(64, 64, _defaultTexture);
		_GenerateDefaultTexture(_defaultTexture.data, 64, 64);
		SetTexture(_defaultTexture);
	}

	Software3D::~Software3D()
	{
		_threadPool.Stop();
		for (uint32_t i = 0; i < _numbuffers; i++)
		{
			if (clearDepthBuffer[i] != nullptr)
			{
				delete[] clearDepthBuffer[i];
				clearDepthBuffer[i] = nullptr;
			}
		}
		DeleteTexture(_defaultTexture);
		depthBuffer = nullptr;
	}

	inline bool Software3D::SetTexture(const Texture& texture) noexcept
	{
		if (texture.data == nullptr)
		{
			return false;
		}
		_currentTexture = texture;
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
		texture.oneOverSize.width = 1.0f / width;
		texture.oneOverSize.height = 1.0f / height;
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
		texture.oneOverSize.width = 0;
		texture.oneOverSize.height = 0;		
	}

	inline void Software3D::_GenerateDefaultTexture(uint32_t* buff, const uint32_t w, const uint32_t h)
	{
		game::Color col1 = game::Colors::Red;
		game::Color col2 = game::Colors::Blue;
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
		depthBuffer = _defaultRenderTarget.depthBuffer;
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
		while (fence < fenceValue) {  };
		fence = 0;
	}

	inline void Software3D::ClearDepth(const float_t depth)
	{
		if (!_usingRenderTarget)
		{
			if (_multiThreaded)
			{
				_threadPool.Queue(std::bind(std::fill_n<float_t*, uint32_t, float>, clearDepthBuffer[_currentDepth], _totalBufferSize, depth));
				_currentDepth++;
				if (_currentDepth > _numbuffers - 1) _currentDepth = 0;
				_currentRenderTarget.depthBuffer = clearDepthBuffer[_currentDepth];
				depthBuffer = clearDepthBuffer[_currentDepth];
			}
			else
			{
				std::fill_n(_currentRenderTarget.depthBuffer, _totalBufferSize, depth);
			}
		}
		else
		{
			std::fill_n(_currentRenderTarget.depthBuffer, _currentRenderTarget.size.width * _currentRenderTarget.size.height, depth);
		}
		//std::fill(depthBuffer, depthBuffer + _totalBufferSize, depth);
		//FillMemory(depthBuffer, 255, _totalBufferSize*4);
		//memcpy(depthBuffer, clearDepthBuffer, _totalBufferSize * 4);
		//MoveMemory(depthBuffer, clearDepthBuffer, _totalBufferSize * 4);
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
			return true;
		}

		return false;
	}

	inline bool Software3D::Initialize(uint32_t* colorBuffer, const Pointi& colorBufferSize, const int32_t threads = -1)
	{
		_defaultRenderTarget.colorBuffer = colorBuffer;
		_defaultRenderTarget.size = colorBufferSize;
		//renderTarget = colorBuffer;
		_colorBufferStride = colorBufferSize.width;
		_totalBufferSize = _colorBufferStride * colorBufferSize.height;
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
			clearDepthBuffer[i] = new float_t[_totalBufferSize];
			std::fill_n(clearDepthBuffer[i], _totalBufferSize, 100.0f);
		}
		_currentDepth = 0;
		_defaultRenderTarget.depthBuffer = clearDepthBuffer[_currentDepth];
		_currentRenderTarget = _defaultRenderTarget;
		//depthBuffer = clearDepthBuffer[_currentDepth];
		return true;
	}

	inline void Software3D::Render(const std::vector<Triangle>& tris, const Recti& clip) noexcept
	{
		if (_multiThreaded)
		{
			_threadPool.Queue(std::bind(&Software3D::_Render, this, tris,clip));
		}
		else
		{
			_Render(tris,clip);
		}
	}

	inline void Software3D::_Render(const std::vector<Triangle>& tris, const Recti& clip) noexcept
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
				renderer(tris[triangleCount]);
		}
		fence++;
	}

	inline const Recti Software3D::TriangleBoundingBox(const Triangle& tri) const noexcept
	{
		Recti boundingBox;

		int32_t sx1 = (int32_t)(tri.vertices[0].x);
		int32_t sx2 = (int32_t)(tri.vertices[1].x);
		int32_t sx3 = (int32_t)(tri.vertices[2].x);
		int32_t sy1 = (int32_t)(tri.vertices[0].y);
		int32_t sy2 = (int32_t)(tri.vertices[1].y);
		int32_t sy3 = (int32_t)(tri.vertices[2].y);

		boundingBox.right = sx1 > sx2 ? (sx1 > sx3 ? sx1 : sx3) : (sx2 > sx3 ? sx2 : sx3);
		boundingBox.bottom = sy1 > sy2 ? (sy1 > sy3 ? sy1 : sy3) : (sy2 > sy3 ? sy2 : sy3);
		boundingBox.left = sx1 < sx2 ? (sx1 < sx3 ? sx1 : sx3) : (sx2 < sx3 ? sx2 : sx3);
		boundingBox.top = sy1 < sy2 ? (sy1 < sy3 ? sy1 : sy3) : (sy2 < sy3 ? sy2 : sy3);

		return boundingBox;
	}

	template<bool renderWireFrame, bool renderColor>
	inline void Software3D::DrawColored(const Triangle& triangle, const Recti& clip) noexcept
	{
		game::Vector3f vertex0(triangle.vertices[0].x, triangle.vertices[0].y, 0);
		game::Vector3f vertex1(triangle.vertices[1].x, triangle.vertices[1].y, 0);
		game::Vector3f vertex2(triangle.vertices[2].x, triangle.vertices[2].y, 0);

		bool foundTriangle(false);
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
		ParameterEquation depthParam(1.0f / triangle.vertices[0].w, 1.0f / triangle.vertices[1].w, 1.0f / triangle.vertices[2].w, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);

		// Face normal light pre calc (directional light) can add ambient here
		Vector3f faceNormal(triangle.faceNormal);// (0.0f, 0.0f, 1.0f);
		Vector3f lightNormal(0.0f, 0.0f, 1.0f);  // direction the light is shining to (opposite for y)
		lightNormal.Normalize();
		//rot += (2 * 3.14f / 10.0f) * (time / 1000.0f);
		//lightNormal = RotateXYZ(lightNormal, 0, time , 0);
		//Color lightColor = Colors::Yellow;
		float_t luminance = -faceNormal.Dot(lightNormal);// Should have the negative as it is left handed
		luminance = max(0.0f, luminance);

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
			foundTriangle = false;
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
				foundTriangle = true;

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
					float_t luminanceAmbient(luminance + 0.25f);
					luminanceAmbient = min(luminanceAmbient, 1.0f);

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
					rc = (uint32_t)(rc * luminanceAmbient);
					gc = (uint32_t)(gc * luminanceAmbient);
					bc = (uint32_t)(bc * luminanceAmbient); 
					//color = ((0xFF << 24) | (bc << 16) | (gc << 8) | (rc)); //7.25

					*colorBuffer = ((0xFF << 24) | (bc << 16) | (gc << 8) | (rc));// colorAtPixel.packedABGR; 
				}
				++colorBuffer;
				++depthBufferPtr;
			}
			colorBuffer += videoBufferStride - xLoopCount;
			depthBufferPtr += videoBufferStride - xLoopCount;
		}
		//fence++;
	}

	// clipping  
	inline void Software3D::ScreenClip(std::vector<Triangle>& in, const Recti clip, std::vector<Triangle>& out) const noexcept
	{
		out.clear();
		Triangle outTri;
		for (int tri = 0; tri < in.size(); tri++)
		{
			//Triangle outTri(in[tri]);
			if (in[tri].backFaceCulled) continue; // was backface culled before

			//// Near Z clip
			//if ((in[tri].vertices[0].w < 0.1f) ||
			//	(in[tri].vertices[1].w < 0.1f) ||
			//	(in[tri].vertices[2].w < 0.1f))
			//{
			//	continue;
			//}

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
				in[tri].area = in[tri].edge0.c + in[tri].edge1.c + in[tri].edge2.c;
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
			if ((in[tri].boundingBox.right < clip.left) || (in[tri].boundingBox.left > clip.right) ||
				(in[tri].boundingBox.bottom < clip.top) || (in[tri].boundingBox.top > clip.bottom))
			{
				continue;
			}


			// This needs copied as the bounding box modification 
			// will only apply to this clip
			outTri = in[tri];

			// Partial offscreen
			if (outTri.boundingBox.left < clip.left)
				outTri.boundingBox.left = clip.left;
			if (outTri.boundingBox.right > clip.right)
				outTri.boundingBox.right = clip.right;
			if (outTri.boundingBox.top < clip.top)
				outTri.boundingBox.top = clip.top;
			if (outTri.boundingBox.bottom > clip.bottom)
				outTri.boundingBox.bottom = clip.bottom;

			out.emplace_back(outTri);
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

			if (!hasNormals)
			{
				for (int i = 0; i < mesh.tris.size(); i++)
				{
					mesh.tris[i].normals[0] = norms[fcount[i].x] / vcount[fcount[i].x];
					mesh.tris[i].normals[1] = norms[fcount[i].y] / vcount[fcount[i].y];
					mesh.tris[i].normals[2] = norms[fcount[i].z] / vcount[fcount[i].z];
					mesh.tris[i].normals[0].Normalize();
					mesh.tris[i].normals[1].Normalize();
					mesh.tris[i].normals[2].Normalize();
				}
			}
			for (int i = 0; i < mesh.tris.size(); i++)
			{
				mesh.tris[i].color[0] = game::Colors::White;
				mesh.tris[i].color[1] = game::Colors::White;
				mesh.tris[i].color[2] = game::Colors::White;
			}

			return true;
		}
		else return false;

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