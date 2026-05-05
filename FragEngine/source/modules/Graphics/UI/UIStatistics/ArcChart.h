#pragma once

#include "modules/Graphics/UI/UIBase/Rectangle.h"
#include "modules/Graphics/UI/UIBase/Text.h"

class ArcChart : public Rectangle {
public:
    void bindValues(const std::vector<float>* values) { _values = values; }
    void setOptionLabels(std::vector<std::string> labels) { _labels = std::move(labels); }
    void setSegmentColors(std::vector<Color> colors) { _colors = std::move(colors); }

    std::string getElementTypeName() const override { return "ArcChart"; }

    void update() override {
        Rectangle::update();
        rebuild();
    }

    void rebuild() override {
        Rectangle::rebuild();
        _mesh.getTriangles().clear();

        if (!_values || _values->empty()) {
            clearLegend();
            return;
        }

        ensureLegendEntries(_values->size());

        float total = 0.0f;
        for (float value : *_values) total += std::max(0.0f, value);
        updateLegend(total);
        if (total <= 0.0f) return;

        auto& tris = _mesh.getTriangles();
        const vec2<float> halfSize = _transform.getHalfSizeNDC();
        float outerRx = halfSize.x;
        float outerRy = halfSize.y;
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
    void ensureLegendEntries(size_t count) {
        while (_legendLabels.size() < count) {
            auto label = std::make_unique<Text>();
            label->setAlignment(Text::Alignment::Center);
            label->setFontSize(12);
            _legendLabels.push_back(static_cast<Text*>(addChild(std::move(label))));
        }

        for (size_t i = 0; i < _legendLabels.size(); ++i) {
            _legendLabels[i]->setVisible(i < count);
        }
    }

    void clearLegend() {
        for (Text* label : _legendLabels) {
            label->setText("");
            label->setVisible(false);
        }
    }

    void updateLegend(float total) {
        const int widthPx = std::max(_transform.getSizePx().x * 2, 120);
        const float startOffset = _transform.getHalfSizeNDC().y + 0.05f;
        const float lineSpacing = 0.035f;

        for (size_t idx = 0; idx < _values->size(); ++idx) {
            const float value = std::max(0.0f, (*_values)[idx]);
            const float percentage = total > 0.0f ? (value / total) * 100.0f : 0.0f;
            const std::string& name = idx < _labels.size() ? _labels[idx] : std::format("Option {}", idx + 1);

            Transform labelTransform;
            labelTransform.setPosition({ 0.0f, -startOffset - static_cast<float>(idx) * lineSpacing });
            labelTransform.setSizePx({ widthPx, 18 });

            const Color color = idx < _colors.size() ? _colors[idx] : _style.getSecondaryColor();
            _legendLabels[idx]->setTransform(labelTransform);
            _legendLabels[idx]->setColor(color);
            _legendLabels[idx]->setVisible(true);
            _legendLabels[idx]->setText(std::format("{}: {:.2f} ms ({:.1f}%)", name, value, percentage));
        }
    }

    const std::vector<float>* _values = nullptr;
    std::vector<std::string> _labels;
    std::vector<Color> _colors;
    std::vector<Text*> _legendLabels;
};
