//#define GAME_SUPPORT_OPENGL
//#define GAME_SUPPORT_DIRECTX9
//#define GAME_SUPPORT_DIRECTX10
#define GAME_SUPPORT_DIRECTX11
//#define GAME_SUPPORT_DIRECTX12

#include "game.h"
#include "GameSoftware3D.h"

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
			clips2[access].right = (clips2[access].left + colsize);
			if (clips2[access].right > size.width - 1) clips2[access].right = size.width - 1;
			clips2[access].top = cc * (rowsize);
			clips2[access].bottom = clips2[access].top + rowsize;
			if (clips2[access].bottom > size.height - 1) clips2[access].bottom = size.height - 1;
			rc++;
		}
		cc++;
	}
	return true;
}

//void GenerateClips(const uint32_t numberOfClips, game::Recti* clips2, const game::Pointi& size)
//{
//	uint32_t cols = (uint32_t)sqrt(numberOfClips);//(int32_t)std::ceil(sqrt(numberOfClips));
//	uint32_t rows = (uint32_t)(numberOfClips / (float_t)cols);//(int32_t)std::ceil(numberOfClips / (float_t)cols);
//
//	uint32_t colsize = (uint32_t)std::ceil(size.width / (float_t)cols);
//	uint32_t rowsize = (uint32_t)std::ceil(size.height / (float_t)rows);
//
//	uint32_t rc = 0;
//	uint32_t cc = 0;
//	for (uint32_t row = 0; row < rows; row++)
//	{
//		rc = 0;
//		for (uint32_t col = 0; col < cols; col++)
//		{
//			uint32_t access = row * cols + col;
//			clips2[access].left = (rc) * (colsize);
//			clips2[access].right = (clips2[access].left + colsize);
//			if (clips2[access].right > size.width - 1) clips2[access].right = size.width - 1;
//			clips2[access].top = cc * (rowsize);
//			clips2[access].bottom = clips2[access].top + rowsize;
//			if (clips2[access].bottom > size.height - 1) clips2[access].bottom = size.height - 1;
//			rc++;
//		}
//		cc++;
//	}
//}
class Game : public game::Engine
{

public:
	// Pixel renderers
	game::PixelMode pixelMode;
	game::Software3D software3D;


	// 3D rendering stuff
	ClippingRects clip;
	//std::vector<game::Triangle> clippedTris[16];
	game::Matrix4x4f projMat;
	game::Matrix4x4f mvpMat;

	// Meshes for scenes
	game::Mesh plane;
	game::Mesh model;
	game::Mesh oneKTris;


	std::vector<game::Triangle> trianglesToRender;
	game::RenderTarget renderTarget;


	game::Camera3D camera;
	uint32_t maxFPS;


	game::FillMode state = game::FillMode::FilledColor;
	game::Pointi resolution = { 1280, 720 };
	bool showText;

	Game() : game::Engine()
	{
		maxFPS = 0;
		showText = true;
	}

	//uint32_t numclips = 16;


	void Initialize()
	{
//#if defined(DEBUG) | defined(_DEBUG)
//		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//		//_CrtSetBreakAlloc(613);
//#endif
		game::Attributes attributes;
		attributes.WindowTitle = "Window Title";
		attributes.VsyncOn = false;
		attributes.RenderingAPI = game::RenderAPI::DirectX11;
		attributes.DebugMode = true;
		geSetAttributes(attributes);

		//GenerateClips(numclips, clip, resolution);
		clip.SetNumberOfClipsRects(16);
		clip.GenerateClips(resolution);
	}

