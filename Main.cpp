//#define GAME_SUPPORT_OPENGL
//#define GAME_SUPPORT_DIRECTX9
//#define GAME_SUPPORT_DIRECTX10
#define GAME_SUPPORT_DIRECTX11
//#define GAME_SUPPORT_DIRECTX12

#include "game.h"
#include "GameSoftware3D.h"


class Game : public game::Engine
{

public:
	// Pixel renderers
	game::PixelMode pixelMode;
	game::Software3D software3D;


	// 3D rendering stuff
	game::ClippingRects clip;
	game::Matrix4x4f projMat;
	game::Matrix4x4f mvpMat;

	// Meshes for scenes
	//game::Mesh plane;
	//game::Mesh alphaCube;
	game::Mesh model;
	//game::Mesh torus;
	//game::Mesh sky;


	//std::vector<game::Triangle> trianglesToRender;
	//game::RenderTarget renderTarget;


	game::Camera3D camera;
	uint32_t maxFPS;


	game::FillMode state = game::FillMode::Filled;
	game::Pointi resolution = { 1280 , 720 };
	bool showText;

	Game() : game::Engine()
	{
		maxFPS = 0;
		showText = true;
	}

	void Initialize()
	{
#if defined(DEBUG) | defined(_DEBUG)
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		//_CrtSetBreakAlloc(613);
#endif
		game::Attributes attributes;
		attributes.WindowTitle = "Window Title";
		attributes.VsyncOn = false;
		attributes.FrameLock = 0;
		attributes.RenderingAPI = game::RenderAPI::DirectX11;
		attributes.DebugMode = false;
		geSetAttributes(attributes);

		//GenerateClips(numclips, clip, resolution);
		clip.SetNumberOfClipsRects(24);
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

		if (!LoadObj("Content/arena.obj", model))
		{
			std::cout << "Could not load model\n";
		}

		//if (!LoadObj("Content/torus2.obj", torus))
		//{
		//	std::cout << "Could not load model\n";
		//}

		//if (!LoadObj("Content/sky.obj", sky))
		//{
		//	std::cout << "Could not load model\n";
		//}

		//if (!LoadObj("Content/cubetest.obj", alphaCube))
		//{
		//	std::cout << "Could not load model\n";
		//}

		//if (!software3D.CreateRenderTarget(1280 >> 3, 720 >> 3, renderTarget))
		//{
		//	std::cout << "Could not create render target\n";
		//}


		//software3D.LoadTexture("content/colormap2.png", model.texture);
		//software3D.DeleteTexture(model.texture);
		//software3D.LoadTexture("content/sky.png", sky.texture);
		//software3D.LoadTexture("content/grate0_alpha.png", alphaCube.texture);

		//game::Random rnd;
		//rnd.NewSeed();

		float_t z = 0.0f;
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

		//plane.tris.emplace_back(topLeftTri);
		//plane.tris.emplace_back(bottomRightTri);


		// Preset some world stuff
		camera.position.z = -1.0f;
		camera.position.y = -0.2f;

		//plane.SetRotation(-3.14159f / 2.0f, 0, 0);
		//plane.SetTranslation(0.0f, 0.1f, 0.0f);
		//plane.SetScale(60.0f, 60.0f, 60.0f);

		//alphaCube.SetTranslation(-0.0f, -0.41f, 0.0f);
		//alphaCube.SetScale(0.5f, 0.5f, 0.5f);

		////model.SetRotation(3.14159f / 2.0f, 3.14159f, 0.0f);
		//model.SetScale(1.0f, 1.0f, 1.0f);
		//model.SetTranslation(0, 0.1f, 0);


		//sky.SetRotation(3.14159f / 2.0f, 0.0f, 0.0f);
		//sky.SetScale(50.0f, 50.0f, 50.0f);
		//sky.SetTranslation(camera.position.x, camera.position.y, camera.position.z);

		//torus.SetTranslation(0.0f, -0.1f, 0.0f);

		// Pre calc projection matrix
		game::my_PerspectiveFOV2(70.0f, resolution.x / (float_t)resolution.y, 0.1f, 100.0f, projMat);
	}

