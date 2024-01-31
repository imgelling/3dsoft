#if !defined(GAMESOFTWARE3D_H)
#define GAMESOFTWARE3D_H

#include <vector>
#include "GameMath.h"
#include "GameColor.h"
#include "GamePixelMode.h"
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
		void Fence(uint64_t fenceValue) noexcept;
		const Recti TriangleBoundingBox(const Triangle& tri) const noexcept;
		void Render(std::vector<Triangle>& tris, const Recti& clip) noexcept;
		template<bool wireFrame, bool filled>
		void DrawColored(const Triangle& tri, const Recti& clip);
		std::atomic<uint32_t> fence;
		uint32_t NumberOfThreads() const noexcept { return _threadPool.NumberOfThreads(); }
		// Must be called, sets the current frame buffer (for now)
		void ClearDepth(const float_t depth);
		void ScreenClip(std::vector<Triangle>& in, const Recti clip, std::vector<Triangle>& out) const noexcept;
		float_t* depthBuffer;
		float_t* clearDepthBuffer[10];
		uint32_t numbuffers;
		uint32_t currentDepth = 0;
		float_t time = 0.0f;
		uint32_t* _currentTexture;
		uint32_t _texW;
		uint32_t _texH;
		uint32_t* _colorBuffer;
	private:
		void _Render(std::vector<Triangle>& tris, const Recti& clip);
		bool _multiThreaded;
		ThreadPool _threadPool;
		uint32_t _colorBufferStride;
		uint32_t _totalBufferSize;
		FillMode _FillMode;
	};

	Software3D::Software3D()
	{
		_colorBuffer = nullptr;
		fence = 0;
		_colorBufferStride = 0;
		_totalBufferSize = 0;
		_multiThreaded = false;
		depthBuffer = nullptr;
		numbuffers = 5;
		for (uint32_t i = 0; i < 10; i++)
			clearDepthBuffer[i] = nullptr;
		_FillMode = FillMode::WireFrameFilled;
		_currentTexture = nullptr;
		_texW = 0;
		_texH = 0;
	}

	Software3D::~Software3D()
	{
		_threadPool.Stop();
		for (uint32_t i = 0; i < numbuffers; i++)
			if (clearDepthBuffer[i] != nullptr) delete[] clearDepthBuffer[i];
		depthBuffer = nullptr;
	}

	inline void Software3D::Fence(uint64_t fenceValue) noexcept
	{
		while (fence < fenceValue) {};
		fence = 0;
	}

	inline void Software3D::ClearDepth(const float_t depth)
	{
		if (_multiThreaded) //1235 torus no move
		{
			_threadPool.Queue(std::bind(std::fill_n<float_t*, uint32_t, float>, clearDepthBuffer[currentDepth], _totalBufferSize, depth));
			currentDepth++;
			if (currentDepth > numbuffers - 1) currentDepth = 0;
			depthBuffer = clearDepthBuffer[currentDepth];
		}
		else
		{
			std::fill_n(depthBuffer, _totalBufferSize, depth); 
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
		_colorBuffer = colorBuffer;
		fence = 0;
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
		depthBuffer = new float_t[colorBufferSize.width * colorBufferSize.height];
		//ClearDepth(1000.0f);
		std::fill_n(depthBuffer, _totalBufferSize, 100.0f); //5.14
		for (uint32_t i = 0; i < numbuffers; i++)
		{
			clearDepthBuffer[i] = new float_t[_totalBufferSize];
			memcpy(clearDepthBuffer[i], depthBuffer, (size_t)_totalBufferSize * 4);
		}
		currentDepth = 0;
		delete[] depthBuffer;
		depthBuffer = clearDepthBuffer[currentDepth];
		return true;
	}

	inline void Software3D::Render(std::vector<Triangle>& tris, const Recti& clip) noexcept
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

		for (uint32_t triangleCount = 0; triangleCount < tris.size(); ++triangleCount) //52.6
		{
				renderer(tris[triangleCount]);
		}
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
	inline void Software3D::DrawColored(const Triangle& triangle, const Recti& clip)
	{
		game::Vector3f vertex0(triangle.vertices[0].x, triangle.vertices[0].y, 0);
		game::Vector3f vertex1(triangle.vertices[1].x, triangle.vertices[1].y, 0);
		game::Vector3f vertex2(triangle.vertices[2].x, triangle.vertices[2].y, 0);

		bool foundTriangle(false);
		uint32_t videoBufferStride(_colorBufferStride);

		game::Vector2f pixelOffset;

		Vector3f oneOverW(1.0f / triangle.vertices[0].w, 1.0f / triangle.vertices[1].w, 1.0f / triangle.vertices[2].w);


		// Color parameter	
		Color colorAtPixel;
		ParameterEquation rColorParam(triangle.color[0].rf * oneOverW.x, triangle.color[1].rf * oneOverW.y, triangle.color[2].rf * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);
		ParameterEquation gColorParam(triangle.color[0].gf * oneOverW.x, triangle.color[1].gf * oneOverW.y, triangle.color[2].gf * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);
		ParameterEquation bColorParam(triangle.color[0].bf * oneOverW.x, triangle.color[1].bf * oneOverW.y, triangle.color[2].bf * oneOverW.z, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);

		// Depth parameter
		float_t oneOverDepthEval(0.0f);
		ParameterEquation depthParam(1.0f / triangle.vertices[0].w, 1.0f / triangle.vertices[1].w, 1.0f / triangle.vertices[2].w, triangle.edge0, triangle.edge1, triangle.edge2, triangle.area);

		// Face normal light pre calc (directional light) can add ambient here
		Vector3f faceNormal(triangle.faceNormal);// (0.0f, 0.0f, 1.0f);
		Vector3f lightNormal(0.0f, 0.0f, 1.0f);  // direction the light is shining to (opposite for y)
		lightNormal.Normalize();
		//rot += (2 * 3.14f / 10.0f) * (time / 1000.0f);
		//lightNormal = RotateXYZ(lightNormal, 0, time , 0);
		Color lightColor = Colors::Yellow;
		float_t luminance = -faceNormal.Dot(lightNormal);// Should have the negative as it is left handed
		luminance = max(0.0f, luminance);// < 0.0f ? 0.0f : lum;

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

		uint32_t* colorBuffer = _colorBuffer + (triangle.boundingBox.top * videoBufferStride + triangle.boundingBox.left);
		float_t* depthBufferPtr = depthBuffer + (triangle.boundingBox.top * videoBufferStride + triangle.boundingBox.left);
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
				if (oneOverDepthEval < *depthBufferPtr)
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
							float_t num = yy * x0 - xx * y0 + xyyx;
							float_t numerator = num * num;
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
					float_t luminanceAmbient(luminance + 0.05f);
					luminanceAmbient = 1.0f;// min(luminanceAmbient, 1.0f);

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
					int32_t tx = max((int32_t)(up * (_texW-1) + 0.5f), 0);	// -1 fix texture seams at max texW and texH
					int32_t ty = max((int32_t)(vp * (_texH-1) + 0.5f), 0);

					// texture lighting
					//uint32_t color = _currentTexture[(int32_t)ty * _texW + (int32_t)tx];
					uint32_t rc = (_currentTexture[ty * _texW + tx] >> 0) & 0xFF;  // 5.07
					uint32_t gc = (_currentTexture[ty * _texW + tx] >> 8) & 0xFF;
					uint32_t bc = (_currentTexture[ty * _texW + tx] >> 16) & 0xFF;
					rc = (uint32_t)(rc * luminanceAmbient);
					gc = (uint32_t)(gc * luminanceAmbient);
					bc = (uint32_t)(bc * luminanceAmbient); // 10.55
					//color = ((0xFF << 24) | (bc << 16) | (gc << 8) | (rc)); //7.25

					*colorBuffer = ((0xFF << 24) | (bc << 16) | (gc << 8) | (rc));// colorAtPixel.packedABGR; //0.91
				}
				++colorBuffer;
				++depthBufferPtr;
			}
			colorBuffer += videoBufferStride - xLoopCount;
			depthBufferPtr += videoBufferStride - xLoopCount;
		}
		fence++;
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

			// back face cull
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