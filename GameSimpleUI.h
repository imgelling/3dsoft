#if !defined(GAMESIMPLEUI_H)
#define GAMESIMPLEUI_H

#include <any>
#include "GameColor.h"
#include "GameMath.h"
#include "GamePixelMode.h"

// simpleui factory
// init with pixelmode and function call back
// updates/renders all
// Everything derives from element
// Add* poops out a smart pointer for said element

namespace game
{
	class ButtonUI
	{
	public:
		ButtonUI();
		Pointi position;
		uint32_t length;
		uint32_t scale;
		std::string label;
		const uint32_t textHeight = 8;

		Color textColor = game::Colors::Black;
		Color enabledColor = game::Colors::Green;// LightGray;
		Color disabledColor = game::Colors::Red;// DarkGray; // 
		Color hoveredColor = game::Colors::White;  //maybe xor the colors
		Color pressedColor = game::Colors::Gray;
		Color outlineColor = game::Colors::Magenta;

		bool enabled;
		bool outlined;

		void Update()
		{
#if defined(_DEBUG)
			if (_pixelMode == nullptr)
			{
				enginePointer->geLogger->Error("Pixel Mode not set in ButtonUI named " + label + "\n");
				return;
			}
#endif
			game::Pointi mouse = _pixelMode->GetScaledMousePosition();
			_pressed = false;
			_hovered = false;
			uint32_t textScaled = textHeight * scale;
			uint32_t halfTextHeight = textScaled >> 1;
			uint32_t buttonRadius = halfTextHeight + 2;
			if (mouse.x < (int32_t)(position.x + length))
			{
				if (mouse.x > (int32_t)(position.x - buttonRadius))
				{
					if (mouse.y < (int32_t)(position.y + buttonRadius))
					{
						if (mouse.y > (int32_t)(position.y - buttonRadius))
						{
							_hovered = true;
							// Bug fix?
							// if WasButtonePressed(left)
							//  means was pressed on object,
							// pressedonobject = true
							if (enginePointer->geMouse.WasButtonReleased(geM_LEFT))
							{
								// if pressedonobject do
								enabled = !enabled;
								//software3D.SetState(GAME_SOFTWARE3D_TEXTURE, enabled);
								std::any t = enabled;
#if defined(_DEBUG)
								if (func == nullptr)
								{
									enginePointer->geLogger->Error("Callback function not set in ButtonUI named " + label + "\n");
									return;
								}
#endif
								func(label, t);
								// and pressedonobject = false
								// else ignore
							}
							if (enginePointer->geMouse.IsButtonHeld(geM_LEFT))
							{
								_pressed = true;
							}

						}

					}
				}
			}
		}

		void Draw()
		{
#if defined(_DEBUG)
			if (_pixelMode == nullptr)
			{
				enginePointer->geLogger->Error("Pixel Mode not set in ButtonUI named " + label + "\n");
				return;
			}
#endif
			uint32_t textScaled = textHeight * scale;
			uint32_t halfTextHeight = textScaled >> 1;
			uint32_t buttonRadius = halfTextHeight + 2;
			uint32_t centerPillX = position.x - buttonRadius + (length >> 1);

			uint32_t textPosX = centerPillX - ((uint32_t)label.length() * halfTextHeight);
			uint32_t textPosY = position.y - halfTextHeight + 1;

			if (outlined)
			{
				_pixelMode->HPillClip(position.x, position.y, length + 2, buttonRadius + 1, outlineColor);
			}
			Color color;

			if (enabled)
			{
				color = enabledColor;
			}
			else
			{
				color = disabledColor;
			}
			if (_hovered)
			{
				color = hoveredColor;
			}
			if (_pressed)
			{
				color = pressedColor;
			}
			_pixelMode->HPillClip(position.x, position.y, length, buttonRadius, color);
			_pixelMode->TextClip(label, textPosX, textPosY, textColor, scale);
		}
		PixelMode* _pixelMode;
		std::function<void(std::string, std::any)> func;
	private:
		bool _hovered;
		bool _pressed;
	};

	inline ButtonUI::ButtonUI()
	{
		length = 1;
		scale = 1;
		label = "Button";
		enabled = false;
		outlined = true;
		_pixelMode = nullptr;
		_hovered = false;
		_pressed = false;
		func = nullptr;
	}
}


#endif