#include "modules/Graphics/UI/UI.h"
#include "modules/Graphics/UI/UIElement.h"

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

Text* UI::createText(const std::string& text, Transform transform, Style style, int fontSize)
{
	auto label = std::make_unique<Text>();
	label->setTransform(transform);
	label->setStyle(style);
	label->setFont(getDefaultFont(fontSize));
	label->setColor(style.getPrimaryColor());
	label->setText(text);

	Text* ptr = label.get();
	_rootElements.push_back(std::move(label));
	return ptr;
}

Button* UI::createButton(const std::string& label, std::function<void()> onClick, Transform transform, Style style, int fontSize)
{
	auto button = std::make_unique<Button>();
	button->setTransform(transform);
	button->setStyle(style);
	button->setFont(getDefaultFont(fontSize));
	button->setLabel(label);
	button->setOnClick(std::move(onClick));
	button->rebuild();

	Button* ptr = button.get();
	_rootElements.push_back(std::move(button));
	return ptr;
}

Histogram* UI::createHistogram(const std::vector<float>* values, Transform transform, Style style)
{
	auto histogram = std::make_unique<Histogram>();
	histogram->setTransform(transform);
	histogram->setStyle(style);
	histogram->bindValues(values);
	histogram->rebuild();

	Histogram* ptr = histogram.get();
	_rootElements.push_back(std::move(histogram));
	return ptr;
}

ArcChart* UI::createArcChart(const std::vector<float>* values, const std::vector<Color>& colors, Transform transform, Style style)
{
	auto chart = std::make_unique<ArcChart>();
	chart->setTransform(transform);
	chart->setStyle(style);
	chart->bindValues(values);
	chart->setSegmentColors(colors);
	chart->rebuild();

	ArcChart* ptr = chart.get();
	_rootElements.push_back(std::move(chart));
	return ptr;
}

void UI::update(float mouseX, float mouseY, bool lmbDown)
{
	for (auto& element : _rootElements) {
		element->update();
	}

	if (_debugDraw) debugDraw();
	else _debugOverlayElements.clear();

	if (!lmbDown) {
		if (_pressTarget) {
			if (_pressTarget->containsPx(mouseX, mouseY, _pressTarget->getParent() ? _pressTarget->getParent()->getAbsoluteWorldPosition() : vec2<float>{ 0.0f, 0.0f })) {
				_pressTarget->onClick();
			}
			_pressTarget->onPointerUp(mouseX, mouseY);
		}
		_dragTarget = nullptr;
		_resizeTarget = nullptr;
		_pressTarget = nullptr;
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
		_dragTarget = nullptr;
		_resizeTarget = nullptr;
		_resizeEdgeMask = 0;
		_pressTarget = nullptr;

		for (auto it = _rootElements.rbegin(); it != _rootElements.rend(); ++it) {
			UIElement* hit = (*it)->findTopElementAt(mouseX, mouseY, { 0.0f, 0.0f });
			if (hit) {
				_pressTarget = hit;
				if (_focusedElement && _focusedElement != hit) {
					_focusedElement->setFocused(false);
				}
				_focusedElement = hit->isFocusTarget() ? hit : nullptr;
				if (_focusedElement) _focusedElement->setFocused(true);
				hit->onPointerDown(mouseX, mouseY);
				break;
			}
		}

		if (!_pressTarget && _focusedElement) {
			_focusedElement->setFocused(false);
			_focusedElement = nullptr;
		}

		for (auto it = _rootElements.rbegin(); it != _rootElements.rend(); ++it) {
			int edgeMask = 0;
			UIElement* hit = (*it)->findInteractionTarget(mouseX, mouseY, { 0.0f, 0.0f }, edgeMask);

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

	if (_pressTarget) {
		_pressTarget->onPointerMove(mouseX, mouseY, lmbDown);
	}

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
	for (auto& el : _debugOverlayElements)
		els.push_back(el.get());
    return els;
}

void UI::debugDraw()
{
	_debugOverlayElements.clear();

	std::vector<std::pair<UIElement*, vec2<float>>> elements;
	for (auto& root : _rootElements) {
		root->collectElements(elements);
	}

	for (const auto& [element, parentPos] : elements) {
		if (element->getElementTypeName() == "Text") continue;

		auto overlay = std::make_unique<Text>();
		auto worldPos = element->getWorldPosition(parentPos);
		auto sizePx = element->getTransform().getSizePx();
		auto labelPos = worldPos;
		//labelPos.x -= element->getTransform().getSize().x / 2.f;
		labelPos.y += element->getTransform().getSize().y / 2.f + 0.015f;
		
		Transform tr;
		tr.setPosition(labelPos);
		tr.setSizePx({ std::max(10, sizePx.x), 28 });

		overlay->setTransform(tr);
		overlay->setStyle(element->getStyle());
		overlay->setFont(getDefaultFont(16));
		overlay->setColor(Color::Red);
		overlay->setText(element->getDebugLabel(parentPos));
		_debugOverlayElements.push_back(std::move(overlay));
	}
}

std::shared_ptr<UIFont> UI::getDefaultFont(int size)
{
	auto it = _fonts.find(size);
	if (it != _fonts.end()) return it->second;

	auto font = std::make_shared<UIFont>("res/Roboto-Black.ttf", size);
	_fonts[size] = font;
	return font;
}
