#pragma once

#include "modules/Graphics/UI/UIElement.h"

class Rectangle : public UIElement {
public:
	void rebuild();

	void onResize(float dx, float dy, int edgeMask);
	void onDrag(float dx, float dy);
};