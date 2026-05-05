#include "modules/Graphics/UI/UIBase/Rectangle.h"
#include "core/config.h"
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

void Rectangle::rebuild() {
    UIElement::rebuild();
    auto& tris = _mesh.getTriangles();
    tris.clear();

    const auto& t = getTransform();
    const auto& s = getStyle();

    const vec2<float> size = t.getSize();
    const float hx = size.x * 0.5f;
    const float hy = size.y * 0.5f;
    if (hx <= 0.0f || hy <= 0.0f) return;

    const float left = -hx;
    const float right = hx;
    const float bottom = -hy;
    const float top = hy;

    const Color primaryCol = s.getPrimaryColor();
    const Color borderCol = s.getBorderColor();
    const bool hasBorder = s.getBorder();
    const float borderWidth = std::max(0.0f, s.getBorderWidth());
    const float requestedRadius = std::max(0.0f, s.getBorderRadius());
    const float aspect = std::max(0.0001f, CFG_GET_WINDOW_ASPECT());

    auto addQuad = [&](float l, float b, float r, float tp, const Color& color) {
        if (r <= l || tp <= b) return;

        Vertex2D v0{ vec2<float>{l, b}, color };
        Vertex2D v1{ vec2<float>{r, b}, color };
        Vertex2D v2{ vec2<float>{r, tp}, color };
        Vertex2D v3{ vec2<float>{l, tp}, color };

        tris.push_back({ v0, v1, v2 });
        tris.push_back({ v0, v2, v3 });
    };

    auto addFilledRoundedRect = [&](float l, float b, float r, float tp, float rx, float ry, const Color& color) {
        if (r <= l || tp <= b) return;

        const float width = r - l;
        const float height = tp - b;
        rx = std::clamp(rx, 0.0f, width * 0.5f);
        ry = std::clamp(ry, 0.0f, height * 0.5f);

        if (rx <= 0.0f || ry <= 0.0f) {
            addQuad(l, b, r, tp, color);
            return;
        }

        addQuad(l + rx, b, r - rx, tp, color);
        addQuad(l, b + ry, l + rx, tp - ry, color);
        addQuad(r - rx, b + ry, r, tp - ry, color);

        constexpr int segments = 8;
        struct Corner { float cx, cy, startAngle; };
        const Corner corners[] = {
            { r - rx, tp - ry, 0.0f },
            { l + rx, tp - ry, M_PI * 0.5f },
            { l + rx, b + ry, M_PI },
            { r - rx, b + ry, M_PI * 1.5f }
        };

        for (const auto& corner : corners) {
            Vertex2D center{ vec2<float>{corner.cx, corner.cy}, color };
            float prevX = corner.cx + rx * std::cos(corner.startAngle);
            float prevY = corner.cy + ry * std::sin(corner.startAngle);

            for (int i = 1; i <= segments; ++i) {
                const float angle = corner.startAngle + (static_cast<float>(i) / segments) * M_PI * 0.5f;
                const float curX = corner.cx + rx * std::cos(angle);
                const float curY = corner.cy + ry * std::sin(angle);

                Vertex2D prev{ vec2<float>{prevX, prevY}, color };
                Vertex2D curr{ vec2<float>{curX, curY}, color };
                tris.push_back({ center, prev, curr });

                prevX = curX;
                prevY = curY;
            }
        }
    };

    const float radiusY = std::min(requestedRadius, std::min(hy, hx * aspect));
    const float radiusX = radiusY / aspect;

    if (!hasBorder || borderWidth <= 0.0f) {
        addFilledRoundedRect(left, bottom, right, top, radiusX, radiusY, primaryCol);
        return;
    }

    const float borderWidthX = std::min(borderWidth / aspect, hx);
    const float borderWidthY = std::min(borderWidth, hy);

    const float il = left + borderWidthX;
    const float ir = right - borderWidthX;
    const float ib = bottom + borderWidthY;
    const float it = top - borderWidthY;

    if (radiusY <= 0.0f || radiusX <= 0.0f) {
        addQuad(il, ib, ir, it, primaryCol);
        addQuad(left, bottom, right, ib, borderCol);
        addQuad(left, it, right, top, borderCol);
        addQuad(left, ib, il, it, borderCol);
        addQuad(ir, ib, right, it, borderCol);
        return;
    }

    addFilledRoundedRect(left, bottom, right, top, radiusX, radiusY, borderCol);

    float innerRadiusY = std::max(0.0f, radiusY - borderWidthY);
    if (ir > il && it > ib) {
        innerRadiusY = std::min(innerRadiusY, std::min((it - ib) * 0.5f, ((ir - il) * 0.5f) * aspect));
        const float innerRadiusX = innerRadiusY / aspect;
        addFilledRoundedRect(il, ib, ir, it, innerRadiusX, innerRadiusY, primaryCol);
    }
}

void Rectangle::onResize(float dx, float dy, int edgeMask) {
	UIElement::onResize(dx, dy, edgeMask);
}

void Rectangle::onDrag(float dx, float dy) {
	UIElement::onDrag(dx, dy);
}
