#pragma once

#include "modules/Graphics/UI/UIBase/Rectangle.h"
#include "modules/Graphics/UI/UIBase/Text.h"
#include "core/DefaultFunctions.h"

class Histogram : public Rectangle {
public:
    void bindValues(const std::vector<float>* values) { _values = values; }

    void rebuild() override {
        Rectangle::rebuild();
        if (!_values || _values->size() < 2) return;

        float maxValue = *std::max_element(_values->begin(), _values->end());
        float minValue = *std::min_element(_values->begin(), _values->end());

        if(minValue == maxValue) {
            WARN("Same value for min and max in Histogram");
            return;
        }

        auto& tris = _mesh.getTriangles();
        float width = _transform.getSize().x;
        float height = _transform.getSize().y;
        float left = _transform.getPosition().x - (width / 2.f);
        float bottom = _transform.getPosition().y - (height / 2.f);

        // Warn about to much data

        float lineThickness = 0.01f;
        Color lineColor = _style.getSecondaryColor();

        float widthPerEntry = (float)(_transform.getSizePx().x) / (float)(_values->size());

        //LOG(CFG_WINDOW_WIDTH << ";" << CFG_WINDOW_HEIGHT);
        //LOG("Size per entry PX: " << widthPerEntry);

        if (widthPerEntry < 5.f) WARN("Could not propperly fit " << _values->size() << " values on " << _transform.getSizePx().x << " pixels, some data may be truncated! Each entry only has " << widthPerEntry << " pixels of space, consider makeing the element bigger or log fewer data!");

        for (size_t i = 1; i < _values->size(); ++i) {

            float step = width / (float)(_values->size() - 1);
            float posX = left + step * i;
            
            //posX = _transform.getPositionPx().x - (_transform.getSizePx().x / 2.f) + ((_transform.getSizePx().x / _values->size()) * i);
            //LOG("Element at position " << posX);

            float t = ((*_values)[i] - minValue) / (maxValue - minValue);
            float posY = bottom + t * height;

            if (i + 1 < _values->size()) {
                float nextPosX = left + step * (i + 1);

                float nextT = ((*_values)[i + 1] - minValue) / (maxValue - minValue);
                float nextPosY = bottom + nextT * height;

                // Create triangles to draw linepiece from current value to next value
                Vertex2D v0{ {posX, posY}, lineColor};
                Vertex2D v1{ {posX + lineThickness, posY}, lineColor };
                Vertex2D v2{ {nextPosX, nextPosY}, lineColor };
                Vertex2D v3{ {nextPosX + lineThickness, nextPosY}, lineColor };

                tris.push_back({ v0, v1, v2 });
                tris.push_back({ v0, v2, v3 });
            }
        }
    }

    void update() override {
        rebuild();
    }

private:
    const std::vector<float>* _values = nullptr;
};