	void LoadContent()
	{
		if (!pixelMode.Initialize(resolution))
		{
			geLogLastError();
		}

		if (!software3D.Initialize(pixelMode, 0))
		{
			geLogLastError();
		}

		software3D.SetState(GAME_SOFTWARE3D_STATE_FILL_MODE, state);

		// cone +z, conex +x, coney +y
		if (!LoadObj("Content/character-ghost.obj", model))
		{
			std::cout << "Could not load model\n";
		}

		if (!software3D.CreateRenderTarget(1280 >> 3, 720 >> 3, renderTarget))
		{
			std::cout << "Could not create render target\n";
		}

		game::ImageLoader imageloader;
		uint32_t t = 0;
		uint32_t texw = 0;
		uint32_t texh = 0;
		uint32_t* temp = (uint32_t*)imageloader.Load("content/colormap2.png", texw, texh, t);
		model.texture.data = new uint32_t[texw * texh];
		memcpy(model.texture.data, temp, (size_t)texw * texh * 4);
		model.texture.size.width = texw;
		model.texture.size.height = texh;
		

		game::Random rnd;
		rnd.NewSeed();

		float_t z = 0.0f;// 100.0f;
		float_t size = 1.0f;
		game::Triangle topLeftTri;
		game::Triangle bottomRightTri;

		// tl
		topLeftTri.vertices[0].x = -size;
		topLeftTri.vertices[0].y = -size;
		topLeftTri.vertices[0].z = z;
		topLeftTri.color[0] = game::Colors::Red;
		topLeftTri.uvs[0].u = 0.0f;
		topLeftTri.uvs[0].v = 0.0f;
		topLeftTri.faceNormal.x = 0.0f;
		topLeftTri.faceNormal.y = 0.0f;
		topLeftTri.faceNormal.z = -1.0f;


		// tr
		topLeftTri.vertices[1].x = size;
		topLeftTri.vertices[1].y = -size;
		topLeftTri.vertices[1].z = z;
		topLeftTri.uvs[1].u = 1.0f;
		topLeftTri.uvs[1].v = 0.0f;
		topLeftTri.color[1] = game::Colors::Green;

		// bl
		topLeftTri.vertices[2].x = -size;
		topLeftTri.vertices[2].y = size;
		topLeftTri.vertices[2].z = z;
		topLeftTri.uvs[2].u = 0.0f;
		topLeftTri.uvs[2].v = 1.0f;
		topLeftTri.color[2] = game::Colors::Blue;

		// tr
		bottomRightTri.vertices[0].x = size;
		bottomRightTri.vertices[0].y = -size;
		bottomRightTri.vertices[0].z = z;
		bottomRightTri.color[0] = game::Colors::Green;
		bottomRightTri.uvs[0].u = 1.0f;
		bottomRightTri.uvs[0].v = 0.0f;
		bottomRightTri.faceNormal.x = 0.0f;
		bottomRightTri.faceNormal.y = 0.0f;
		bottomRightTri.faceNormal.z = -1.0f;

		// br
		bottomRightTri.vertices[1].x = size;
		bottomRightTri.vertices[1].y = size;
		bottomRightTri.vertices[1].z = z;
		bottomRightTri.uvs[1].u = 1.0f;
		bottomRightTri.uvs[1].v = 1.0f;
		bottomRightTri.color[1] = game::Colors::White;

		// bl
		bottomRightTri.vertices[2].x = -size;
		bottomRightTri.vertices[2].y = size;
		bottomRightTri.vertices[2].z = z;
		bottomRightTri.uvs[2].u = 0.0f;
		bottomRightTri.uvs[2].v = 1.0f;
		bottomRightTri.color[2] = game::Colors::Blue;

		for (uint32_t i = 0; i < 3; i++)
		{
			topLeftTri.normals[i] = { 0.0f,0.0f,-1.0f };
			bottomRightTri.normals[i] = { 0.0f,0.0f,-1.0f };
		}

		plane.tris.emplace_back(topLeftTri);
		plane.tris.emplace_back(bottomRightTri);


		// Pre calc projection numbers
		//game::my_PerspectiveFOV2(90.0f, resolution.x / (float_t)resolution.y, 0.1f, 100.0f, projection);
		// Pre calc projection matrix
		game::my_PerspectiveFOV2(90.0f, resolution.x / (float_t)resolution.y, 0.1f, 100.0f, projMat);
		trianglesToRender.reserve(1000);

		camera.position.z = -2.0f;

		//currentMesh = &model;
	}

