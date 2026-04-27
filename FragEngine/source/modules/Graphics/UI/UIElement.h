#pragma once

//#include "modules/Graphics/UI/UI.h"
#include "core/Mesh2D.h"
#include "core/DataTypes.h"
#include <functional>

class UIElement {
public:
	UIElement() = default;

	void setTransform(const Transform& t) { _transform = t; }
	Transform& getTransform() { return _transform; }
	const Transform& getTransform() const { return _transform; }

	void setStyle(const Style& s) { _style = s; }
	Style& getStyle() { return _style; }
	const Style& getStyle() const { return _style; }

	// Children
	void addChild(UIElement child) {
		_children.push_back(child);
	}

	UIElement* getChild(size_t i) {
		return &_children[i];
	}

	const UIElement* getChild(size_t i) const {
		return &_children[i];
	}

	size_t getChildCount() const {
		return _children.size();
	}

	std::vector<UIElement>& getChildren() {
		return _children;
	}

	const std::vector<UIElement>& getChildren() const {
		return _children;
	}

	void setDraggable(bool v) { _draggable = v; }
	bool isDraggable() const { return _draggable; }

	void setOnDrag(const std::function<void(float, float)>& fn) {
		_onDrag = fn;
	}

	const std::function<void(float, float)>& getOnDrag() const {
		return _onDrag;
	}

	void setResizable(bool v) { _resizable = v; }
	bool isResizable() const { return _resizable; }

	void setOnResize(const std::function<void(float, float, int)>& fn) {
		_onResize = fn;
	}

	const std::function<void(float, float, int)>& getOnResize() const {
		return _onResize;
	}

	void setMesh2D(Mesh2D mesh) {
		_mesh = mesh;
	}

	const Mesh2D& getMesh2D() const {
		return _mesh;
	}

	bool containsPx(float x, float y) const {
		auto pos = _transform.getPositionPx();
		auto size = _transform.getSizePx();

		return x >= pos.x &&
			y >= pos.y &&
			x <= pos.x + size.x &&
			y <= pos.y + size.y;
	}

	int getResizeEdgeMask(float x, float y, float threshold = 5.0f) const {
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

	UIElement* findInteractionTarget(float x, float y, int& edgeMask) {
		for (auto it = _children.rbegin(); it != _children.rend(); ++it) {
			UIElement* child = (it)->findInteractionTarget(x, y, edgeMask);
			if (child) return child;
		}

		if (!containsPx(x, y)) return nullptr;

		edgeMask = getResizeEdgeMask(x, y);
		return this;
	}

private:
	Mesh2D _mesh;
	Transform _transform;
	Style _style;

	std::vector<UIElement> _children;

	bool _draggable = false;
	std::function<void(float, float)> _onDrag;

	bool _resizable = false;
	std::function<void(float, float, int)> _onResize;
};