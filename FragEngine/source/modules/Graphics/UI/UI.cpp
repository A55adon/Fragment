#include "modules/Graphics/UI/UI.h"
#include "modules/Graphics/UI/UIElement.h"
#include "modules/IO/UserInput/Input.h"

UIElement* UI::createUIElement(Transform transform, Style style)
{
	std::unique_ptr<UIElement> el = std::make_unique<UIElement>();
	el->setTransform(transform);
	el->setStyle(style);	
	_rootElements.push_back(std::move(el));
	return _rootElements.back().get();
}

Rectangle* UI::createRectangle(Transform transform, Style style)
{
	auto rect = std::make_unique<Rectangle>();

	rect->setTransform(transform);
	rect->setStyle(style);
	rect->rebuild();

	Rectangle* ptr = rect.get();
	_rootElements.push_back(std::move(rect));

	return ptr;
}

void UI::update(float mouseX, float mouseY, bool lmbDown)
{
	if(_debugDraw) debugDraw();

	if (!lmbDown) {
		_dragTarget = nullptr;
		_resizeTarget = nullptr;
		_resizeEdgeMask = 0;
		_lmbWasDown = false;
		_lastMouseX = mouseX;
		_lastMouseY = mouseY;
		return;
	}

	if (!_lmbWasDown) {
		_lmbWasDown = true;
		_lastMouseX = mouseX;
		_lastMouseY = mouseY;

		for (auto it = _rootElements.rbegin(); it != _rootElements.rend(); ++it) {
			int edgeMask = 0;
			UIElement* hit = (*it)->findInteractionTarget(mouseX, mouseY, edgeMask);

			if (hit) {
				if (edgeMask != 0 && hit->isResizable()) {
					_resizeTarget = hit;
					_resizeEdgeMask = edgeMask;
				}
				else if (hit->isDraggable()) {
					_dragTarget = hit;
				}
				break;
			}
		}
		return;
	}

	float dx = mouseX - _lastMouseX;
	float dy = mouseY - _lastMouseY;

	_lastMouseX = mouseX;
	_lastMouseY = mouseY;

	if (dx == 0 && dy == 0) return;

	// normalize to UI space (0–1)
	float dxUI = dx / CFG_WINDOW_WIDTH;
	float dyUI = dy / CFG_WINDOW_HEIGHT;

	if (_resizeTarget) {
		_resizeTarget->onResize(dxUI, dyUI, _resizeEdgeMask);
	}
	else if (_dragTarget) {
		_dragTarget->onDrag(dxUI, dyUI);
	}
}

std::vector<UIElement*> UI::getRootElements() {
    std::vector<UIElement*> els;
    for (auto& el : _rootElements)
        els.push_back(el.get());
    return els;
}