	void Shutdown()
	{
		software3D.DeleteRenderTarget(renderTarget);
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

		if (geKeyboard.WasKeyPressed(geK_F1))
		{
			state++;
			software3D.SetState(GAME_SOFTWARE3D_STATE_FILL_MODE, state);
		}

		if (geKeyboard.WasKeyPressed(geK_LBRACKET))
		{
			software3D.SetState(GAME_SOFTWARE3D_STATE_THREADED, -1);
		}

		if (geKeyboard.WasKeyPressed(geK_RBRACKET))
		{
			software3D.SetState(GAME_SOFTWARE3D_STATE_THREADED, 0);
		}

		float_t speed = 5.0f * msElapsed / 1000.0f;
		if (geKeyboard.IsKeyHeld(geK_SHIFT))
		{
			speed = 0.1f * msElapsed / 1000.0f;
		}

		// Move forward
		if (geKeyboard.IsKeyHeld(geK_W))
		{
			camera.position += (camera.forward * speed);
		}

		// Move backward
		if (geKeyboard.IsKeyHeld(geK_S))
		{
			camera.position -= (camera.forward * speed);
		}

		// strafe left
		if (geKeyboard.IsKeyHeld(geK_A))
		{
			camera.position -= camera.right * speed;
		}

		// strafe right
		if (geKeyboard.IsKeyHeld(geK_D))
		{
			camera.position += camera.right * speed;
		}

		// y is inverted because.... we are in Q4
		if (geKeyboard.IsKeyHeld(geK_UP))
		{
			camera.position.y -= speed;
		}

		// move actually down
		if (geKeyboard.IsKeyHeld(geK_DOWN))
		{
			camera.position.y += speed;
		}

		// Mouse look
		game::Pointi mouse = geMouse.GetPositionRelative();
		// Y rotation
		if (mouse.x)
		{
			if (geMouse.IsButtonHeld(geM_LEFT))
			{
				camera.SetRotation(0.0f, mouse.x * (3.14159f / 180.0f), 0.0f);
			}
		}
		// X rotation
		if (mouse.y)
		{
			if (geMouse.IsButtonHeld(geM_LEFT))
			{
				camera.SetRotation(-mouse.y * (3.14159f / 180.0f), 0.0f, 0.0f);
			}
		}


		if (geKeyboard.WasKeyPressed(geK_F3))
		{
			showText = !showText;
		}
	}	

