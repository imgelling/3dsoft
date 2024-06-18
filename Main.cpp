//#define GAME_SUPPORT_OPENGL
//#define GAME_SUPPORT_DIRECTX9
//#define GAME_SUPPORT_DIRECTX10
#define GAME_SUPPORT_DIRECTX11
//#define GAME_SUPPORT_DIRECTX12

#include "game.h"
#include "GameSoftware3D.h"

#include "GameSimpleUI.h"
#include "any"


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
			particles[light].size = { 0.05f, 0.05f };
			particles[light].position = { 0.0f,0.0f, 0.0f};
			//particles[light].position.w = 0;
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
				particles[light].position = lights[light].position;
				particles[light].color = lights[light].diffuse;
				particles[light].alive = true;
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
	//game::Mesh model;
	Lights lights;
	game::Mesh room;

	// UI
	game::SimpleUI simpleUI;
	game::ButtonUI textureButton;
	game::ButtonUI lightingButton;
	game::RadialUI lightingDepthRadial;
	game::RadialUI lightingFaceRadial;
	game::RadialUI lightingVertexRadial;
	game::RadialUI lightingPointRadial;
	game::CheckBoxUI backFaceCullingCheckBox;
	game::SliderUI pointLightConstSlider;
	game::SliderUI pointLightLinearSlider;
	game::SliderUI pointLightQuadraticSlider;
	game::Software3DStateObject uiStateObject;

	game::Camera3D camera;
	uint32_t maxFPS;

	int32_t lightColor = 0;


	game::FillMode state = game::FillMode::Filled;
	const game::Pointi resolution = { 1280 , 720 };
	bool showText;

	Game() : game::Engine()
	{
		maxFPS = 0;
		showText = true;
	}


	void simpleUICallBack(const std::string& str, std::any& value)
	{
		if (str == "TextureButton")
		{
			bool rec = false;
			//try
			//{
			//	//std::cout << str << " sent " << std::any_cast<bool>(value) << " as value" << ".\n";
			//	rec = std::any_cast<bool>(value);
			//	
			//}
			//catch (...) //const std::bad_any_cast& e) 
			//{
			//	std::cout << str << " sent an INVALID VALUE.\n";// << e.what() << '\n';
			//	return;
			//}
#if defined(_DEBUG)
			UI_VALUE_CHECK(str, "bool", value);
#endif
			rec = std::any_cast<bool>(value);
			software3D.SetState(GAME_SOFTWARE3D_TEXTURE, rec);
			uiStateObject.texturing = rec;
			return;
		}

		if (str == "BackFaceCullingCheckBox")
		{
			bool rec = false;
#if defined(_DEBUG)
			UI_VALUE_CHECK(str, "bool", value);
#endif
			rec = std::any_cast<bool>(value);
			software3D.SetState(GAME_SOFTWARE3D_BACKFACECULL, rec);
			uiStateObject.backFaceCulling = rec;
			return;
		}

		if (str == "LightingButton")
		{
			bool rec = false;
#if defined(_DEBUG)
			UI_VALUE_CHECK(str, "bool", value);
#endif
			rec = std::any_cast<bool>(value);
			software3D.SetState(GAME_SOFTWARE3D_LIGHTING, rec);
			uiStateObject.lighting = rec;
			return;
		}

		if (str == "PointLightConstSlider")
		{
#if defined(_DEBUG)
			UI_VALUE_CHECK(str, "float", value);
#endif
			lights.lights[0].attenuation.constant = std::any_cast<float>(value);
			lights.lights[0].CalculateRadius();
			return;
		}

		if (str == "PointLightLinearSlider")
		{
#if defined(_DEBUG)
			UI_VALUE_CHECK(str, "float", value);
#endif
			lights.lights[0].attenuation.linear = std::any_cast<float>(value);
			lights.lights[0].CalculateRadius();
			return;
		}

		if (str == "PointLightQuadraticSlider")
		{
#if defined(_DEBUG)
			UI_VALUE_CHECK(str, "float", value);
#endif
			lights.lights[0].attenuation.quadratic = std::any_cast<float>(value);
			lights.lights[0].CalculateRadius();
			//pointLightExponentialSlider.value = lights.lights[0].attenuation.exponential;
			return;
		}

		if (str == "LightingDepthRadial")
		{
			// Radials will always be true
			software3D.SetState(GAME_SOFTWARE3D_LIGHTING_TYPE, game::LightingType::Depth);
			uiStateObject.lightingType = game::LightingType::Depth;
			lightingFaceRadial.checked = false;
			lightingVertexRadial.checked = false;
			lightingPointRadial.checked = false;
			return;
		}

		if (str == "LightingFaceRadial")
		{
			software3D.SetState(GAME_SOFTWARE3D_LIGHTING_TYPE, game::LightingType::Face);
			uiStateObject.lightingType = game::LightingType::Face;
			lightingDepthRadial.checked = false;
			lightingVertexRadial.checked = false;
			lightingPointRadial.checked = false;
			return;
		}

		if (str == "LightingVertexRadial")
		{
			software3D.SetState(GAME_SOFTWARE3D_LIGHTING_TYPE, game::LightingType::Vertex);
			uiStateObject.lightingType = game::LightingType::Vertex;
			lightingDepthRadial.checked = false;
			lightingFaceRadial.checked = false;
			lightingPointRadial.checked = false;
			return;
		}

		if (str == "lightingPointRadial")
		{
			software3D.SetState(GAME_SOFTWARE3D_LIGHTING_TYPE, game::LightingType::Point);
			uiStateObject.lightingType = game::LightingType::Point;
			lightingDepthRadial.checked = false;
			lightingFaceRadial.checked = false;
			lightingVertexRadial.checked = false;
			return;
		}
		;
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
		lights.InitializeLights(3);
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

		software3D.SetState(GAME_SOFTWARE3D_FILL_MODE, state);

		//if (!LoadObj("Content/coney.obj", model))
		//{
		//	std::cout << "Could not load model\n";
		//}
		
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
		software3D.LoadTexture("content/particle1.png", lights.mesh.texture);
		//software3D.LoadTexture("content/sky.png", room.texture);
		//text.texture = lights.mesh.texture;
		//model.texture = lights.mesh.texture;
		

		// Preset some world stuff
		camera.position.z = -2.0f;
		camera.position.y = 0;


		// Pre calc projection matrix
		game::my_PerspectiveFOV2(70.0f, resolution.x / (float_t)resolution.y, 0.1f, 100.0f, projMat);

		// Text stuff
		geKeyboard.SetTextInputText("3D text");

		// State stuff
		uiStateObject.texturing = true;
		uiStateObject.lighting = false;
		uiStateObject.lightingType = game::LightingType::Face;
		uiStateObject.multiThreaded = true;

		// Simple UI
		int uiScaleX = (resolution.x / 1280);


		simpleUI.Initialize(pixelMode, std::bind(&Game::simpleUICallBack, this, std::placeholders::_1, std::placeholders::_2));

		textureButton.label = "Texture";
		textureButton.name = "TextureButton";
		textureButton.toggledColor = game::Colors::Green;
		textureButton.unToggledColor = game::Colors::Red;
		//textureButton.labelColor = game::Colors::White;
		textureButton.toggled = uiStateObject.texturing;
		textureButton.position.x = 1100 * uiScaleX;
		textureButton.position.y = 20;
		textureButton.length = 100;
		textureButton.outlined = true;

		lightingButton.label = "Lighting";
		lightingButton.name = "LightingButton";
		lightingButton.toggledColor = game::Colors::Green;
		lightingButton.unToggledColor = game::Colors::Red;
		lightingButton.toggled = uiStateObject.lighting;
		lightingButton.position.x = 1100 * uiScaleX;
		lightingButton.position.y = 40;
		lightingButton.length = 100;
		lightingButton.outlined = true;

		lightingDepthRadial.position.x = 1100 * uiScaleX;
		lightingDepthRadial.position.y = 60;
		lightingDepthRadial.label = "Depth Lighting";
		lightingDepthRadial.name = "LightingDepthRadial";
		lightingDepthRadial.labelColor = game::Colors::White;

		lightingFaceRadial.position.x = 1100 * uiScaleX;
		lightingFaceRadial.position.y = 80;
		lightingFaceRadial.label = "Face Lighting";
		lightingFaceRadial.name = "LightingFaceRadial";
		lightingFaceRadial.labelColor = game::Colors::White;
		lightingFaceRadial.checked = true;

		lightingVertexRadial.position.x = 1100 * uiScaleX;
		lightingVertexRadial.position.y = 100;
		lightingVertexRadial.label = "Vertex Lighting";
		lightingVertexRadial.name = "LightingVertexRadial";
		lightingVertexRadial.labelColor = game::Colors::White;
		//lightingVertexRadial.scale = 1;

		
		lightingPointRadial.position.x = 1100 * uiScaleX;
		lightingPointRadial.position.y = 120;
		lightingPointRadial.label = "Point Lighting";
		lightingPointRadial.name = "lightingPointRadial";
		lightingPointRadial.labelColor = game::Colors::White;


		lights.lights[0].attenuation.constant = 1.0f;
		lights.lights[0].attenuation.linear = 0.7f;
		lights.lights[0].attenuation.quadratic = 1.8f;
		lights.lights[0].diffuse = game::Colors::Red;
		lights.lights[0].CalculateRadius();

		pointLightConstSlider.position.x = 1100 * uiScaleX;
		pointLightConstSlider.position.y = 140;
		pointLightConstSlider.name = "PointLightConstSlider";
		pointLightConstSlider.label = "Fall Off - Constant";
		pointLightConstSlider.labelColor = game::Colors::White;
		pointLightConstSlider.value = lights.lights[0].attenuation.constant;// .0f;
		pointLightConstSlider.minValue = 0;// .0f;
		pointLightConstSlider.maxValue = 10;// .0f;
		pointLightConstSlider.length = 17*8;

		pointLightLinearSlider.position.x = 1100 * uiScaleX;
		pointLightLinearSlider.position.y = 170;
		pointLightLinearSlider.name = "PointLightLinearSlider";
		pointLightLinearSlider.label = "Fall Off - Linear";
		pointLightLinearSlider.labelColor = game::Colors::White;
		pointLightLinearSlider.value = lights.lights[0].attenuation.linear;// .0f;
		pointLightLinearSlider.minValue = 0;// .0f;
		pointLightLinearSlider.maxValue = 10;// .0f;
		pointLightLinearSlider.length = 17 * 8;

		pointLightQuadraticSlider.position.x = 1100 * uiScaleX;
		pointLightQuadraticSlider.position.y = 200;
		pointLightQuadraticSlider.name = "PointLightQuadraticSlider";
		pointLightQuadraticSlider.label = "Fall Off - Quadradic";
		pointLightQuadraticSlider.labelColor = game::Colors::White;
		pointLightQuadraticSlider.value = lights.lights[0].attenuation.quadratic;// .0f;
		pointLightQuadraticSlider.minValue = 0;// .0f;
		pointLightQuadraticSlider.maxValue = 10;// .0f;
		pointLightQuadraticSlider.length = 17 * 8;
		
		backFaceCullingCheckBox.position.x = 1100 * uiScaleX;
		backFaceCullingCheckBox.position.y = 230;
		backFaceCullingCheckBox.name = "BackFaceCullingCheckBox";
		backFaceCullingCheckBox.label = "BackFace Culling";
		backFaceCullingCheckBox.checked = true;
		backFaceCullingCheckBox.labelColor = game::Colors::White;

		simpleUI.Add(&textureButton);
		simpleUI.Add(&lightingButton);
		simpleUI.Add(&lightingDepthRadial);
		simpleUI.Add(&lightingFaceRadial);
		simpleUI.Add(&lightingVertexRadial);
		simpleUI.Add(&lightingPointRadial);
		simpleUI.Add(&backFaceCullingCheckBox);
		simpleUI.Add(&pointLightConstSlider);
		simpleUI.Add(&pointLightLinearSlider);
		simpleUI.Add(&pointLightQuadraticSlider);


		//game::GeneratePlane(room, { 0,0,0 }, 1, game::Colors::White);
		game::GenerateCube(room, { 0,0,0 }, game::Colors::Black);
		//game::GenerateUVSphere(room, 10, 20, { 0,0,0 }, game::Colors::White);
		//game::GenerateCylinder(room, 0.0f, 0.5f, 20, 1, { 0,0,0 }, game::Colors::White);
		room.SetScale(2.00f, 2.00f, 2.00f);
		// invert model
		for (uint32_t c = 0; c < room.tris.size(); c++)
		{
			room.tris[c].faceNormal *= -1.0f;
			room.tris[c].faceNormal.Normalize();
			room.tris[c].normals[0] *= -1.0f;
			room.tris[c].normals[1] *= -1.0f;
			room.tris[c].normals[2] *= -1.0f;
			std::swap(room.tris[c].vertices[1], room.tris[c].vertices[2]);
			std::swap(room.tris[c].uvs[1], room.tris[c].uvs[2]);
			std::swap(room.tris[c].normals[1], room.tris[c].normals[2]);
			std::swap(room.tris[c].faceNormal, room.tris[c].faceNormal);
		}


	}

	void Shutdown()
	{
		//software3D.DeleteRenderTarget(renderTarget);
		//software3D.DeleteTexture(sky.texture);
		software3D.DeleteTexture(room.texture);
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
			lightColor--;
			lightColor = max(0, lightColor);
			switch (lightColor)
			{
			case 0: lights.lights[0].diffuse = game::Colors::Red; break;
			case 1: lights.lights[0].diffuse = game::Colors::Green; break;
			case 2: lights.lights[0].diffuse = game::Colors::Blue; break;
			}
		}

		if (geKeyboard.WasKeyReleased(geK_PERIOD))
		{
			lightColor++;
			lightColor = min(2, lightColor);
			switch (lightColor)
			{
			case 0: lights.lights[0].diffuse = game::Colors::Red; break;
			case 1: lights.lights[0].diffuse = game::Colors::Green; break;
			case 2: lights.lights[0].diffuse = game::Colors::Blue; break;
			}
		}

		if (geKeyboard.WasKeyPressed(geK_F1))
		{
			state++;
			software3D.SetState(GAME_SOFTWARE3D_FILL_MODE, state);
		}

		if (geKeyboard.WasKeyPressed(geK_LBRACKET))
		{
			software3D.SetState(GAME_SOFTWARE3D_THREADED, -1);
			clip.SetNumberOfClipsRects(1);
			uiStateObject.multiThreaded = false;
			clip.GenerateClips(resolution);
		}

		if (geKeyboard.WasKeyPressed(geK_RBRACKET))
		{
			software3D.SetState(GAME_SOFTWARE3D_THREADED, 0);
			uiStateObject.multiThreaded = true;
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
				if (geKeyboard.IsKeyHeld(geK_SPACE))
				{
					lights.lights[0].position.z += speed;
				}
				else
				camera.position += (camera.forward * speed);
			}

			// Move backward
			if (geKeyboard.IsKeyHeld(geK_S))
			{
				if (geKeyboard.IsKeyHeld(geK_SPACE))
				{
					lights.lights[0].position.z -= speed;
				}
				else
				camera.position -= (camera.forward * speed);
			}

			// strafe left
			if (geKeyboard.IsKeyHeld(geK_A))
			{
				if (geKeyboard.IsKeyHeld(geK_SPACE))
				{
					lights.lights[0].position.x -= speed;
				}
				else
				camera.position -= camera.right * speed;
			}

			// strafe right
			if (geKeyboard.IsKeyHeld(geK_D))
			{
				if (geKeyboard.IsKeyHeld(geK_SPACE))
				{
					lights.lights[0].position.x += speed;
				}
				else
				camera.position += camera.right * speed;
			}

			// y is inverted because.... we are in Q4
			if (geKeyboard.IsKeyHeld(geK_UP))
			{
				if (geKeyboard.IsKeyHeld(geK_SPACE))
				{
					lights.lights[0].position.y -= speed;
				}
				else
				camera.position.y -= speed;
			}

			// move actually down
			if (geKeyboard.IsKeyHeld(geK_DOWN))
			{
				if (geKeyboard.IsKeyHeld(geK_SPACE))
				{
					lights.lights[0].position.y += speed;
				}
				else
				camera.position.y += speed;
			}
		}

		if (!simpleUI.inputWasUsed)
		{
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


	// Generate points (particles/etc)
	// torus 
	// Needs a data structure
	void GenerateTextMesh(game::Mesh& mesh, const std::string& text, const game::Vector3f& __restrict pos, const bool centerX, const bool centerY, float_t value, game::Color color)  noexcept
	{
		static std::string old;
		//if (text == old) return;
		old = text;
		float_t z = pos.z;
		const float_t size = 0.5f;
		const float_t sizeX2 = size * 2.0f;
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
						//game::GenerateCube(cube, p, color);
						//game::GenerateUVSphere(cube, 10, 20, p, color);
						//GenerateCylinder(cube, size, size, 10, 0.5f, p, color);
						GeneratePlane(cube, p, 1, color);
						const uint64_t size = cube.tris.size();
						for (uint32_t i = 0; i < size; ++i)
						{
							mesh.tris.emplace_back(cube.tris[i]);
						}

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

		//if (geKeyboard.IsKeyHeld(geK_R)) 
			rotation += (1 *  3.14f / 10.0f) * (msElapsed / 1000.0f);

		pos += 1.5f * (msElapsed / 1000.0f);

		geClear(GAME_FRAME_BUFFER_BIT, game::Colors::Blue);

		pixelMode.Clear(game::Colors::Black);
		software3D.ClearDepth(100.0f);


		camera.GenerateViewMatrix();
		
		mvpMat = projMat * camera.view; // not sure if this should be in the RenderMesh


		software3D.SetStateObject(uiStateObject);
		 
		//room.SetRotation(rotation, -rotation, 2.0f * rotation);
		software3D.RenderMesh(room, room.tris.size(), mvpMat, camera, clip);


		software3D.SetState(GAME_SOFTWARE3D_ALPHA_TEST, true);
		software3D.SetState(GAME_SOFTWARE3D_TEXTURE, true);
		software3D.SetState(GAME_SOFTWARE3D_LIGHTING, false);
		software3D.SetState(GAME_SOFTWARE3D_COLOR_TINTING, true);

		lights.lights[1].diffuse = game::Colors::Green;
		lights.lights[2].diffuse = game::Colors::Blue;

		//lights.lights[0].diffuse = game::Colors::Blue;
		//lights.lights[0].position = { 0.75f,0.0f,0.75f };
		//lights.lights[0].position = game::RotateX(lights.lights[0].position, (1 * 3.14f / 10.0f) * (msElapsed / 1000.0f));
		//lights.lights[0].position = game::RotateY(lights.lights[0].position, -(1 * 3.14f / 10.0f) * (msElapsed / 1000.0f));
		//lights.lights[0].position = game::RotateZ(lights.lights[0].position, (1 * 3.14f / 10.0f) * (msElapsed / 1000.0f) * 0.75f);
		lights.Update();
		software3D.lights = lights.lights;
		lights.GeneratePointSpriteMatrix(camera);
		lights.GenerateQuads();
		software3D.RenderMesh(lights.mesh, lights.particlesAlive << 1, mvpMat, camera, clip);

		//// show depth buffer
		//if (!geKeyboard.IsTextInput() && geKeyboard.IsKeyHeld(geK_SPACE))
		//{
		//	game::Color dColor;
		//	float_t depth = 0.0f;
		//	float_t* zbuffer = software3D.depthBuffer;
		//	uint32_t* vbuffer = pixelMode.videoBuffer;
		//	for (int pos = 0; pos < resolution.height * resolution.width; pos++)
		//	{
		//		depth = *zbuffer;
		//		zbuffer++;
		//		depth += 1.0f;  // 1 added because z becomes < 1.0f near camera and makes depth > 1.0 making colors
		//						// go all weird
		//		depth = 1.0f/depth;
		//		dColor.Set(depth, depth, depth, 1.0f);
		//		*vbuffer = dColor.packedABGR;
		//		vbuffer++;
		//	}
		//	if (showText)
		//		pixelMode.Text("Showing Depth buffer.", 0, 60, game::Colors::Magenta, 1);
		//}


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
				pixelMode.Text("Text Input Mode: True", 0, 50, game::Colors::Magenta, 1);
			}
			else
			{
				pixelMode.Text("Text Input Mode: False", 0, 50, game::Colors::Magenta, 1);
			}
			// Update and render UI
			simpleUI.Update();
			simpleUI.Draw();
		}
		//pixelMode.TextClip("rotation is " + std::to_string(rotation / 3.14159f), 0, 70, game::Colors::Magenta, 2);


	
		// Gamma
		if (geKeyboard.IsKeyHeld(geK_G))
		{
			game::Color fc;
			uint32_t c = 0;
			uint32_t* v = pixelMode.videoBuffer;
			float r = 0;
			float g = 0;
			float b = 0;
			float a = 0;
			for (int pos = 0; pos < resolution.height * resolution.width; pos++)
			{
				c = *v;
				r = ((c >> 0) & 0xFF) * (1.0f / 255.0f);
				g = ((c >> 8) & 0xFF) * (1.0f / 255.0f);
				b = ((c >> 16) & 0xFF) * (1.0f / 255.0f);
				a = ((c >> 24) & 0xFF) * (1.0f / 255.0f);
				r = pow(r, 1.0f/2.2f);
				b = pow(b, 1.0f/2.2f);
				g = pow(g, 1.0f/2.2f);
				r = min(1.0f, r);
				g = min(1.0f, g);
				b = min(1.0f, b);
				*v = (uint32_t(a * 255) << 24) | (uint32_t(b * 255) << 16) | (uint32_t(g * 255) << 8) | uint32_t(r * 255);
				v++;
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

	game::Software3DStateObject sso = {};


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