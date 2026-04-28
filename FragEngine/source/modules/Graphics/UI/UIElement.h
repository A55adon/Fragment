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

	bool containsPx(float x, float y) const;

	int getResizeEdgeMask(float x, float y, float threshold = 5.0f) const;

	UIElement* findInteractionTarget(float x, float y, int& edgeMask);

	virtual void rebuild();

	virtual void onResize(float dx, float dy, int edgeMask);
	virtual void onDrag(float dx, float dy);

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