	void Render(const float_t msElapsed)
	{
		//static float_t rotation = 0.0f;
		static float_t pos = 0.0f;

		//rotation += (2 * 3.14f / 10.0f) * (msElapsed / 1000.0f);
		pos += 1 * (msElapsed / 1000.0f);

		geClear(GAME_FRAME_BUFFER_BIT, game::Colors::Blue);

		pixelMode.Clear(game::Colors::Black);
		software3D.ClearDepth(100.0f);

		trianglesToRender.clear();		

		plane.SetRotation(-3.14159f / 2.0f, 0, 0);
		plane.SetTranslation(0.0f, 0.1f, 0.0f);
		plane.SetScale(50.0f, 50.0f, 50.0f);

		model.SetRotation(3.14159f / 2.0f, 3.14159f, 0.0f);
		model.SetTranslation(0.0f,0.1f,0.0f);
		model.GenerateModelMatrix();
		
		game::Vector3f center(model.centerPoint);
		center = center * model.model;
		camera.GenerateLookAtMatrix(center);
		camera.GenerateViewMatrix();
		mvpMat = projMat * camera.lookAt;


		software3D.VertexProcessor(plane, mvpMat, trianglesToRender, camera);


		software3D.SetTexture(plane.texture);
		uint64_t fenceCount = 0;
		for (uint32_t c = 0; c < clip.numberOfClipRects; c++)
		{
			clip.clippedTris[c].clear();
			software3D.ScreenClip(trianglesToRender, clip.clips[c], clip.clippedTris[c]);
			if (!clip.clippedTris[c].size()) continue;
			std::sort(clip.clippedTris[c].begin(), clip.clippedTris[c].end(), [](const game::Triangle& a, const game::Triangle& b) 
				{
					float_t az = a.vertices[0].z + a.vertices[1].z + a.vertices[2].z;
					float_t bz = b.vertices[0].z + b.vertices[1].z + b.vertices[2].z;
					return az < bz;
				});
			//pixelMode.Rect(clip[c], game::Colors::Yellow);
			software3D.Render(clip.clippedTris[c], clip.clips[c]);
			fenceCount++;
		}
		software3D.Fence(fenceCount);
		trianglesToRender.clear();


		software3D.VertexProcessor(model, mvpMat, trianglesToRender, camera);
		software3D.SetTexture(model.texture);
		fenceCount = 0;
		for (uint32_t c = 0; c < clip.numberOfClipRects; c++)
		{
			clip.clippedTris[c].clear();
			software3D.ScreenClip(trianglesToRender, clip.clips[c], clip.clippedTris[c]);
			if (!clip.clippedTris[c].size()) continue;
			std::sort(clip.clippedTris[c].begin(), clip.clippedTris[c].end(), [](const game::Triangle& a, const game::Triangle& b)
				{
					float_t az = a.vertices[0].z + a.vertices[1].z + a.vertices[2].z;
					float_t bz = b.vertices[0].z + b.vertices[1].z + b.vertices[2].z;
					return az < bz;
				});
			//pixelMode.Rect(clip[c], game::Colors::Yellow);
			software3D.Render(clip.clippedTris[c], clip.clips[c]);
			fenceCount++;
		}
		software3D.Fence(fenceCount);

		//uint64_t num2 = currentMesh->tris.size();
		//for (uint32_t i2 = 0; i2 < num2; i2++)
		//{
		//	game::Vector3f norm;
		//	//for (int v = 0; v < 1; v++)
		//	int v = 0;
		//	{
		//		game::Vector3f point = currentMesh->tris[i2].vertices[0];
		//		point += currentMesh->tris[i2].vertices[1];
		//		point += currentMesh->tris[i2].vertices[2];
		//		point /= 3.0f;

		//		norm = point + (currentMesh->tris[i2].faceNormal * 0.5f);
		//		norm = norm * mvpMat;
		//		if (norm.z < 0) norm.z = 0.0f;
		//		if (norm.w < 0) norm.w = 0.0f;
		//		norm = norm / norm.w;
		//		norm.x += 1.0f;
		//		norm.y += 1.0f;
		//		norm.x *= resolution.width >> 1;
		//		norm.y *= resolution.height >> 1;

		//		point = point * mvpMat;
		//		//if (point.z < 0) point.z = 0.0f;
		//		if (point.w < 0) point.w = 0.0f;
		//		point = point / point.w;
		//		point.x += 1.0f;
		//		point.y += 1.0f;
		//		point.x *= resolution.width >> 1;
		//		point.y *= resolution.height >> 1;


		//		pixelMode.LineClip(uint32_t(point.x),
		//			uint32_t(point.y),
		//			uint32_t(norm.x),
		//			uint32_t(norm.y),
		//			game::Colors::Yellow
		//		);
		//	}
		//}

		// show depth buffer
		if (geKeyboard.IsKeyHeld(geK_SPACE))
		{
			game::Color dColor;
			float_t depth = 0.0f;
			float_t* zbuffer = software3D.depthBuffer;
			uint32_t* vbuffer = pixelMode.videoBuffer;
			for (int pos = 0; pos < resolution.height * resolution.width; pos++)
			{
				depth = *zbuffer;
				zbuffer++;
				depth += 1.0f;  // 1 added because z becomes < 1.0f near camera and makes depth > 1.0 making colors
								// go all weird
				depth = 1.0f/depth;
				dColor.Set(1.0f * depth, 1.0f * depth, 1.0f * depth, 1.0f);
				*vbuffer = dColor.packedABGR;
				vbuffer++;
			}
			if (showText)
				pixelMode.Text("Showing Depth buffer.", 0, 60, game::Colors::Yellow, 1);
		}

		maxFPS = max(maxFPS, geGetFramesPerSecond());
		if (showText)
		{
			game::Pointi m = pixelMode.GetScaledMousePosition();
			float_t* zb = software3D.depthBuffer;
			m.x = min(m.x, resolution.width - 1);
			m.y = min(m.y, resolution.height - 1);
			m.x = max(m.x, 0);
			m.y = max(m.y, 0);
			float_t depthAtMouse = zb[(m.y * resolution.width + m.x)];
			pixelMode.Text("Depth at mouse: " + std::to_string(depthAtMouse), 0, 40, game::Colors::Yellow, 1);


			pixelMode.Text("FPS: " + std::to_string(geGetFramesPerSecond()), 0, 0, game::Colors::Yellow, 1);
			
			pixelMode.Text("Max FPS: " + std::to_string(maxFPS), 0, 10, game::Colors::Yellow, 1);
			std::stringstream ss;
			ss << "Fill Mode: " << state;
			pixelMode.Text(ss.str(), 0, 20, game::Colors::Yellow, 1);
			pixelMode.Text("Working Threads: " + std::to_string(software3D.NumberOfThreads()), 0, 30, game::Colors::Yellow, 1);
		}
		pixelMode.Render();
		if (geKeyboard.WasKeyPressed(geK_F5))
		{
			game::ImageSaver save;
			if (!save.Save(pixelMode.videoBuffer, "test.png", resolution.width, resolution.height, 0))
			{
				std::cout << "save failed---- :(\n";
			}
		}
	}



};

int32_t main()
{
	game::Logger logger("Log.html");
	Game engine;
	engine.geSetLogger(&logger);

	testmy_PerspectiveFOV (90.0f, 16.0f / 9.0f, 0.1f, 100.0f);
	testmy_PerspectiveFOV2(90.0f, 16.0f / 9.0f, 0.1f, 100.0f);

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