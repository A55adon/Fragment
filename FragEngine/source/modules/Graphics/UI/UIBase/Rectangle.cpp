#include "modules/Graphics/UI/UIBase/Rectangle.h"

void Rectangle::rebuild() {
    auto& tris = _mesh.getTriangles();
    tris.clear();

    const auto& t = getTransform();
    const auto& s = getStyle();

    vec2<float> pos = t.getPosition();
    vec2<float> size = t.getSizeUS();
    size.x *= 2.0f;
    size.y *= 2.0f;

    float hx = size.x * 0.5f;
    float hy = size.y * 0.5f;

    float left = pos.x - hx;
    float right = pos.x + hx;
    float top = pos.y + hy;
    float bottom = pos.y - hy;

    Color col = s.getPrimaryColor();

    Vertex2D v0{ {left,  bottom}, col };
    Vertex2D v1{ {right, bottom}, col };
    Vertex2D v2{ {right, top},    col };
    Vertex2D v3{ {left,  top},    col };

    tris.push_back({ v0, v1, v2 });
    tris.push_back({ v0, v2, v3 });
}


void Rectangle::onResize(float dx, float dy, int edgeMask) {
	UIElement::onResize(dx, dy, edgeMask);
}

void Rectangle::onDrag(float dx, float dy) {
	UIElement::onDrag(dx, dy);
}
