#pragma once

#include "modules/Graphics/UI/UIBase/Rectangle.h"

class ArcChart : public Rectangle {
public:
    void bindValues(const std::vector<float>* values) { _values = values; }
    void setSegmentColors(std::vector<Color> colors) { _colors = std::move(colors); }

    std::string getElementTypeName() const override { return "ArcChart"; }

    void update() override {
        Rectangle::update();
        rebuild();
    }

    void rebuild() override {
        Rectangle::rebuild();
        if (!_values || _values->empty()) return;

        float total = 0.0f;
        for (float value : *_values) total += std::max(0.0f, value);
        if (total <= 0.0f) return;

        auto& tris = _mesh.getTriangles();
        float outerRx = _transform.getSizeUS().x;
        float outerRy = _transform.getSizeUS().y;
        float innerRx = outerRx * 0.45f;
        float innerRy = outerRy * 0.45f;
        const int segments = 24;

        float start = 0.0f;
        for (size_t idx = 0; idx < _values->size(); ++idx) {
            float value = std::max(0.0f, (*_values)[idx]);
            float angleSpan = (value / total) * 6.28318530718f;
            Color color = idx < _colors.size() ? _colors[idx] : _style.getSecondaryColor();

            for (int i = 0; i < segments; ++i) {
                float a0 = start + angleSpan * (static_cast<float>(i) / segments);
                float a1 = start + angleSpan * (static_cast<float>(i + 1) / segments);

                Vertex2D o0{ { std::cos(a0) * outerRx, std::sin(a0) * outerRy }, {}, color };
                Vertex2D o1{ { std::cos(a1) * outerRx, std::sin(a1) * outerRy }, {}, color };
                Vertex2D i0{ { std::cos(a0) * innerRx, std::sin(a0) * innerRy }, {}, color };
                Vertex2D i1{ { std::cos(a1) * innerRx, std::sin(a1) * innerRy }, {}, color };

                tris.push_back({ o0, i0, i1 });
                tris.push_back({ o0, i1, o1 });
            }

            start += angleSpan;
        }
    }

private:
    const std::vector<float>* _values = nullptr;
    std::vector<Color> _colors;
};
