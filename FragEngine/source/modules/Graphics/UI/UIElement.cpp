#include "UIElement.h"

bool UIElement::containsPx(float x, float y) const {
	auto pos = _transform.getPositionPx();
	auto size = _transform.getSizePx();

	return x >= pos.x &&
		y >= pos.y &&
		x <= pos.x + size.x &&
		y <= pos.y + size.y;
}

int UIElement::getResizeEdgeMask(float x, float y, float threshold) const {
	if (!_resizable) return 0;

	auto pos = _transform.getPositionPx();
	auto size = _transform.getSizePx();

	int mask = 0;

	if (abs(x - pos.x) <= threshold) mask |= 1; // left
	if (abs(x - (pos.x + size.x)) <= threshold) mask |= 2; // right
	if (abs(y - pos.y) <= threshold) mask |= 4; // top
	if (abs(y - (pos.y + size.y)) <= threshold) mask |= 8; // bottom

	return mask;
}

UIElement* UIElement::findInteractionTarget(float x, float y, int& edgeMask) {
	for (auto it = _children.rbegin(); it != _children.rend(); ++it) {
		UIElement* child = (it)->findInteractionTarget(x, y, edgeMask);
		if (child) return child;
	}

	if (!containsPx(x, y)) return nullptr;

	edgeMask = getResizeEdgeMask(x, y);
	return this;
}

void UIElement::rebuild()
{
}

void UIElement::onResize(float dx, float dy, int edgeMask)
{
	if (_onResize) _onResize(dx, dy, edgeMask);

}

void UIElement::onDrag(float dx, float dy)
{
	if (_onDrag) _onDrag(dx, dy);

}
