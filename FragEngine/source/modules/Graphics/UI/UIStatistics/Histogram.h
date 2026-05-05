#pragma once

#include "modules/Graphics/UI/UIBase/Rectangle.h"
#include "modules/Graphics/UI/UIBase/Text.h"
#include "core/DefaultFunctions.h"

class Histogram : public Rectangle {
public:
    void bindValues(const std::vector<float>* values) {
        _values = values;
        rebuild();
    }

    void rebuild() override {
        //Rectangle::rebuild();
        _lineVertices.clear();
        ensureLabels();

        if (!_values || _values->empty()) {
            _maxLabel->setText("");
            _minLabel->setText("");
            return;
        }

        float maxValue = *std::max_element(_values->begin(), _values->end());
        float minValue = *std::min_element(_values->begin(), _values->end());
        layoutLabels(maxValue, minValue);

        if (_values->size() < 2) return;

        if(minValue == maxValue) {
            //WARN("Same value for min and max in Histogram");
            return;
        }

        float width = _transform.getSize().x;
        float height = _transform.getSize().y;
        float left = -width / 2.f;
        float bottom = -height / 2.f;

        Color lineColor = _style.getSecondaryColor();

        float widthPerEntry = (float)(_transform.getSizePx().x) / (float)(_values->size());

        //LOG(CFG_WINDOW_WIDTH << ";" << CFG_WINDOW_HEIGHT);
        //LOG("Size per entry PX: " << widthPerEntry);

        //if (widthPerEntry < 5.f) WARN("Could not propperly fit " << _values->size() << " values on " << _transform.getSizePx().x << " pixels, some data may be truncated! Each entry only has " << widthPerEntry << " pixels of space, consider makeing the element bigger or log fewer data!");

        for (size_t i = 0; i < _values->size(); ++i) {
            float step = width / (float)(_values->size() - 1);
            float posX = left + step * i;

            float t = ((*_values)[i] - minValue) / (maxValue - minValue);
            float posY = bottom + t * height;
            _lineVertices.push_back({ {posX, posY}, lineColor });
        }
    }

    void update() override {
        rebuild();
    }

    void draw(vec2<float> parentPosition, Shader* shader, GLuint vbo, GLuint vao) override {
        Rectangle::draw(parentPosition, shader, vbo, vao);

        if (_lineVertices.size() < 2) return;

        const vec2<float> worldPos = parentPosition + _transform.getPosition();
        std::vector<Vertex2D> verts = _lineVertices;
        for (auto& vert : verts) {
            vert.position.x += worldPos.x;
            vert.position.y += worldPos.y;
        }

        shader->use();
        shader->setInt("useTexture", 0);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex2D) * verts.size(), verts.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(verts.size()));
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

private:
    void ensureLabels() {
        if (!_maxLabel) {
            auto label = std::make_unique<Text>();
            label->setAlignment(Text::Alignment::Center);
            label->setColor(_style.getTertiaryColor());
            label->setFontSize(14);
            _maxLabel = static_cast<Text*>(addChild(std::move(label)));
        }

        if (!_minLabel) {
            auto label = std::make_unique<Text>();
            label->setAlignment(Text::Alignment::Center);
            label->setColor(_style.getTertiaryColor());
            label->setFontSize(14);
            _minLabel = static_cast<Text*>(addChild(std::move(label)));
        }
    }

    void layoutLabels(float maxValue, float minValue) {
        const int widthPx = std::max(_transform.getSizePx().x, 40);
        //const float horizontalOffset = -0.07f;
        const float horizontalOffset = 0.06f;
        const vec2<float> halfSize = _transform.getHalfSizeNDC();

        float rightX = halfSize.x + horizontalOffset;

        Transform topTransform;
        topTransform.setPosition({ rightX, halfSize.y - 0.03f});
        topTransform.setSizePx({ widthPx, 18 });
        _maxLabel->setTransform(topTransform);
        _maxLabel->setColor(_style.getTertiaryColor());
        _maxLabel->setText(std::format("Highest: {:.2f}", maxValue));

        Transform bottomTransform;
        bottomTransform.setPosition({ rightX, -halfSize.y + 0.03f});
        bottomTransform.setSizePx({ widthPx, 18 });
        _minLabel->setTransform(bottomTransform);
        _minLabel->setColor(_style.getTertiaryColor());
        _minLabel->setText(std::format("Lowest: {:.2f}", minValue));
    }

    const std::vector<float>* _values = nullptr;
    std::vector<Vertex2D> _lineVertices;
    Text* _maxLabel = nullptr;
    Text* _minLabel = nullptr;
};
