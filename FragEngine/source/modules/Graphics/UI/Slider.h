#pragma once

#include "modules/Graphics/UI/UIObject.h"
#include "modules/Graphics/UI/Rect.h"
#include "modules/Graphics/UI/Button.h"

template<typename T>
class Slider : public UIObject {
public:
    using UIObject::UIObject;

    void create(T& value, vec2<T> minMax, const UITransform& transform = UITransform(), const UIStyle& style = UIStyle()) {
        _boundValue = &value;
        _minMax = minMax;
        _transform = transform;
        _style = style;

        UIStyle barStyle = style;
        barStyle.cornerRoundness = 1.f;
        _bar = _parentUI->registerNew<Rect>(_name + "_bar");
        _bar->create(UITransform{ {0, 0}, {_transform.size.x * _parentUI->_window->getWidth() / _parentUI->_window->getHeight(), _transform.size.y * 0.3f}}, barStyle);
        _parentUI->setParent(_bar, this);

        _knob = _parentUI->registerNew<Rect>(_name + "_knob");

        float t = (float)(value - minMax.x) / (float)(minMax.y - minMax.x);
        float knobX = t * _transform.size.x - _transform.size.x / 2.f;

        _knob->create(UITransform{ {knobX, 0}, {2, _transform.size.y} }, style);
        _knob->isDraggable = true;
        _knob->setOnDrag([this](float dx, float dy) {
            // clamp knob within slider bounds
            UITransform ktr = _knob->getTransform();
            float halfW = _transform.size.x / 2.f;
            ktr.position.x = std::clamp(ktr.position.x + dx, -halfW, halfW);
            _knob->setTransform(ktr);
            _knob->create(ktr, _style);

            // map position back to value
            float t = (ktr.position.x + halfW) / _transform.size.x;
            *_boundValue = (T)(_minMax.x + t * (_minMax.y - _minMax.x));

            if (onValueChange) onValueChange(*_boundValue);
        });

        _parentUI->setParent(_knob, this);
    }

    std::function<void(T)> onValueChange;

private:
    T* _boundValue = nullptr;
    vec2<T> _minMax = {};
    Rect* _knob = nullptr;
    Rect* _bar = nullptr;
};