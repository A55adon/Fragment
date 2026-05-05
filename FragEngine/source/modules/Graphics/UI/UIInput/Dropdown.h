#pragma once

#include "modules/Graphics/UI/UIInput/Button.h"

template<typename T>
class Dropdown : public Button {
public:
    void bindValue(T* value) {
        _boundValue = value;
        updateCurrentLabel();
    }

    void setOptions(std::vector<std::pair<T, std::string>> options) {
        _options = std::move(options);
        rebuild();
    }

    void setOnValueChange(std::function<void(T)> fn) {
        _onValueChange = std::move(fn);
    }

    std::string getElementTypeName() const override { return "Dropdown"; }

    void rebuild() override {
        Button::rebuild();
        rebuildOptions();
        updateCurrentLabel();
    }

    void onClick() override {
        _open = !_open;
        rebuildOptions();
    }

private:
    void updateCurrentLabel() {
        if (!_boundValue) return;

        for (const auto& [value, label] : _options) {
            if (value == *_boundValue) {
                setLabel(label);
                return;
            }
        }
    }

    void rebuildOptions() {
        while (_children.size() > 1) {
            _children.pop_back();
        }

        _optionButtons.clear();
        if (!_open) return;

        for (size_t i = 0; i < _options.size(); ++i) {
            auto option = std::make_unique<Button>();
            option->setStyle(_style);
            option->setFont(_font);
            option->setLabel(_options[i].second);

            const vec2<float> parentSize = getTransform().getSize();
            Transform tr;
            tr.setPosition({ 0.0f, -parentSize.y * 1.1f * static_cast<float>(i + 1) });
            tr.setSize(parentSize);
            option->setTransform(tr);

            const T value = _options[i].first;
            option->setOnClick([this, value]() {
                if (_boundValue) {
                    *_boundValue = value;
                    if (_onValueChange) _onValueChange(value);
                }
                _open = false;
                rebuild();
            });

            option->rebuild();
            _optionButtons.push_back(static_cast<Button*>(addChild(std::move(option))));
        }
    }

    T* _boundValue = nullptr;
    bool _open = false;
    std::vector<std::pair<T, std::string>> _options;
    std::vector<Button*> _optionButtons;
    std::function<void(T)> _onValueChange;
};
