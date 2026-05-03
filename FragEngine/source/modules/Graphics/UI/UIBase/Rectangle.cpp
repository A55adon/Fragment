#include "modules/Graphics/UI/UIBase/Rectangle.h"
#include "core/config.h"
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

    vec2<float> size = t.getSizeUS();
    size.x *= 2.0f;
    size.y *= 2.0f;

    float hx = size.x * 0.5f;
    float hy = size.y * 0.5f;

    float left = -hx;
    float right = hx;
    float top = hy;
    float bottom = -hy;

    Color primaryCol = s.getPrimaryColor();
    Color borderCol = s.getBorderColor();
    bool hasBorder = s.getBorder();
    float borderWidth = s.getBorderWidth();
    float borderRadius = s.getBorderRadius();

    float aspect = CFG_GET_WINDOW_ASPECT();   // width / height

    // Clamp border radius
    float maxRadiusY = hy;
    float maxRadiusX = hx;
    if (borderRadius > maxRadiusY) borderRadius = maxRadiusY;

    // Aspect‑correct border widths (pixel‑uniform)
    float borderWidthX = borderWidth / aspect;
    float borderWidthY = borderWidth;

    // Clamp border widths
    if (hasBorder) {
        if (borderWidthX > hx) borderWidthX = hx;
        if (borderWidthY > hy) borderWidthY = hy;
    }

    // Radii for outer corners (circular in screen pixels)
    float rx = borderRadius / aspect;
    float ry = borderRadius;
    if (rx > hx) rx = hx;
    if (ry > hy) ry = hy;

    if (borderRadius <= 0.0f) {
        if (hasBorder && borderWidth > 0.0f) {
            float il = left + borderWidthX;
            float ir = right - borderWidthX;
            float it = top - borderWidthY;
            float ib = bottom + borderWidthY;

            // Inner fill
            Vertex2D iv0{ vec2<float>{il, ib}, primaryCol };
            Vertex2D iv1{ vec2<float>{ir, ib}, primaryCol };
            Vertex2D iv2{ vec2<float>{ir, it}, primaryCol };
            Vertex2D iv3{ vec2<float>{il, it}, primaryCol };
            tris.push_back({ iv0, iv1, iv2 });
            tris.push_back({ iv0, iv2, iv3 });

            // Border quads
            // Bottom
            Vertex2D b_b0{ vec2<float>{left,  bottom}, borderCol };
            Vertex2D b_b1{ vec2<float>{right, bottom}, borderCol };
            Vertex2D b_b2{ vec2<float>{ir,    ib},     borderCol };
            Vertex2D b_b3{ vec2<float>{il,    ib},     borderCol };
            tris.push_back({ b_b0, b_b1, b_b2 });
            tris.push_back({ b_b0, b_b2, b_b3 });
            // Top
            Vertex2D b_t0{ vec2<float>{left, top}, borderCol };
            Vertex2D b_t1{ vec2<float>{il,   it},  borderCol };
            Vertex2D b_t2{ vec2<float>{ir,   it},  borderCol };
            Vertex2D b_t3{ vec2<float>{right,top}, borderCol };
            tris.push_back({ b_t0, b_t1, b_t2 });
            tris.push_back({ b_t0, b_t2, b_t3 });
            // Left
            Vertex2D b_l0{ vec2<float>{left,  bottom}, borderCol };
            Vertex2D b_l1{ vec2<float>{il,    ib},     borderCol };
            Vertex2D b_l2{ vec2<float>{il,    it},     borderCol };
            Vertex2D b_l3{ vec2<float>{left,  top},    borderCol };
            tris.push_back({ b_l0, b_l1, b_l2 });
            tris.push_back({ b_l0, b_l2, b_l3 });
            // Right
            Vertex2D b_r0{ vec2<float>{right, bottom}, borderCol };
            Vertex2D b_r1{ vec2<float>{right, top},    borderCol };
            Vertex2D b_r2{ vec2<float>{ir,    it},     borderCol };
            Vertex2D b_r3{ vec2<float>{ir,    ib},     borderCol };
            tris.push_back({ b_r0, b_r1, b_r2 });
            tris.push_back({ b_r0, b_r2, b_r3 });
        }
        else {
            Vertex2D v0{ vec2<float>{left,  bottom}, primaryCol };
            Vertex2D v1{ vec2<float>{right, bottom}, primaryCol };
            Vertex2D v2{ vec2<float>{right, top},    primaryCol };
            Vertex2D v3{ vec2<float>{left,  top},    primaryCol };
            tris.push_back({ v0, v1, v2 });
            tris.push_back({ v0, v2, v3 });
        }
        return;
    }

    const int segments = 8;

    // ---------- Filled rounded rectangle (no border) ----------
    if (!hasBorder || borderWidth <= 0.0f) {
        // Center
        Vertex2D c0{ vec2<float>{left + rx, bottom}, primaryCol };
        Vertex2D c1{ vec2<float>{right - rx, bottom}, primaryCol };
        Vertex2D c2{ vec2<float>{right - rx, top},    primaryCol };
        Vertex2D c3{ vec2<float>{left + rx, top},     primaryCol };
        tris.push_back({ c0, c1, c2 });
        tris.push_back({ c0, c2, c3 });

        // Left strip
        Vertex2D l0{ vec2<float>{left, bottom + ry}, primaryCol };
        Vertex2D l1{ vec2<float>{left + rx, bottom + ry}, primaryCol };
        Vertex2D l2{ vec2<float>{left + rx, top - ry}, primaryCol };
        Vertex2D l3{ vec2<float>{left, top - ry}, primaryCol };
        tris.push_back({ l0, l1, l2 });
        tris.push_back({ l0, l2, l3 });

        // Right strip
        Vertex2D r0{ vec2<float>{right - rx, bottom + ry}, primaryCol };
        Vertex2D r1{ vec2<float>{right, bottom + ry}, primaryCol };
        Vertex2D r2{ vec2<float>{right, top - ry}, primaryCol };
        Vertex2D r3{ vec2<float>{right - rx, top - ry}, primaryCol };
        tris.push_back({ r0, r1, r2 });
        tris.push_back({ r0, r2, r3 });

        // Corners
        struct Corner { float cx, cy, startAngle; };
        Corner corners[] = {
            { right - rx, top - ry, 0.0f },
            { left + rx, top - ry, M_PI * 0.5f },
            { left + rx, bottom + ry, M_PI },
            { right - rx, bottom + ry, M_PI * 1.5f }
        };
        for (const auto& cn : corners) {
            Vertex2D center{ vec2<float>{cn.cx, cn.cy}, primaryCol };
            float prevX = cn.cx + rx * cosf(cn.startAngle);
            float prevY = cn.cy + ry * sinf(cn.startAngle);
            for (int i = 1; i <= segments; ++i) {
                float angle = cn.startAngle + (static_cast<float>(i) / segments) * M_PI * 0.5f;
                float curX = cn.cx + rx * cosf(angle);
                float curY = cn.cy + ry * sinf(angle);
                Vertex2D prev{ vec2<float>{prevX, prevY}, primaryCol };
                Vertex2D curr{ vec2<float>{curX, curY}, primaryCol };
                tris.push_back({ center, prev, curr });
                prevX = curX;
                prevY = curY;
            }
        }
        return;
    }

    // ---------- Rounded rectangle WITH border ----------
    float innerR = borderRadius - borderWidth;   // vertical inner radius
    if (innerR < 0.0f) innerR = 0.0f;

    float innerRx = innerR / aspect;
    float innerRy = innerR;

    // Inner rectangle edges
    float il = left + borderWidthX;
    float ir = right - borderWidthX;
    float it = top - borderWidthY;
    float ib = bottom + borderWidthY;

    // Clamp inner radii
    float maxInnerRx = (ir - il) * 0.5f;
    float maxInnerRy = (it - ib) * 0.5f;
    if (innerRx > maxInnerRx) innerRx = maxInnerRx;
    if (innerRy > maxInnerRy) innerRy = maxInnerRy;

    // --- Inner filled rounded rectangle ---
    // Center
    Vertex2D ic0{ vec2<float>{il + innerRx, ib}, primaryCol };
    Vertex2D ic1{ vec2<float>{ir - innerRx, ib}, primaryCol };
    Vertex2D ic2{ vec2<float>{ir - innerRx, it}, primaryCol };
    Vertex2D ic3{ vec2<float>{il + innerRx, it}, primaryCol };
    tris.push_back({ ic0, ic1, ic2 });
    tris.push_back({ ic0, ic2, ic3 });

    // Left strip
    Vertex2D il0{ vec2<float>{il, ib + innerRy}, primaryCol };
    Vertex2D il1{ vec2<float>{il + innerRx, ib + innerRy}, primaryCol };
    Vertex2D il2{ vec2<float>{il + innerRx, it - innerRy}, primaryCol };
    Vertex2D il3{ vec2<float>{il, it - innerRy}, primaryCol };
    tris.push_back({ il0, il1, il2 });
    tris.push_back({ il0, il2, il3 });

    // Right strip
    Vertex2D ir0{ vec2<float>{ir - innerRx, ib + innerRy}, primaryCol };
    Vertex2D ir1{ vec2<float>{ir, ib + innerRy}, primaryCol };
    Vertex2D ir2{ vec2<float>{ir, it - innerRy}, primaryCol };
    Vertex2D ir3{ vec2<float>{ir - innerRx, it - innerRy}, primaryCol };
    tris.push_back({ ir0, ir1, ir2 });
    tris.push_back({ ir0, ir2, ir3 });

    // Inner corners
    struct Corner { float cx, cy, startAngle; };
    Corner innerCorners[] = {
        { ir - innerRx, it - innerRy, 0.0f },
        { il + innerRx, it - innerRy, M_PI * 0.5f },
        { il + innerRx, ib + innerRy, M_PI },
        { ir - innerRx, ib + innerRy, M_PI * 1.5f }
    };
    for (const auto& cn : innerCorners) {
        if (innerRx > 0.0f && innerRy > 0.0f) {
            Vertex2D center{ vec2<float>{cn.cx, cn.cy}, primaryCol };
            float prevX = cn.cx + innerRx * cosf(cn.startAngle);
            float prevY = cn.cy + innerRy * sinf(cn.startAngle);
            for (int i = 1; i <= segments; ++i) {
                float angle = cn.startAngle + (static_cast<float>(i) / segments) * M_PI * 0.5f;
                float curX = cn.cx + innerRx * cosf(angle);
                float curY = cn.cy + innerRy * sinf(angle);
                Vertex2D prev{ vec2<float>{prevX, prevY}, primaryCol };
                Vertex2D curr{ vec2<float>{curX, curY}, primaryCol };
                tris.push_back({ center, prev, curr });
                prevX = curX;
                prevY = curY;
            }
        }
    }

    // --- Outer border ---
    Corner outerCorners[] = {
        { right - rx, top - ry, 0.0f },
        { left + rx, top - ry, M_PI * 0.5f },
        { left + rx, bottom + ry, M_PI },
        { right - rx, bottom + ry, M_PI * 1.5f }
    };

    // Border corner arcs
    for (int c = 0; c < 4; ++c) {
        float ox = outerCorners[c].cx;
        float oy = outerCorners[c].cy;
        float ix = innerCorners[c].cx;
        float iy = innerCorners[c].cy;
        float startAng = outerCorners[c].startAngle;

        float prevOX = ox + rx * cosf(startAng);
        float prevOY = oy + ry * sinf(startAng);
        float prevIX = ix + innerRx * cosf(startAng);
        float prevIY = iy + innerRy * sinf(startAng);

        for (int i = 1; i <= segments; ++i) {
            float angle = startAng + (static_cast<float>(i) / segments) * M_PI * 0.5f;
            float curOX = ox + rx * cosf(angle);
            float curOY = oy + ry * sinf(angle);
            float curIX = ix + innerRx * cosf(angle);
            float curIY = iy + innerRy * sinf(angle);

            Vertex2D oPrev{ vec2<float>{prevOX, prevOY}, borderCol };
            Vertex2D iPrev{ vec2<float>{prevIX, prevIY}, borderCol };
            Vertex2D iCurr{ vec2<float>{curIX, curIY}, borderCol };
            Vertex2D oCurr{ vec2<float>{curOX, curOY}, borderCol };

            tris.push_back({ oPrev, iPrev, iCurr });
            tris.push_back({ oPrev, iCurr, oCurr });

            prevOX = curOX; prevOY = curOY;
            prevIX = curIX; prevIY = curIY;
        }
    }

    // Straight border sections
    // Top
    Vertex2D bt0{ vec2<float>{left + rx, top}, borderCol };
    Vertex2D bt1{ vec2<float>{left + rx, it},  borderCol };
    Vertex2D bt2{ vec2<float>{right - rx, it},  borderCol };
    Vertex2D bt3{ vec2<float>{right - rx, top}, borderCol };
    tris.push_back({ bt0, bt1, bt2 });
    tris.push_back({ bt0, bt2, bt3 });

    // Bottom
    Vertex2D bb0{ vec2<float>{left + rx, bottom}, borderCol };
    Vertex2D bb1{ vec2<float>{right - rx, ib},    borderCol };
    Vertex2D bb2{ vec2<float>{left + rx, ib},     borderCol };
    Vertex2D bb3{ vec2<float>{right - rx, bottom}, borderCol };
    tris.push_back({ bb0, bb1, bb2 });
    tris.push_back({ bb0, bb3, bb1 });

    // Left
    Vertex2D bl0{ vec2<float>{left, bottom + ry}, borderCol };
    Vertex2D bl1{ vec2<float>{il,   ib + innerRy}, borderCol };
    Vertex2D bl2{ vec2<float>{il,   it - innerRy}, borderCol };
    Vertex2D bl3{ vec2<float>{left, top - ry},    borderCol };
    tris.push_back({ bl0, bl1, bl2 });
    tris.push_back({ bl0, bl2, bl3 });

    // Right
    Vertex2D br0{ vec2<float>{right, bottom + ry}, borderCol };
    Vertex2D br1{ vec2<float>{right, top - ry},    borderCol };
    Vertex2D br2{ vec2<float>{ir,    it - innerRy}, borderCol };
    Vertex2D br3{ vec2<float>{ir,    ib + innerRy}, borderCol };
    tris.push_back({ br0, br1, br2 });
    tris.push_back({ br0, br2, br3 });
}

void Rectangle::onResize(float dx, float dy, int edgeMask) {
	UIElement::onResize(dx, dy, edgeMask);
}

void Rectangle::onDrag(float dx, float dy) {
	UIElement::onDrag(dx, dy);
}
