#pragma once

#include "modules/Graphics/UI/UIBase/Rectangle.h"
#include "modules/Graphics/UI/UIBase/Text.h"
#include <sstream>

template<typename T>
class Slider : public Rectangle {
public:
    Slider() = default;

    void bindValue(T* value) {
        _boundValue = value;
        rebuild();
    }

    void setRange(T minValue, T maxValue, T step) {
        _min = minValue;
        _max = maxValue;
        _step = step;
        rebuild();
    }

    void setFont(const std::shared_ptr<UIFont>& font) {
        _font = font;
        ensureValueLabel();
        _valueLabel->setFont(font);
        rebuild();
    }

    void setOnValueChange(std::function<void(T)> fn) {
        _onValueChange = std::move(fn);
    }

    std::string getElementTypeName() const override { return "Slider"; }

    void rebuild() override {
        Rectangle::rebuild();
        ensureParts();
        updateVisuals();
    }

    void onPointerDown(float mouseX, float mouseY) override {
        _sliding = true;
        updateFromMouse(mouseX);
    }

    void onPointerMove(float mouseX, float, bool lmbDown) override {
        if (!_sliding || !lmbDown) return;
        updateFromMouse(mouseX);
    }

    void onPointerUp(float, float) override {
        _sliding = false;
    }

private:
    void ensureParts() {
        if (!_fill) {
            auto fill = std::make_unique<Rectangle>();
            fill->setHitTestVisible(false);
            _fill = static_cast<Rectangle*>(addChild(std::move(fill)));
        }
        if (!_knob) {
            auto knob = std::make_unique<Rectangle>();
            knob->setHitTestVisible(false);
            _knob = static_cast<Rectangle*>(addChild(std::move(knob)));
        }
        ensureValueLabel();
    }

    void ensureValueLabel() {
        if (_valueLabel) return;
        auto label = std::make_unique<Text>();
        label->setAlignment(Text::Alignment::Center);
        _valueLabel = static_cast<Text*>(addChild(std::move(label)));
        if (_font) _valueLabel->setFont(_font);
    }

    float normalizedValue() const {
        if (!_boundValue || _max == _min) return 0.0f;
        return std::clamp((static_cast<float>(*_boundValue) - static_cast<float>(_min)) / (static_cast<float>(_max) - static_cast<float>(_min)), 0.0f, 1.0f);
    }

    void updateVisuals() {
        if (!_fill || !_knob || !_valueLabel) return;

        float t = normalizedValue();
        auto style = _style;
        auto fillStyle = _style;
        fillStyle.setPrimaryColor(_style.getSecondaryColor());

        Transform fillTransform;
        fillTransform.setPosition({
            -_transform.getSizeUS().x + (_transform.getSizeUS().x * t),
            0.0f
        });
        fillTransform.setSizeUS({
            std::max(0.01f, _transform.getSizeUS().x * t),
            _transform.getSizeUS().y * 0.55f
        });
        _fill->setTransform(fillTransform);
        _fill->setStyle(fillStyle);
        _fill->rebuild();

        Transform knobTransform;
        knobTransform.setPosition({
            -_transform.getSizeUS().x + (_transform.getSizeUS().x * 2.0f * t),
            0.0f
        });
        knobTransform.setSizeUS({
            std::max(0.012f, _transform.getSizeUS().y * 0.35f),
            std::max(0.018f, _transform.getSizeUS().y * 0.9f)
        });
        _knob->setTransform(knobTransform);
        _knob->setStyle(style);
        _knob->rebuild();

        Transform labelTransform;
        labelTransform.setPosition({ 0.0f, _transform.getSizeUS().y * 0.1f });
        labelTransform.setSizePx({ _transform.getSizePx().x, 16 });
        _valueLabel->setTransform(labelTransform);
        if (_font) _valueLabel->setFont(_font);

        std::ostringstream oss;
        if (_boundValue) oss << *_boundValue;
        _valueLabel->setText(oss.str());
    }

    void updateFromMouse(float mouseX) {
        auto pos = getTransform().getPositionPx();
        auto size = getTransform().getSizePx();
        float left = static_cast<float>(pos.x - size.x / 2);
        float t = std::clamp((mouseX - left) / static_cast<float>(std::max(size.x, 1)), 0.0f, 1.0f);

        float rawValue = static_cast<float>(_min) + t * (static_cast<float>(_max) - static_cast<float>(_min));
        if (_step > T{}) {
            rawValue = std::round(rawValue / static_cast<float>(_step)) * static_cast<float>(_step);
        }

        if (_boundValue) {
            *_boundValue = static_cast<T>(rawValue);
            if (_onValueChange) _onValueChange(*_boundValue);
        }

        updateVisuals();
    }

    T* _boundValue = nullptr;
    T _min{};
    T _max{ 1 };
    T _step{};
    bool _sliding = false;
    Rectangle* _fill = nullptr;
    Rectangle* _knob = nullptr;
    Text* _valueLabel = nullptr;
    std::shared_ptr<UIFont> _font;
    std::function<void(T)> _onValueChange;
};