	void Shutdown()
	{
		//software3D.DeleteRenderTarget(renderTarget);
		//software3D.DeleteTexture(sky.texture);
		software3D.DeleteTexture(model.texture);
		//software3D.DeleteTexture(alphaCube.texture);
		//software3D.DeleteTexture(fire.mesh.texture);
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

		if (geKeyboard.WasKeyReleased(geK_COMMA))
		{
			software3D.SetState(GAME_SOFTWARE3D_WIREFRAME_COLOR, (int)game::Colors::CornFlowerBlue.packedABGR);
		}

		if (geKeyboard.WasKeyReleased(geK_PERIOD))
		{
			software3D.SetState(GAME_SOFTWARE3D_WIREFRAME_COLOR, (int)game::Colors::White.packedABGR);
		}

		if (geKeyboard.WasKeyPressed(geK_F1))
		{
			state++;
			software3D.SetState(GAME_SOFTWARE3D_STATE_FILL_MODE, state);
		}

		if (geKeyboard.WasKeyPressed(geK_LBRACKET))
		{
			software3D.SetState(GAME_SOFTWARE3D_STATE_THREADED, -1);
			clip.SetNumberOfClipsRects(1);
			clip.GenerateClips(resolution);
		}

		if (geKeyboard.WasKeyPressed(geK_RBRACKET))
		{
			software3D.SetState(GAME_SOFTWARE3D_STATE_THREADED, 0);
			clip.SetNumberOfClipsRects(16);
			clip.GenerateClips(resolution);
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
		float smoothedMouseDeltax = 0;
		// Y rotation
		if (mouse.x)
		{
			if (geMouse.IsButtonHeld(geM_LEFT))
			{
				smoothedMouseDeltax = (mouse.x / 2560.0f * 400.0f);//smoothingFactor * mouse.x + (1.0f - smoothingFactor) * smoothedMouseDeltax;
				camera.SetRotation(0.0f, smoothedMouseDeltax * (3.14159f / 180.0f), 0.0f);
			}
		}
		// X rotation
		float smoothedMouseDelta = 0;
		if (mouse.y)
		{
			if (geMouse.IsButtonHeld(geM_LEFT))
			{
				
				smoothedMouseDelta = (mouse.y / 1440.0f * 400.0f);//smoothingFactor * mouse.y + (1.0f - smoothingFactor) * smoothedMouseDelta;
				camera.SetRotation(-smoothedMouseDelta * (3.14159f / 180.0f), 0.0f, 0.0f);
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
		//static float_t pos = 0.0f;

		//rotation += (2 * 3.14f / 10.0f) * (msElapsed / 1000.0f);
		//pos += 0.5f * (msElapsed / 1000.0f);

		geClear(GAME_FRAME_BUFFER_BIT, game::Colors::Blue);

		pixelMode.Clear(game::Colors::Black);
		software3D.ClearDepth(100.0f);

		//torus.SetRotation(rotation, -rotation, rotation - 3.14156f / 2.0f);
		
		//model.SetRotation(3.14159f / 2.0f, 3.14159f, 0.0f);
		//sky.SetTranslation(camera.position.x, 1.5f, camera.position.z);
		//particle1.SetRotation(0,0,rotation);
		//particle1.SetTranslation(3.0f * sin(rotation), -0.5f, 0);
		//particle1.GenerateModelMatrix();
		//alphaCube.SetRotation(0.0f, rotation, 0.0f);
		//game::Vector3f center;// (model.centerPoint);
		//game::Vector3MultMatrix4x4(particle1.centerPoint, particle1.model, center);
		//camera.GenerateLookAtMatrix(camera.forward);
		camera.GenerateViewMatrix();
		//game::Vector3f c = { model.centerPoint.x, model.centerPoint.y, model.centerPoint.z - 0.07f };
		//camera.GenerateLookAtMatrix2(c); // doesn't work with billboards


		
		mvpMat = projMat * camera.view; // not sure if this should be in the RenderMesh

		software3D.SetState(GAME_SOFTWARE3D_LIGHTING, true);
		software3D.SetState(GAME_SOFTWARE3D_LIGHTING_TYPE, game::LightingType::Depth);
		software3D.SetState(GAME_SOFTWARE3D_TEXTURE, true);
		software3D.SetState(GAME_SOFTWARE3D_DEPTH_WRITE, true);
		software3D.SetState(GAME_SOFTWARE3D_SORT, game::SortingType::FrontToBack);
		software3D.SetState(GAME_SOFTWARE3D_BACKFACECULL, true);
		software3D.SetState(GAME_SOFTWARE3D_ALPHA_BLEND, false);
		software3D.SetState(GAME_SOFTWARE3D_ALPHA_TEST, false);
		software3D.SetState(GAME_SOFTWARE3D_COLOR_TINTING, false);
		software3D.RenderMesh(model, model.tris.size(), mvpMat, camera, clip);

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
				pixelMode.Text("Showing Depth buffer.", 0, 60, game::Colors::Magenta, 1);
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
			pixelMode.Text("Depth at mouse: " + std::to_string(depthAtMouse), 0, 40, game::Colors::Magenta, 1);
			pixelMode.Text("FPS: " + std::to_string(geGetFramesPerSecond()), 0, 0, game::Colors::Magenta, 1);
			
			pixelMode.Text("Max FPS: " + std::to_string(maxFPS), 0, 10, game::Colors::Magenta, 1);
			std::stringstream ss;
			ss << "Fill Mode: " << state;
			pixelMode.Text(ss.str(), 0, 20, game::Colors::Magenta, 1);
			pixelMode.Text("Working Threads: " + std::to_string(software3D.NumberOfThreads()), 0, 30, game::Colors::Magenta, 1);
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