#include "UIElement.h"
#include <cmath>

void UIElement::update()
{
	for (auto& child : _children) {
		child->update();
	}
}

void UIElement::draw(vec2<float> parentPosition, Shader* shader, GLuint vbo, GLuint vao)
{
	if (!_visible) return;
	vec2<float> worldPos = parentPosition + _transform.getPosition();

	//Draw
	for (auto& t : _mesh.getTriangles())
	{
		shader->use();

		if (_mesh.getTexture())
		{
			_mesh.getTexture()->bind(0);

			shader->setInt("uTexture", 0);
			shader->setInt("useTexture", 1);
		}
		else
		{
			shader->setInt("useTexture", 0);
		}

		Vertex2D verts[3];
		for (int i = 0; i < 3; ++i)
		{
			verts[i] = t.vertices[i];
			verts[i].position.x += worldPos.x;
			verts[i].position.y += worldPos.y;
		}

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);
	}

	for (auto& child : _children) {
		child->draw(worldPos, shader, vbo, vao);
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	_dirtyDraw = false;
}

vec2<float> UIElement::getWorldPosition(vec2<float> parentPos) const
{
	return { parentPos.x + _transform.getPosition().x, parentPos.y + _transform.getPosition().y };
}

vec2<float> UIElement::getAbsoluteWorldPosition() const
{
	if (!_parent) return _transform.getPosition();
	return _parent->getAbsoluteWorldPosition() + _transform.getPosition();
}

bool UIElement::containsPx(float x, float y, vec2<float> parentPos) const {
	auto worldPos = getWorldPosition(parentPos);
	auto size = _transform.getSizePx();
	auto pos = vec2<int>{
		static_cast<int>((worldPos.x + 1.0f) * 0.5f * CFG_WINDOW_WIDTH),
		static_cast<int>((worldPos.y + 1.0f) * 0.5f * CFG_WINDOW_HEIGHT)
	};
	int left = pos.x - size.x / 2;
	int bottom = pos.y - size.y / 2;
	int right = left + size.x;
	int top = bottom + size.y;

	return x >= left &&
		y >= bottom &&
		x <= right &&
		y <= top;
}

int UIElement::getResizeEdgeMask(float x, float y, vec2<float> parentPos, float threshold) const {
	if (!_resizable) return 0;

	auto worldPos = getWorldPosition(parentPos);
	auto size = _transform.getSizePx();
	auto pos = vec2<int>{
		static_cast<int>((worldPos.x + 1.0f) * 0.5f * CFG_WINDOW_WIDTH),
		static_cast<int>((worldPos.y + 1.0f) * 0.5f * CFG_WINDOW_HEIGHT)
	};
	float left = static_cast<float>(pos.x - size.x / 2);
	float right = left + static_cast<float>(size.x);
	float bottom = static_cast<float>(pos.y - size.y / 2);
	float top = bottom + static_cast<float>(size.y);

	int mask = 0;

	if (std::abs(x - left) <= threshold) mask |= 1; // left
	if (std::abs(x - right) <= threshold) mask |= 2; // right
	if (std::abs(y - top) <= threshold) mask |= 4; // top
	if (std::abs(y - bottom) <= threshold) mask |= 8; // bottom

	return mask;
}

UIElement* UIElement::findInteractionTarget(float x, float y, vec2<float> parentPos, int& edgeMask) {
	if (!_visible) return nullptr;

	vec2<float> worldPos = getWorldPosition(parentPos);

	for (auto it = _children.rbegin(); it != _children.rend(); ++it) {
		UIElement* child = (*it)->findInteractionTarget(x, y, worldPos, edgeMask);
		if (child) return child;
	}

	if (!_hitTestVisible) return nullptr;
	if (!containsPx(x, y, parentPos)) return nullptr;

	edgeMask = getResizeEdgeMask(x, y, parentPos);
	return this;
}

UIElement* UIElement::findTopElementAt(float x, float y, vec2<float> parentPos)
{
	if (!_visible) return nullptr;

	vec2<float> worldPos = getWorldPosition(parentPos);
	for (auto it = _children.rbegin(); it != _children.rend(); ++it) {
		UIElement* child = (*it)->findTopElementAt(x, y, worldPos);
		if (child) return child;
	}

	if (!_hitTestVisible) return nullptr;
	return containsPx(x, y, parentPos) ? this : nullptr;
}

void UIElement::rebuild()
{
	markDirty();
}

void UIElement::markDirty()
{
	_dirtyDraw = true;
	if (_parent) {
		_parent->markDirty();
	}
}
void UIElement::onResize(float dx, float dy, int edgeMask)
{
	auto& t = _transform;

	auto pos = t.getPositionUS();
	auto size = t.getSizeUS();

	// left
	if (edgeMask & 1) {
		pos.x += dx * 0.5f;
		size.x -= dx;
	}

	// right 
	if (edgeMask & 2) {
		pos.x += dx * 0.5f;
		size.x += dx;
	}

	// top
	if (edgeMask & 4) {
		pos.y += dy * 0.5f;
		size.y += dy;
	}

	// bottom
	if (edgeMask & 8) {
		pos.y += dy * 0.5f;
		size.y -= dy;
	}

	// clamp minimum size
	const float minSize = 1.0f;
	size.x = std::max(size.x, minSize);
	size.y = std::max(size.y, minSize);

	t.setPositionUS(pos);
	t.setSizeUS(size);

	rebuild();
}

void UIElement::onDrag(float dx, float dy)
{
	auto& t = _transform;

	auto pos = t.getPositionUS();
	pos.x += dx;
	pos.y += dy;

	t.setPositionUS(pos);

	rebuild();
}

std::string UIElement::getDebugLabel(vec2<float> parentPos) const
{
	auto worldPos = getWorldPosition(parentPos);
	auto posPx = vec2<int>{
		static_cast<int>((worldPos.x + 1.0f) * 0.5f * CFG_WINDOW_WIDTH),
		static_cast<int>((worldPos.y + 1.0f) * 0.5f * CFG_WINDOW_HEIGHT)
	};
	auto sizePx = _transform.getSizePx();

	return std::format(
		"[{}x{}] ({}, {})",
		sizePx.x,
		sizePx.y,
		posPx.x,
		posPx.y
	);
}

void UIElement::collectElements(std::vector<std::pair<UIElement*, vec2<float>>>& out, vec2<float> parentPos)
{
	if (!_visible) return;

	vec2<float> worldPos = getWorldPosition(parentPos);
	out.push_back({ this, parentPos });

	for (auto& child : _children) {
		child->collectElements(out, worldPos);
	}
}
