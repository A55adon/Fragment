#pragma once

//#include "modules/Graphics/UI/UI.h"
#include "core/Mesh2D.h"
#include "core/DataTypes.h"
#include <functional>

class UIElement {
public:
	UIElement() = default;
	virtual ~UIElement() = default;

	void setTransform(const Transform& t) { _transform = t; }
	Transform& getTransform() { return _transform; }
	const Transform& getTransform() const { return _transform; }

	void setStyle(const Style& s) { _style = s; }
	Style& getStyle() { return _style; }
	const Style& getStyle() const { return _style; }

	// Children
	UIElement* addChild(std::unique_ptr<UIElement> child) {
		child->_parent = this;
		_children.push_back(std::move(child));
		return _children.back().get();
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

	void setVisible(bool v) { _visible = v; }
	bool isVisible() const { return _visible; }

	void setHitTestVisible(bool v) { _hitTestVisible = v; }
	bool isHitTestVisible() const { return _hitTestVisible; }

	void setFocusTarget(bool v) { _focusTarget = v; }
	bool isFocusTarget() const { return _focusTarget; }

	void setFocused(bool v) { _focused = v; }
	bool isFocused() const { return _focused; }

	UIElement* getParent() const { return _parent; }

	virtual std::string getElementTypeName() const { return "UIElement"; }

	virtual void update();

	bool containsPx(float x, float y, vec2<float> parentPos = { 0.0f, 0.0f }) const;

	int getResizeEdgeMask(float x, float y, vec2<float> parentPos = { 0.0f, 0.0f }, float threshold = 5.0f) const;

	UIElement* findInteractionTarget(float x, float y, vec2<float> parentPos, int& edgeMask);
	UIElement* findTopElementAt(float x, float y, vec2<float> parentPos);

	vec2<float> getWorldPosition(vec2<float> parentPos = { 0.0f, 0.0f }) const;
	vec2<float> getAbsoluteWorldPosition() const;

	virtual void rebuild();

	virtual void onResize(float dx, float dy, int edgeMask);
	virtual void onDrag(float dx, float dy);
	virtual void onPointerDown(float mouseX, float mouseY) {}
	virtual void onPointerMove(float mouseX, float mouseY, bool lmbDown) {}
	virtual void onPointerUp(float mouseX, float mouseY) {}
	virtual void onClick() {}

	std::string getDebugLabel(vec2<float> parentPos = { 0.0f, 0.0f }) const;
	void collectElements(std::vector<std::pair<UIElement*, vec2<float>>>& out, vec2<float> parentPos = { 0.0f, 0.0f });

protected:
	Mesh2D _mesh;
	Transform _transform;
	Style _style;

	std::vector<std::unique_ptr<UIElement>> _children;
private:
	bool _draggable = false;
	bool _resizable = false;
	bool _visible = true;
	bool _hitTestVisible = true;
	bool _focusTarget = false;
	bool _focused = false;
	UIElement* _parent = nullptr;
};
