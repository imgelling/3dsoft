//#define GAME_SUPPORT_OPENGL
//#define GAME_SUPPORT_DIRECTX9
//#define GAME_SUPPORT_DIRECTX10
#define GAME_SUPPORT_DIRECTX11
//#define GAME_SUPPORT_DIRECTX12

#include "game.h"
#include "GameSoftware3D.h"


class Lights : public game::EmitterBase
{
public:
	std::vector<game::Light> lights;
	void InitializeLights(const uint32_t numLights)
	{
		numberOfParticles = numLights;

		Initialize(numLights, { 0,0,0 });
		for (uint32_t light = 0; light < numberOfParticles; light++)
		{
			particles[light].size = { 1.0f, 1.0f };
			particles[light].position = { 0.0f,0.0f, 0.9f};
			particles[light].position.w = 0;
			particles[light].alive = true;
			game::Light pointLight;
			pointLight.diffuse = { 1.0f, 0.0f, 0.0f, 1.0f };
			pointLight.position = particles[light].position; // needs to be in camera/view space
			pointLight.specular = { 0.0f, 1.0f, 0.0f, 1.0f };
			lights.emplace_back(pointLight);
		}
	}
	void Update()
	{
		particlesAlive = 0;
		if (particles.size())
		{
			for (uint32_t light = 0; light < numberOfParticles; light++)
			{
				//particles[light].size = { 1.0f, 1.0f };
				//particles[light].position = { 0.0f,0.0f, 0.9f };
				//particles[light].alive = true;
				particles[light].color = game::Colors::White;
				particlesAlive++;
			}
		}
	}
};


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
	Lights lights;
	//game::Mesh torus;
	//game::Mesh sky;
	game::Mesh text;


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
		lights.InitializeLights(1);
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
		software3D.LoadTexture("content/sky.png", lights.mesh.texture);
		text.texture = lights.mesh.texture;
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
		camera.position.y = 0;

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

		// Text stuff
		geKeyboard.SetTextInputText("3D text");
	}

	void Shutdown()
	{
		//software3D.DeleteRenderTarget(renderTarget);
		//software3D.DeleteTexture(sky.texture);
		software3D.DeleteTexture(model.texture);
		software3D.DeleteTexture(lights.mesh.texture);
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
			clip.SetNumberOfClipsRects(24);
			clip.GenerateClips(resolution);
		}

		if (!geKeyboard.IsTextInput())
		{
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

		if (geKeyboard.WasKeyPressed(geK_F8))
		{
			geKeyboard.TextInputMode(true);
		}

		if (geKeyboard.WasKeyPressed(geK_F9))
		{
			geKeyboard.TextInputMode(false);
		}
	}	


	// Generate plane, sphere, cubes, points (particles/etc)
	// Cone, cylindar, torus 
	// Needs a data structure


	// Insert vertices in the order of your triangle
	inline game::Vector3f GenerateFaceNormal(game::Vector3f& __restrict A, game::Vector3f& __restrict B, game::Vector3f& __restrict C)
	{
		game::Vector3f AC = C - A; // Vector from A to C
		game::Vector3f AB = B - A; // Vector from A to B
		game::Vector3f N = AC.Cross(AB); // Cross product of AB and AC
		N.Normalize();
		return N;
	}

	// Insert vertices in the order of your triangle
	inline void GenerateFaceNormal(game::Vector3f& __restrict A, game::Vector3f& __restrict B, game::Vector3f& __restrict C, game::Vector3f& __restrict out)
	{
		game::Vector3f AC = C - A; // Vector from A to C
		game::Vector3f AB = B - A; // Vector from A to B
		out = AC.Cross(AB); // Cross product of AB and AC
		out.Normalize();
	}

	// Function to create a UV sphere
	// needs uvs
	void GenerateUVSphere(game::Mesh& mesh, const uint32_t stacks, const uint32_t slices, const game::Vector3f& __restrict pos, const game::Color color) 
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

				// First triangle of quad
				tri.vertices[0] = v1 + pos; //bl
				tri.normals[0] = v1;
				tri.uvs[0].u = atan2f(v1.z, v1.x) / (2.0f * 3.14159f) + 0.5f;
				tri.uvs[0].v = asin(v1.y) / 3.14159f + 0.5f;

				tri.vertices[1] = v3 + pos; // tl
				tri.normals[1] = v3;
				tri.uvs[1].u = atan2f(v3.z, v3.x) / (2.0f * 3.14159f) + 0.5f;
				tri.uvs[1].v = asin(v3.y) / 3.14159f + 0.5f;

				tri.vertices[2] = v2 + pos; //br
				tri.normals[2] = v2;
				tri.uvs[2].u = atan2f(v2.z, v2.x) / (2.0f * 3.14159f) + 0.5f;
				tri.uvs[2].v = asin(v2.y) / 3.14159f + 0.5f;

				GenerateFaceNormal(v1, v3, v2, tri.faceNormal);

				mesh.tris.emplace_back(tri);

				// Second triangle of quad
				tri.vertices[0] = v2 + pos;
				tri.normals[0] = v2;
				tri.uvs[0].u = atan2f(v2.z, v2.x) / (2.0f * 3.14159f) + 0.5f;
				tri.uvs[0].v = asin(v2.y) / 3.14159f + 0.5f;

				tri.vertices[1] = v3 + pos;
				tri.normals[1] = v3;
				tri.uvs[1].u = atan2f(v3.z, v3.x) / (2.0f * 3.14159f) + 0.5f;
				tri.uvs[1].v = asin(v3.y) / 3.14159f + 0.5f;

				tri.vertices[2] = v4 + pos;
				tri.normals[2] = v4;
				tri.uvs[2].u = atan2f(v4.z, v4.x) / (2.0f * 3.14159f) + 0.5f;
				tri.uvs[2].v = asin(v4.y) / 3.14159f + 0.5f;

				GenerateFaceNormal(v2, v3, v4, tri.faceNormal);

				mesh.tris.emplace_back(tri);

			}
		}
	}

	void GenerateCube(game::Mesh& mesh, const float_t size, const game::Vector3f& __restrict pos, const game::Color& color)  noexcept
	{
		mesh.tris.clear();
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
	
	void GenerateTextMesh(game::Mesh& mesh, const std::string& text, const game::Vector3f& __restrict pos, const bool centerX, const bool centerY, float_t value, game::Color color)  noexcept
	{
		static std::string old;
		if (text == old) return;
		old = text;
		float_t z = pos.z;
		const float_t size = 0.5f;
		const float_t sizeX2 = size * 2.0f;
		const game::Vector3f normal = { 0.0f,0.0f,-1.0f };
		float_t px = pos.x; 
		float_t py = pos.y;
		if (centerX)
		{
			px -= (float_t)(text.length() << 2) * sizeX2;
		}
		if (centerY)
		{
			py -= 4.0f * sizeX2;
		}
		int32_t ox = 0;
		int32_t oy = 0;

		mesh.tris.clear();

		game::Triangle topLeftTri;
		game::Triangle bottomRightTri;

		game::Mesh cube;
		for (uint8_t letter : text)
		{
			ox = (letter - 32) % 16;
			oy = (letter - 32) >> 4;
			for (uint32_t i = 0; i < 8; i++)
			{
				for (uint32_t j = 0; j < 8; j++)
				{
					if (pixelMode._fontROM[((j + (oy << 3)) << 7) + (i + (ox << 3))] > 0)
					{
						
						const float_t pxi = px + (i * sizeX2);
						const float_t pyj = py + (j * sizeX2);
						const game::Vector3f p = { pxi, pyj, 0 };
						//GenerateCube(cube, size, p, color);
						GenerateUVSphere(cube, 5, 5, p, color);
						for (int i = 0; i < cube.tris.size(); i++)
						{
							mesh.tris.emplace_back(cube.tris[i]);
						}
						// tl
						//topLeftTri.vertices[0].x = -size + pxi;
						//topLeftTri.vertices[0].y = -size + pyj;
						//topLeftTri.vertices[0].z = z;
						//topLeftTri.color[0] = game::Colors::Red;
						//topLeftTri.uvs[0].u = 0.0f;
						//topLeftTri.uvs[0].v = 0.0f;
						//topLeftTri.faceNormal = normal;

						//// tr
						//topLeftTri.vertices[1].x = size + pxi;
						//topLeftTri.vertices[1].y = -size + pyj;
						//topLeftTri.vertices[1].z = z;
						//topLeftTri.uvs[1].u = 1.0f;
						//topLeftTri.uvs[1].v = 0.0f;
						//topLeftTri.color[1] = game::Colors::Green;

						//// bl
						//topLeftTri.vertices[2].x = -size + pxi;
						//topLeftTri.vertices[2].y = size + pyj;
						//topLeftTri.vertices[2].z = z;
						//topLeftTri.uvs[2].u = 0.0f;
						//topLeftTri.uvs[2].v = 1.0f;
						//topLeftTri.color[2] = game::Colors::Blue;

						//// tr
						//bottomRightTri.vertices[0].x = size + pxi;
						//bottomRightTri.vertices[0].y = -size + pyj;
						//bottomRightTri.vertices[0].z = z;
						//bottomRightTri.color[0] = game::Colors::Green;
						//bottomRightTri.uvs[0].u = 1.0f;
						//bottomRightTri.uvs[0].v = 0.0f;
						//bottomRightTri.faceNormal = normal;

						//// br
						//bottomRightTri.vertices[1].x = size + pxi;
						//bottomRightTri.vertices[1].y = size + pyj;
						//bottomRightTri.vertices[1].z = z;
						//bottomRightTri.uvs[1].u = 1.0f;
						//bottomRightTri.uvs[1].v = 1.0f;
						//bottomRightTri.color[1] = game::Colors::White;

						//// bl
						//bottomRightTri.vertices[2].x = -size + pxi;
						//bottomRightTri.vertices[2].y = size + pyj;
						//bottomRightTri.vertices[2].z = z;
						//bottomRightTri.uvs[2].u = 0.0f;
						//bottomRightTri.uvs[2].v = 1.0f;
						//bottomRightTri.color[2] = game::Colors::Blue;

						//for (uint32_t i = 0; i < 3; i++)
						//{
						//	topLeftTri.normals[i] = normal;// { 0.0f, 0.0f, -1.0f };
						//	bottomRightTri.normals[i] = normal;// { 0.0f, 0.0f, -1.0f };
						//	//topLeftTri.vertices[i] -= {pxi, pyj, z};
						//	//topLeftTri.vertices[i] = game::RotateX(topLeftTri.vertices[i], value);
						//	//topLeftTri.vertices[i] = game::RotateY(topLeftTri.vertices[i], -value);
						//	//topLeftTri.vertices[i] = game::RotateZ(topLeftTri.vertices[i], value - 3.14159f);
						//	//topLeftTri.vertices[i] += {pxi , pyj, z};
						//	//bottomRightTri.vertices[i] -= {pxi, pyj, z};
						//	//bottomRightTri.vertices[i] = game::RotateX(bottomRightTri.vertices[i], value);
						//	//bottomRightTri.vertices[i] = game::RotateY(bottomRightTri.vertices[i], -value);
						//	//bottomRightTri.vertices[i] = game::RotateZ(bottomRightTri.vertices[i], value - 3.14159f);
						//	//bottomRightTri.vertices[i] += {pxi, pyj, z};
						//}
						////for (uint32_t i = 0; i < 3; i++)
						////{
						////	topLeftTri.vertices[i] -= {px + i, py + j, 0};
						////	bottomRightTri.vertices[i] -= {px + i, py + j, 0};
						////}
						////topLeftTri = game::RotateZ(topLeftTri, value);
						////bottomRightTri = game::RotateZ(bottomRightTri, value);
						////for (uint32_t i = 0; i < 3; i++)
						////{
						////	topLeftTri.vertices[i] += {px + i, py + j, 0};
						////	bottomRightTri.vertices[i] += {px + i, py + j, 0};
						////}
						//mesh.tris.emplace_back(topLeftTri);
						//mesh.tris.emplace_back(bottomRightTri);
					}
				}
			}
			px += 8 * sizeX2;
		}
	}


	void Render(const float_t msElapsed)
	{
		static float_t rotation = 0.0f;
		static float_t pos = 0.0f;

		rotation += (2 * 3.14f / 10.0f) * (msElapsed / 1000.0f);

		pos += 1.5f * (msElapsed / 1000.0f);

		geClear(GAME_FRAME_BUFFER_BIT, game::Colors::Blue);

		pixelMode.Clear(game::Colors::CornFlowerBlue);
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
		software3D.SetState(GAME_SOFTWARE3D_LIGHTING_TYPE, game::LightingType::Face);
		software3D.SetState(GAME_SOFTWARE3D_TEXTURE, true);
		//software3D.SetState(GAME_SOFTWARE3D_DEPTH_WRITE, false);
		//software3D.SetState(GAME_SOFTWARE3D_SORT, game::SortingType::BackToFront);
		software3D.SetState(GAME_SOFTWARE3D_BACKFACECULL, true); // changed
		//software3D.SetState(GAME_SOFTWARE3D_ALPHA_BLEND, true);
		//software3D.SetState(GAME_SOFTWARE3D_ALPHA_TEST, true);
		//software3D.SetState(GAME_SOFTWARE3D_COLOR_TINTING, true);
		//software3D.RenderMesh(model, model.tris.size(), mvpMat, camera, clip);	
		
		//GenerateTextMesh(text, geKeyboard.GetTextInput(), {0 ,0, 0}, true, true, rotation, game::Colors::DarkRed);
		GenerateUVSphere(text, 10, 20, { 0,0,0 },game::Colors::White);
		text.SetScale(0.05f, 0.05f, 0.05f);
		
		//text.SetTranslation((geKeyboard.GetTextInput().length() * -0.5f) * 0.05f, 0, 0);
		text.SetRotation(0, rotation, 0);
		//GenerateCube(text, 0.5f, { 0,0,0 });
		//GenerateUVSphere(text, 20, 40);
		software3D.RenderMesh(text, text.tris.size(), mvpMat, camera, clip);


		//software3D.SetState(GAME_SOFTWARE3D_TEXTURE, true);
		////software3D.SetState(GAME_SOFTWARE3D_DEPTH_WRITE, false);
		//software3D.SetState(GAME_SOFTWARE3D_LIGHTING, false);
		//software3D.SetState(GAME_SOFTWARE3D_COLOR_TINTING, true);
		//software3D.SetState(GAME_SOFTWARE3D_BACKFACECULL, false);
		//software3D.SetState(GAME_SOFTWARE3D_SORT, game::SortingType::BackToFront);
		//software3D.SetState(GAME_SOFTWARE3D_ALPHA_BLEND, true);
		//software3D.SetState(GAME_SOFTWARE3D_ALPHA_TEST, true);

		//lights.Update();
		//lights.GeneratePointSpriteMatrix(camera);
		//lights.GenerateQuads();
		//software3D.RenderMesh(lights.mesh, lights.particlesAlive << 1, mvpMat, camera, clip);

		// show depth buffer
		if (!geKeyboard.IsTextInput() && geKeyboard.IsKeyHeld(geK_SPACE))
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
				dColor.Set(depth, depth, depth, 1.0f);
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
			if (geKeyboard.IsTextInput())
			{
				pixelMode.Text("Text Input Mode: True", 0, 50, game::Colors::Magenta, 2);
			}
			else
			{
				pixelMode.Text("Text Input Mode: False", 0, 50, game::Colors::Magenta, 2);
			}
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