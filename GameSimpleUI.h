#if !defined(GAMESIMPLEUI_H)
#define GAMESIMPLEUI_H

#include <any>
#include "GameColor.h"
#include "GameMath.h"
#include "GamePixelMode.h"

namespace game
{
	inline bool CallBackValueCheckUI(const std::string& type, const std::any& value)
	{
		return (value.type().name() == type);
	}

#define UI_VALUE_CHECK(nameOf, typeOf, valueOf) if(!game::CallBackValueCheckUI(typeOf,valueOf)) \
	{ \
		std::cout << nameOf << " sent an INVALID VALUE: " << valueOf.type().name() << "\n"; \
		return; \
	}\

	// UI element base class, all UI needs to derive from this
	class ElementUI
	{
		friend class SimpleUI;
	public:
		ElementUI();
		Pointi position;
		std::string name;
		bool enabled;
		virtual bool Update() { enginePointer->geLogger->Error("Must create Update() method."); return false; }
		virtual void Draw() { enginePointer->geLogger->Error("Must create Draw() method."); }
	protected:
		std::function<void(const std::string&, std::any&)> _uiCallback;
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

	// Can be toggled on or off.  Passes a bool to callback.
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

		Color labelColor = game::Colors::Black;
		Color toggledColor = game::Colors::LightGray;
		Color unToggledColor = game::Colors::DarkGray; // 
		Color hoveredColor = game::Colors::White;  //maybe xor the colors
		Color pressedColor = game::Colors::Gray;
		Color outlineColor = game::Colors::Magenta;

		bool Update() override;
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

	inline void ButtonUI::Draw()
	{
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
		_pixelMode->TextClip(label, textPosX, textPosY, labelColor, scale);
	}

	inline bool ButtonUI::Update()
	{
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
							toggled = !toggled;
							std::any t = toggled;
							_uiCallback(name, t);
						}

