#if !defined(GAMESIMPLEUI_H)
#define GAMESIMPLEUI_H

#include <any>
#include "GameColor.h"
#include "GameMath.h"
#include "GamePixelMode.h"

namespace game
{
	class ElementUI
	{
		friend class SimpleUI;
	public:
		ElementUI();
		Pointi position;
		std::string name;
		bool enabled;
		virtual void Update() { enginePointer->geLogger->Error("Must create Update() method."); }
		virtual void Draw() { enginePointer->geLogger->Error("Must create Draw() method."); }
	protected:
		std::function<void(std::string, std::any)> _uiCallback;
		PixelMode* _pixelMode;
		const uint32_t _textHeight = 8;
		bool _hovered;
		bool _pressed;
	private:
	};
	ElementUI::ElementUI()
	{
		name = "None";
		enabled = true;
		_hovered = false;
		_pressed = false;
		_uiCallback = nullptr;
		_pixelMode = nullptr;
	}


	class ButtonUI : public ElementUI
	{
	public:
		ButtonUI();
		// Length of rectangle region of pill shape
		uint32_t length;
		// Text scale (scales the vertical size of pillshape also)
		uint32_t scale;
		// Text of button
		std::string label;
		// Button on/off
		bool toggled;
		bool outlined;

		Color textColor = game::Colors::Black;
		Color toggledColor = game::Colors::LightGray;
		Color unToggledColor = game::Colors::DarkGray; // 
		Color hoveredColor = game::Colors::White;  //maybe xor the colors
		Color pressedColor = game::Colors::Gray;
		Color outlineColor = game::Colors::Magenta;

		void Update() override;
		void Draw() override;		
	private:
	};

	inline ButtonUI::ButtonUI()
	{
		length = 1;
		scale = 1;
		label = "Button";
		outlined = true;
		toggled = false;
	}

	void ButtonUI::Draw()
	{
#if defined(_DEBUG)
		if (_pixelMode == nullptr)
		{
			enginePointer->geLogger->Error("Pixel Mode not set in ButtonUI named " + label + "\n");
			return;
		}
#endif
		uint32_t textScaled = _textHeight * scale;
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

		if (toggled)
		{
			color = toggledColor;
		}
		else
		{
			color = unToggledColor;
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

	inline void ButtonUI::Update()
	{
#if defined(_DEBUG)
		if (_pixelMode == nullptr)
		{
			enginePointer->geLogger->Error("Pixel Mode not set in ButtonUI named " + label + "\n");
			return;
		}
#endif
		const game::Pointi mouse = _pixelMode->GetScaledMousePosition();
		_pressed = false;
		_hovered = false;
		const uint32_t textScaled = _textHeight * scale;
		const uint32_t halfTextHeight = textScaled >> 1;
		const uint32_t buttonRadius = halfTextHeight + 2;

		if (mouse.x < (int32_t)(position.x + length))
		{
			if (mouse.x > (int32_t)(position.x - buttonRadius))
			{
				if (mouse.y < (int32_t)(position.y + buttonRadius))
				{
					if (mouse.y > (int32_t)(position.y - buttonRadius))
					{
						_hovered = true;
						if (enginePointer->geMouse.WasButtonReleased(geM_LEFT))
						{
							// if pressedonobject do
							toggled = !toggled;
							std::any t = toggled;
#if defined(_DEBUG)
							if (_uiCallback == nullptr)
							{
								enginePointer->geLogger->Error("Callback function not set in ButtonUI named " + label + "\n");
								return;
							}
#endif
							_uiCallback(name, t);
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

 
	class SimpleUI
	{
	public:
		SimpleUI()
		{
			_pixelMode = nullptr;
			_uiCallback = nullptr;
		}
		SimpleUI(PixelMode &pixelMode, std::function<void(std::string, std::any)> callback)
		{
			_pixelMode = &pixelMode;
			_uiCallback = callback;
		}
		void Initialize(PixelMode& pixelMode, std::function<void(std::string, std::any)> callback)
		{
			_pixelMode = &pixelMode;
			_uiCallback = callback;
		}
		std::vector<ElementUI*> elements;
		void Add(ElementUI* newElement)
		{
			newElement->_pixelMode = _pixelMode;
			newElement->_uiCallback = _uiCallback;
			elements.emplace_back(newElement);
		}

		void Update()
		{
			const uint64_t size = elements.size();
			for (uint32_t ele = 0; ele < size; ++ele)
			{
				elements[ele]->Update();
			}
		}

		void Draw()
		{
			const uint64_t size = elements.size();
			for (uint32_t ele = 0; ele < size; ++ele)
			{
				elements[ele]->Draw();
			}
		}

	private:
		PixelMode* _pixelMode;
		std::function<void(std::string, std::any)> _uiCallback;
	};

}


#endif