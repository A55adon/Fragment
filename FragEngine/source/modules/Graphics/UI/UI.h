#pragma once

#include "core/config.h"
#include "core/DataTypes.h"
#include "modules/Graphics/UI/UIElement.h"
#include "modules/Graphics/Texture.h"
#include <array>
// UI API
/*
Derivative: UIElement (transform, style, (animationhandler), drag/resize) + debugdraw()->objectname, scale, pos, etc
Types:
- Base:
    - Rectangle(Rounded) ->()
    - Text -> (text, (font))
- Input:
    - Button ->(onClick)
    - Dropdown<Enum> ->(optionsEnum, toChangeEnumValue) + onValueChange()
    - Input field<datatype> ->(toChangeValue) + onValueChange()
    - Slider<datatype> -> (toChangeValue, min/max, step) + onValueChange()
- Statistics:
    - Histogram (line with past values)<datatype> -> (toShowVector)
    - Pie/Arc chart<datatype> -> (toShowData ? like vector or sum, colors for each data)

Factory:
    create[ObjectName](Args...,Transform,Style);

Transform:
    - position ->(%/px)
    - size ->(%/px)
Style:
    - primaryColor
    - secondaryColor
    - tertiaryColor

    - border bool
    - border width
    - border radius
    - border color

UI -> UIElement -> Base -> Input
                        -> Statistics
*/


class UI {
public:
	UI() = default;

	

	UIElement* createUIElement();

	void update(float mouseX, float mouseY, bool lmbDown);
private:
    std::vector<UIElement> _rootElements;

	// Interaction
	UIElement* _dragTarget = nullptr;
	UIElement* _resizeTarget = nullptr;
	int _resizeEdgeMask = 0;

	bool _lmbWasDown = false;
	float _lastMouseX = 0.0f;
	float _lastMouseY = 0.0f;
};

