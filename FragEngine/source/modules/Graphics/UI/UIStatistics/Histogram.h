#pragma once

#include "modules/Graphics/UI/UIBase/Rectangle.h"

class Histogram : public Rectangle {
public:
    void bindValues(const std::vector<float>* values) { _values = values; }

    std::string getElementTypeName() const override { return "Histogram"; }

    void update() override {
        Rectangle::update();
        rebuild();
    }

    void rebuild() override {
        Rectangle::rebuild();
        if (!_values || _values->size() < 2) return;

        float maxValue = *std::max_element(_values->begin(), _values->end());
        if (maxValue <= 0.0f) maxValue = 1.0f;

        auto& tris = _mesh.getTriangles();
        Color lineColor = _style.getSecondaryColor();
        float width = _transform.getSizeUS().x * 2.0f;
        float height = _transform.getSizeUS().y * 2.0f;
        float left = -width * 0.5f;
        float bottom = -height * 0.5f;
        float thickness = std::max(1.0f, _style.getBorderWidth() * CFG_WINDOW_HEIGHT) / CFG_WINDOW_HEIGHT * 2.0f;

        for (size_t i = 1; i < _values->size(); ++i) {
            float t0 = static_cast<float>(i - 1) / static_cast<float>(_values->size() - 1);
            float t1 = static_cast<float>(i) / static_cast<float>(_values->size() - 1);
            vec2<float> p0{ left + width * t0, bottom + height * ((*_values)[i - 1] / maxValue) };
            vec2<float> p1{ left + width * t1, bottom + height * ((*_values)[i] / maxValue) };

            vec2<float> dir{ p1.x - p0.x, p1.y - p0.y };
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len <= 1e-6f) continue;
            dir.x /= len;
            dir.y /= len;
            vec2<float> normal{ -dir.y * thickness, dir.x * thickness };

            Vertex2D v0{ { p0.x + normal.x, p0.y + normal.y }, {}, lineColor };
            Vertex2D v1{ { p0.x - normal.x, p0.y - normal.y }, {}, lineColor };
            Vertex2D v2{ { p1.x - normal.x, p1.y - normal.y }, {}, lineColor };
            Vertex2D v3{ { p1.x + normal.x, p1.y + normal.y }, {}, lineColor };
            tris.push_back({ v0, v1, v2 });
            tris.push_back({ v0, v2, v3 });
        }
    }

private:
    const std::vector<float>* _values = nullptr;
};
