#pragma once

#include "modules/Graphics/UI/UIElement.h"

class Rectangle : public UIElement {
public:
	void rebuild() override;

	void onResize(float dx, float dy, int edgeMask) override;
	void onDrag(float dx, float dy) override;
};
