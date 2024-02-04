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


	// 3D stuff
	game::Triangle topLeftTri;
	game::Triangle bottomRightTri;
	game::Recti clip[16];  // in renderer
	std::vector<game::Triangle> clippedTris[16];
	game::Projection projection;
	game::Matrix4x4f projMat;
	std::vector<game::Triangle> tris;
	game::Mesh model;

	std::vector<game::Triangle> quad;
	game::Triangle test;

	game::Camera3D camera;
	uint32_t maxFPS;
	uint32_t scene;
	float_t tz;
	uint32_t* texture;
	uint32_t texW;
	uint32_t texH;

	game::FillMode state = game::FillMode::FilledColor;
	game::Pointi resolution = { 1280 , 720 }; //2560, 1440 };
	bool showText;

	Game() : game::Engine()
	{
		ZeroMemory(&projection, sizeof(game::Projection));
		maxFPS = 0;
		scene = 2;
		tz = 0.0f;
		showText = true;
		texture = nullptr;
		texW = 64;
		texH = 64;
	}

	uint32_t numclips = 16;
	void GenerateClips(const uint32_t numberOfClips, game::Recti *clips2, const game::Pointi& size)
	{
		uint32_t cols = (uint32_t)sqrt(numberOfClips);//(int32_t)std::ceil(sqrt(numberOfClips));
		uint32_t rows = (uint32_t)(numberOfClips / (float_t)cols);//(int32_t)std::ceil(numberOfClips / (float_t)cols);

		uint32_t colsize = (uint32_t)std::ceil(size.width / (float_t)cols);
		uint32_t rowsize = (uint32_t)std::ceil(size.height / (float_t)rows);

		uint32_t rc = 0;
		uint32_t cc = 0;
		for (uint32_t row = 0; row < rows; row++)
		{
			rc = 0;
			for (uint32_t col = 0; col < cols; col++)
			{
				uint32_t access = row * cols + col;
				clips2[access].left = (rc) * (colsize - 1);
				clips2[access].right = (clips2[access].left + colsize);
				if (clips2[access].right > size.width - 1) clips2[access].right = size.width - 1;
				clips2[access].top = cc * (rowsize - 1);
				clips2[access].bottom = clips2[access].top + rowsize;
				if (clips2[access].bottom > size.height - 1) clips2[access].bottom = size.height - 1;
				rc++;
			}
			cc++;
		}
	}

	void Initialize()
	{
//#if defined(DEBUG) | defined(_DEBUG)
		//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		//_CrtSetBreakAlloc(613);
//#endif
		game::Attributes attributes;

		attributes.WindowTitle = "Window Title";
		attributes.VsyncOn = false;
		attributes.RenderingAPI = game::RenderAPI::DirectX11;
		attributes.DebugMode = false;
		geSetAttributes(attributes);

		GenerateClips(numclips, clip, resolution);
	}

	void LoadContent()
	{
		if (!pixelMode.Initialize(resolution))// 640, 360 }))
		{
			geLogLastError();
		}

		if (!software3D.Initialize(pixelMode.videoBuffer, pixelMode.GetPixelFrameBufferSize(),0))
		{
			geLogLastError();
		}

		software3D.SetState(GAME_SOFTWARE3D_STATE_FILL_MODE, state);

		// cone +z, conex +x, coney +y
		if (!LoadObj("Content/torus2.obj", model))
		{
			std::cout << "Could not load model\n";
		}

		ConvertBlenderToThis(model);

		texture = new uint32_t[texW * texH];
		GenerateCheckerboard(texture, texW, texH);
		software3D._currentTexture = texture;
		software3D._texH = texH;
		software3D._texW = texW;

		//game::ImageLoader imageLoader;
		//uint32_t t = 0;
		//uint32_t* temp = (uint32_t*)imageLoader.Load("Content/skin_adventurer.png", texW, texH, t);
		//texture = new uint32_t[texW * texH];
		//memcpy(texture, temp, (size_t)texW * texH * 4);
		//software3D._currentTexture = texture;
		//software3D._texH = texH;
		//software3D._texW = texW;

		game::Random rnd;
		rnd.NewSeed();

		float_t z = 0.0f;// 100.0f;
		float_t size = 1.0f;

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


		// Generate a 1000 tris
		for (uint32_t i = 0; i < 1000; i++)
		{
			game::Triangle temp(topLeftTri);
			float_t tz = rnd.RndRange(0, 1000) / (float_t)rnd.RndRange(1, 10);
			for (uint32_t v = 0; v < 3; v++)
			{
				temp.vertices[v].x = (float_t)rnd.RndRange(0, resolution.x);
				temp.vertices[v].x = temp.vertices[v].x * 2.0f / (float_t)resolution.x - 1.0f;
				temp.vertices[v].y = (float_t)rnd.RndRange(0, resolution.y);
				temp.vertices[v].y = temp.vertices[v].y * 2.0f / (float_t)resolution.y - 1.0f;
				temp.vertices[v].z = 1000.0f / i;
			}

			game::EdgeEquation e0(temp.vertices[1], temp.vertices[2]);
			game::EdgeEquation e1(temp.vertices[2], temp.vertices[0]);
			game::EdgeEquation e2(temp.vertices[0], temp.vertices[1]);

			float_t area(e0.c + e1.c + e2.c);
			// If area is negative, it means wrong winding
			if (area < 0)
			{
				std::swap(temp.vertices[1], temp.vertices[2]);
			}
			tris.emplace_back(temp);
		}

		// Pre calc projection numbers
		//game::my_PerspectiveFOV2(90.0f, resolution.x / (float_t)resolution.y, 0.1f, 100.0f, projection);
		// Pre calc projection matrix
		game::my_PerspectiveFOV2(90.0f, resolution.x / (float_t)resolution.y, 0.1f, 100.0f, projMat);
		quad.reserve(1000);

		camera.position.z = -2.0f;
	}

	void Shutdown()
	{

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

		if (geKeyboard.WasKeyPressed(geK_1))
		{
			scene = 0;
		}

		if (geKeyboard.WasKeyPressed(geK_2))
		{
			scene = 1;
		}

		if (geKeyboard.WasKeyPressed(geK_3))
		{
			scene = 2;
		}

		float_t speed = 0.1f * msElapsed / 1000.0f;
		if (geKeyboard.IsKeyHeld(geK_SHIFT))
		{
			speed = 5.0f * msElapsed / 1000.0f;
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
		if (geKeyboard.IsKeyHeld(geK_Q))
		{
			camera.position -= camera.right * speed;
		}

		// strafe right
		if (geKeyboard.IsKeyHeld(geK_E))
		{
			camera.position += camera.right * speed;
		}

		// y is inverted because....
		if (geKeyboard.IsKeyHeld(geK_UP))
		{
			camera.position.y -= speed;
		}

		// move actually down
		if (geKeyboard.IsKeyHeld(geK_DOWN))
		{
			camera.position.y += speed;
		}

		game::Pointi mouse = geMouse.GetPositionRelative();
		if (mouse.x)
		{
			if (geMouse.IsButtonHeld(geM_LEFT))
			{
				camera.SetRotation(0.0f, mouse.x * (3.14159f / 180.0f), 0.0f);
			}
		}
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
		static float_t rotation = 0.0f;

		//rotation += (2 * 3.14f / 10.0f) * (msElapsed / 1000.0f);
		//software3D.time = rotation;
		geClear(GAME_FRAME_BUFFER_BIT, game::Colors::Blue);

		pixelMode.Clear(game::Colors::Black);
		software3D.renderTarget = pixelMode.videoBuffer;
		software3D.ClearDepth(100.0f);

		quad.clear();

		game::Matrix4x4f viewMat;
		game::Matrix4x4f mvpMat;

		viewMat = camera.GenerateView();
		mvpMat = projMat * viewMat;

		if (scene == 0)
		{
			test = topLeftTri;
			//test.faceNormal = test.faceNormal;
			//test.normals[0] = test.normals[0];
			//test.normals[1] = test.normals[1];
			//test.normals[2] = test.normals[2];
			test.vertices[0] = (topLeftTri.vertices[0] * mvpMat);
			test.vertices[1] = (topLeftTri.vertices[1] * mvpMat);
			test.vertices[2] = (topLeftTri.vertices[2] * mvpMat);

			PerspectiveDivide(test);
			ScaleToScreen(test, resolution);
			quad.emplace_back(test);

			test = bottomRightTri;
			//test.faceNormal = test.faceNormal;
			//test.normals[0] = test.normals[0];
			//test.normals[1] = test.normals[1];
			//test.normals[2] = test.normals[2];
			test.vertices[0] = bottomRightTri.vertices[0] * mvpMat;
			test.vertices[1] = bottomRightTri.vertices[1] * mvpMat;
			test.vertices[2] = bottomRightTri.vertices[2] * mvpMat;

			PerspectiveDivide(test);
			ScaleToScreen(test, resolution);
			quad.emplace_back(test);
		}

		if (scene == 1)
		{
			for (int i = 0; i < tris.size(); i++)
			{
				test = tris[i];
				//test.faceNormal = test.faceNormal;
				//test.normals[0] = test.normals[0];
				//test.normals[1] = test.normals[1];
				//test.normals[2] = test.normals[2];
				test.vertices[0] = (tris[i].vertices[0] * mvpMat);
				test.vertices[1] = (tris[i].vertices[1] * mvpMat);
				test.vertices[2] = (tris[i].vertices[2] * mvpMat);

				PerspectiveDivide(test);
				ScaleToScreen(test, resolution);
				quad.emplace_back(test);
			}
		}

		if (scene == 2)
		{
			for (int i = 0; i < model.tris.size(); i++)
			{
				test = model.tris[i];
				//test.faceNormal = test.faceNormal;
				//test.normals[0] = test.normals[0];
				//test.normals[1] = test.normals[1];
				//test.normals[2] = test.normals[2];
				test.vertices[0] = (model.tris[i].vertices[0] * mvpMat);
				test.vertices[1] = (model.tris[i].vertices[1] * mvpMat);
				test.vertices[2] = (model.tris[i].vertices[2] * mvpMat);

				//if (test.faceNormal.z > 0)  // somewhat works, but doesn't take account camera
				//{
				//	culled++;
				//	continue;
				//}				
	 
				if ((test.vertices[0].z < 0.0) ||
					(test.vertices[1].z < 0.0) ||
					(test.vertices[2].z < 0.0))
				{
					game::Vector3f planePoint(0.0f, 0.0f, 0.0f);
					game::Vector3f planeNormal(0.0f, 0.0f, 1.0f);

					game::Triangle out1;
					game::Triangle out2;
					uint32_t numtris = ClipAgainstPlane(planePoint, planeNormal, test, out1, out2);
					if (numtris == 2)
					{
						PerspectiveDivide(out2);
						ScaleToScreen(out2, resolution);

						if (CheckWinding(out2.vertices[0], out2.vertices[1], out2.vertices[2]) < 0)
						{
							std::swap(out2.vertices[1], out2.vertices[0]);
							std::swap(out2.normals[1], out2.normals[0]);
							std::swap(out2.uvs[1], out2.uvs[0]);
							std::swap(out2.color[1], out2.color[0]);
						}
						quad.emplace_back(out2);
					}
					PerspectiveDivide(out1);
					ScaleToScreen(out1, resolution);
					if (CheckWinding(out1.vertices[0], out1.vertices[1], out1.vertices[2]) < 0)
					{
						std::swap(out1.vertices[1], out1.vertices[0]);
						std::swap(out1.normals[1], out1.normals[0]);
						std::swap(out1.uvs[1], out1.uvs[0]);
						std::swap(out1.color[1], out1.color[0]);
					}

					quad.emplace_back(out1);
				}
				else
				{
					PerspectiveDivide(test);
					ScaleToScreen(test, resolution);
					quad.emplace_back(test);
				}
			}
			//std::cout << culled << "\n";
		}

		uint32_t fenceCount = 0;
		for (uint32_t c = 0; c < numclips; c++)
		{
			clippedTris[c].clear();
			software3D.ScreenClip(quad, clip[c], clippedTris[c]);
			if (!clippedTris[c].size()) continue;
			std::sort(clippedTris[c].begin(), clippedTris[c].end(), [](const game::Triangle& a, const game::Triangle& b) 
				{
					float_t az = a.vertices[0].z + a.vertices[1].z + a.vertices[2].z;
					float_t bz = b.vertices[0].z + b.vertices[1].z + b.vertices[2].z;
					return az < bz;
				});
			//pixelMode.Rect(clip[c], game::Colors::Yellow);
			software3D.Render(clippedTris[c], clip[c]);
			fenceCount += (uint32_t)clippedTris[c].size();
		}
		software3D.Fence(fenceCount);

		// show depth buffer 468
		if (geKeyboard.IsKeyHeld(geK_D))
		{

			game::Color dColor;
			float_t depth = 0.0f;
			float_t* zbuffer = software3D.depthBuffer;
			uint32_t* vbuffer = pixelMode.videoBuffer;
			for (int pos = 0; pos < pixelMode.GetPixelFrameBufferSize().y * pixelMode.GetPixelFrameBufferSize().x; pos++)
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

		if (showText)
		{
			//pixelMode.Text("Translate Z : " + std::to_string(tz), 0, 40, game::Colors::Yellow, 1);
			game::Pointi m = pixelMode.GetScaledMousePosition();
			float_t* zb = software3D.depthBuffer;
			float_t depthAtMouse = zb[(m.y * pixelMode.GetPixelFrameBufferSize().x + m.x)];
			pixelMode.Text("Depth at mouse: " + std::to_string(depthAtMouse), 0, 40, game::Colors::Yellow, 1);


			pixelMode.Text("FPS: " + std::to_string(geGetFramesPerSecond()), 0, 0, game::Colors::Yellow, 1);
			if (geGetFramesPerSecond() > maxFPS) maxFPS = geGetFramesPerSecond();
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

	void GenerateCheckerboard(uint32_t* buff, unsigned int w, unsigned int h)
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

	void ConvertBlenderToThis(game::Mesh& mesh)
	{
		for (int tri = 0; tri < mesh.tris.size(); tri++)
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