						if (enginePointer->geMouse.IsButtonHeld(geM_LEFT))
						{
							_pressed = true;
						}
						return true;
					}

				}
			}
		}
		return false;
	}

	// Can be checked or not checked.  Passes a bool to callback
	class CheckBoxUI : public ElementUI
	{
	public:
		CheckBoxUI();
		// Text scale (scales the vertical size of checkbox)
		uint32_t scale;
		// Text of button
		std::string label;
		bool checked;
		bool outlined;

		Color labelColor = game::Colors::Black;
		Color outlineColor = game::Colors::Magenta;
		Color boxColor = game::Colors::Yellow;
		Color xColor = game::Colors::DarkRed;

		bool Update() override;
		void Draw() override;
	private:
	};

	inline CheckBoxUI::CheckBoxUI()
	{
		scale = 1;
		label = "CheckBox";
		outlined = true;
		checked = false;
	}

	inline void CheckBoxUI::Draw()
	{
		const int32_t textScaled = scale * _textHeight;

		// Outline
		_pixelMode->RectClip({ position.x - 1, position.y - 1, position.x + (textScaled + 2) + 1, position.y + (textScaled + 2) + 1 }, outlineColor);
		// Box
		_pixelMode->RectFilledClip({ position.x,position.y,position.x + (textScaled + 2),position.y + (textScaled + 2) }, boxColor);
		if (checked)
		{
			// Checkmark "X"
			_pixelMode->TextClip("x", (position.x)+1, (position.y + 1), xColor, scale);
		}
		// Label
		_pixelMode->TextClip(label, position.x + (textScaled * 2), position.y + 2, labelColor, scale);
	}

	inline bool CheckBoxUI::Update()
	{
		const game::Pointi mouse = _pixelMode->GetScaledMousePosition();
		_pressed = false;
		_hovered = false;
		const int32_t textScaled = _textHeight * scale;

		if (mouse.x < (position.x + (textScaled + 2) + 1))
		{
			if (mouse.x > (position.x))
			{
				if (mouse.y < (position.y + (textScaled + 2) + 1))
				{
					if (mouse.y > (position.y))
					{
						_hovered = true;
						if (enginePointer->geMouse.WasButtonReleased(geM_LEFT))
						{
							checked = !checked;
							std::any t = checked;
							_uiCallback(name, t);
						}

						if (enginePointer->geMouse.IsButtonHeld(geM_LEFT))
						{
							_pressed = true;
						}
						return true;
					}

				}
			}
		}
		return false;
	}
 
	// Can be checked or not checked and only one in a group can be checked.  Passes a bool to callback
	class RadialUI : public ElementUI
	{
	public:
		RadialUI();
		// Text scale (scales the vertical size of radial)
		uint32_t scale;
		// Text of radial
		std::string label;
		bool checked;
		bool outlined;
		int32_t group;

		Color labelColor = game::Colors::Black;
		Color outlineColor = game::Colors::Magenta;
		Color boxColor = game::Colors::Yellow;
		Color xColor = game::Colors::DarkRed;

		bool Update() override;
		void Draw() override;
	private:
	};

	inline RadialUI::RadialUI()
	{
		scale = 1;
		label = "Radial";
		outlined = true;
		checked = false;
		group = -1;
	}

	inline void RadialUI::Draw()
	{
		const int32_t textScaled = scale * _textHeight;
		const int32_t scaledRadius = 5 * scale;

		// Outline
		_pixelMode->CircleClip(position.x + scaledRadius, position.y + scaledRadius, scaledRadius+1, game::Colors::Magenta);

		// Circle
		_pixelMode->CircleFilledClip(position.x + scaledRadius, position.y + scaledRadius, scaledRadius, game::Colors::White);

		// Dot
		if (checked)
		{
			_pixelMode->CircleFilledClip(position.x + scaledRadius, position.y + scaledRadius, scaledRadius - (2 * scale), game::Colors::Black);
		}

		// Label
		_pixelMode->TextClip(label, position.x + (textScaled) * 2, position.y + (scaledRadius - (scaledRadius - (1+scale))), game::Colors::White, scale);
	
	}

	inline bool RadialUI::Update()
	{
		const game::Pointi mouse = _pixelMode->GetScaledMousePosition();
		_pressed = false;
		_hovered = false;
		const int32_t scaledDiameter = 5 * scale * 2;// _textHeight* scale;
		//_pixelMode->RectClip({ position.x,position.y,(position.x + scaledDiameter), (position.y + scaledDiameter) }, game::Colors::Yellow);
		if (mouse.x < (position.x + (scaledDiameter)))
		{
			if (mouse.x >= (position.x))
			{
				if (mouse.y <= (position.y + (scaledDiameter)))
				{
					if (mouse.y >= (position.y))
					{
						_hovered = true;
						if (enginePointer->geMouse.WasButtonReleased(geM_LEFT))
						{
							if (!checked)
							{
								checked = !checked;
								std::any t = checked;
								_uiCallback(name, t);
							}
						}

						if (enginePointer->geMouse.IsButtonHeld(geM_LEFT))
						{
							_pressed = true;
						}
						return true;
					}

				}
			}
		}
		return false;
	}

	class SliderUI : public ElementUI
	{
	public:
		SliderUI();
		uint32_t scale;
		// Text of slider
		std::string label;
		bool outlined;

		float_t minValue;
		float_t maxValue;
		float_t value;
		uint32_t length;

		Color labelColor = game::Colors::Black;
		Color outlineColor = game::Colors::Magenta;
		Color boxColor = game::Colors::Yellow;
		Color xColor = game::Colors::DarkRed;

		bool Update() override;
		void Draw() override;
	private:
		float_t _valuePercentOfBar;
	};

	inline SliderUI::SliderUI()
	{
		scale = 1;
		label = "Slider";
		length = 0;
		outlined = true;
		minValue = 0;
		maxValue = 0;
		value = 0;
		_valuePercentOfBar = 0;
	}

	inline void SliderUI::Draw()
	{
		const int32_t textScaled = scale * _textHeight;
		const int32_t scaledRadius = 5 * scale;

		// h bar
		//_pixelMode->RectFilledClip({ position.x, position.y + 8 + 4, position.x + (int32_t)length, position.y + 8 + 9 }, boxColor);
		_pixelMode->HLineClip(position.x, position.x + (int32_t)length, position.y + 12 + (9 - 4) + 1, boxColor);
		_pixelMode->HLineClip(position.x, position.x + (int32_t)length, position.y + 12 + (9 - 4) + 2, boxColor);

		// Value bar
		float vPos = length * _valuePercentOfBar;
		if (_hovered)
		_pixelMode->RectFilledClip({ position.x + (int32_t)(vPos), position.y + 12 + 4 - 4, position.x + (int32_t)(vPos) + 3, position.y + 10 + 9 + 4}, game::Colors::Magenta);
		else
			_pixelMode->RectFilledClip({ position.x + (int32_t)(vPos), position.y + 12 + 4 - 4, position.x + (int32_t)(vPos)+3, position.y + 10 + 9 + 4 }, game::Colors::DarkRed);

		// Label
		_pixelMode->TextClip(label, position.x + (textScaled) * 2, position.y + (scaledRadius - (scaledRadius - (1 + scale))), game::Colors::White, scale);
		// the following could be optimized
		_pixelMode->TextClip(std::to_string(value).substr(0, std::to_string(value).find(".") + 3), position.x - (uint32_t)(std::to_string(value).substr(0, std::to_string(value).find(".") + 3).length() + 1) * 8, position.y + 12 + 4, game::Colors::White);
	}

	inline bool SliderUI::Update()
	{
		const game::Pointi mouse = _pixelMode->GetScaledMousePosition();
		_pressed = false;
		_hovered = false;


		_valuePercentOfBar = (value + abs(minValue)) / (float)(abs(minValue) + abs(maxValue));

		if (mouse.x <= (position.x + (int32_t)(length)))
		{
			if (mouse.x >= (position.x))
			{
				if (mouse.y <= (position.y + 12 + 9 + 4))
				{
					if (mouse.y >= (position.y + 12 + 4 - 4))
					{
						_hovered = true;

						if (enginePointer->geMouse.IsButtonHeld(geM_LEFT))
						{
							_pressed = true;
							
							_valuePercentOfBar = (mouse.x - position.x) / (float)(length);
							float_t barScale = (_valuePercentOfBar) * (float)(maxValue - minValue) + minValue;
							
							value = barScale;
							
							value = max(minValue, value);
							value = min(maxValue, value);

							std::any t = value;
							_uiCallback(name, t);
						}
						return true;

					}

				}
			}
		}
		return false;

	}

	class SimpleUI
	{
	public:
		SimpleUI()
		{
			_pixelMode = nullptr;
			_uiCallback = nullptr;
			inputWasUsed = false;
		}
		bool inputWasUsed;
		SimpleUI(PixelMode &pixelMode, std::function<void(const std::string&, std::any&)> callback)
		{
			_pixelMode = &pixelMode;
			_uiCallback = callback;
			inputWasUsed = false;
		}
		void Initialize(PixelMode& pixelMode, std::function<void(const std::string&, std::any&)> callback)
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
#if defined(_DEBUG)
			if (_uiCallback == nullptr)
			{
				enginePointer->geLogger->Error("Callback function not set in SimpleUI.\n");
				return;
			}
#endif
			inputWasUsed = false;
			const uint64_t size = elements.size();
			bool ret = false;
			for (uint32_t ele = 0; ele < size; ++ele)
			{
				ret = elements[ele]->Update();
				if (ret) inputWasUsed = true;
			}
		}

		void Draw()
		{
#if defined(_DEBUG)
			if (_pixelMode == nullptr)
			{
				enginePointer->geLogger->Error("Pixel Mode not set in SimpleUI.\n");
				return;
		}
#endif
			const uint64_t size = elements.size();
			for (uint32_t ele = 0; ele < size; ++ele)
			{
				elements[ele]->Draw();
			}
		}

	private:
		PixelMode* _pixelMode;
		std::function<void(const std::string&, std::any&)> _uiCallback;

	};

}


#endif