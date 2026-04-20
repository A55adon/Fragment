#pragma once

#include "modules/Graphics/UI/UIObject.h"

class Rect : public UIObject {
public:
    using UIObject::UIObject;

    void create() {
        create(_transform, _style);
    }

    void onResize(float dx, float dy, int edgeMask) override {
        UIObject::onResize(dx, dy, edgeMask);
        create();
    }

    void onDrag(float dx, float dy) override {
        UIObject::onDrag(dx, dy);
        create();
    }

    void create(const UITransform& transform, const UIStyle& style) {
        _transform = transform;
        _style = style;
        _mesh.triangles.clear();

        float aspectFix = 1.0f;
        if (_parentUI && _parentUI->_window) {
            float w = (float)_parentUI->_window->getWidth();
            float h = (float)_parentUI->_window->getHeight();
            if (w > 0.f) {
                aspectFix = h / w;
            }
        }

        float rw = transform.size.x / 100.f;
        float rh = transform.size.y / 100.f;

        float bw = style.borderWidth / 100.f;
        float r = style.cornerRoundness / 100.f;

        r = std::min(r, std::min(rw * 0.5f, rh * 0.5f));

        int segs = std::max(2, style.cornerSegments);

        auto buildRoundedRect = [&](float halfW, float halfH, float radius, Color color) {
            std::vector<vec2<float>> outline;

            float cx = (halfW - radius) * aspectFix;
            float cy = (halfH - radius);

            auto addCorner = [&](float ox, float oy, float startAngle) {
                for (int i = 0; i <= segs; ++i) {
                    float t = float(i) / float(segs);
                    float angle = startAngle + t * (3.14159265f * 0.5f);

                    float x = std::cos(angle) * radius;
                    float y = std::sin(angle) * radius;

                    outline.push_back({
                        ox + x * aspectFix,
                        oy + y
                        });
                }
                };

            addCorner(-cx, -cy, -3.14159265f);
            addCorner(cx, -cy, -3.14159265f * 0.5f);
            addCorner(cx, cy, 0.0f);
            addCorner(-cx, cy, 3.14159265f * 0.5f);

            for (size_t i = 0; i < outline.size(); ++i) {
                vec2<float> a = outline[i];
                vec2<float> b = outline[(i + 1) % outline.size()];

                Triangle tri;
                tri.vertices[0].position = { 0.f, 0.f, 0.f };
                tri.vertices[1].position = { a.x, a.y, 0.f };
                tri.vertices[2].position = { b.x, b.y, 0.f };

                tri.vertices[0].color = color;
                tri.vertices[1].color = color;
                tri.vertices[2].color = color;
                tri.normal = { 0.f, 0.f, -1.f };

                _mesh.triangles.push_back(tri);
            }
            };

        if (bw <= 0.f) {
            buildRoundedRect(rw, rh, r, style.baseColor);
            return;
        }

        buildRoundedRect(rw, rh, r, style.borderColor);

        float innerW = std::max(0.f, rw - 2.f * bw);
        float innerH = std::max(0.f, rh - 2.f * bw);
        float innerR = std::max(0.f, r - bw);

        innerR = std::min(innerR, std::min(innerW * 0.5f, innerH * 0.5f));

        buildRoundedRect(innerW, innerH, innerR, style.baseColor);
    }
};