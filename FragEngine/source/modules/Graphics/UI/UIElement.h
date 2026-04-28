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
	void addChild(std::unique_ptr<UIElement> child) {
		_children.push_back(std::move(child));
	}

	UIElement* getChild(size_t i) {
		return _children[i].get();
	}

	size_t getChildCount() const {
		return _children.size();
	}

	std::vector<UIElement*> getChildren() {
		std::vector<UIElement*> els;
		for (auto& el : _children) 
			els.push_back(el.get());
		return els;
	}

	void setDraggable(bool v) { _draggable = v; }
	bool isDraggable() const { return _draggable; }

	void setResizable(bool v) { _resizable = v; }
	bool isResizable() const { return _resizable; }

	void setMesh2D(Mesh2D mesh) {
		_mesh = mesh;
	}

	Mesh2D& getMesh2D() {
		return _mesh;
	}

	bool containsPx(float x, float y) const;

	int getResizeEdgeMask(float x, float y, float threshold = 5.0f) const;

	UIElement* findInteractionTarget(float x, float y, int& edgeMask);

	void rebuild();

	virtual void onResize(float dx, float dy, int edgeMask);
	virtual void onDrag(float dx, float dy);

protected:
	Mesh2D _mesh;
	Transform _transform;
	Style _style;

	std::vector<std::unique_ptr<UIElement>> _children;
private:
	bool _draggable = false;
	bool _resizable = false;
};