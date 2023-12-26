#include <stdint.h>
#include "game.h"

class Game : public game::Engine
{

public:

	Game() : game::Engine()
	{
	}

	void Initialize()
	{
		game::Attributes attributes;

		attributes.WindowTitle = "Window Title";
		attributes.VsyncOn = true;
		geSetAttributes(attributes);

	}

	void LoadContent()
	{
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
	}

	void Render(const float_t msElapsed)
	{
		geClear(GAME_FRAME_BUFFER_BIT, game::Colors::Blue);

	}
};

int32_t main()
{
	game::Logger logger("Log.html");
	Game engine;
	engine.geSetLogger(&logger);

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