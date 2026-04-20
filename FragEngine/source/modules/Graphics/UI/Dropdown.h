#pragma once

#include "modules/Graphics/UI/UIObject.h"
#include "modules/Graphics/UI/Font.h"
#include "modules/Graphics/UI/Button.h"

#include <vector>
#include <string>
#include <sstream>
#include <memory>
#include <type_traits>

namespace EnumReflect {
    template<typename T>
    struct Reflect {
        static std::vector<std::string> names() { return {}; }
    };
}

#define REFLECT_EXISTING_ENUM(EnumName, ...) \
namespace EnumReflect { \
template<> struct Reflect<EnumName> { \
    static std::vector<std::string> names() { \
        std::string raw = #__VA_ARGS__; \
        std::vector<std::string> result; \
        std::istringstream ss(raw); \
        std::string token; \
        while (std::getline(ss, token, ',')) { \
            auto s = token.find_first_not_of(" \t\n\r"); \
            auto e = token.find_last_not_of(" \t\n\r"); \
            if (s != std::string::npos) result.push_back(token.substr(s, e - s + 1)); \
        } \
        return result; \
    } \
}; }

#define REFLECT_ENUM(EnumName, ...) \
enum class EnumName { __VA_ARGS__ }; \
REFLECT_EXISTING_ENUM(EnumName, __VA_ARGS__)

template<typename T>
class Dropdown : public UIObject {
    static_assert(std::is_enum_v<T>, "Dropdown<T> requires T to be an enum type.");
public:
    using UIObject::UIObject;

    void create(T& boundValue,
        UITransform transform,
        UIStyle style,
        std::shared_ptr<Font> font = nullptr)
    {
        _boundValue = &boundValue;
        _transform = transform;
        _style = style;
        _font = font;
        _names = EnumReflect::Reflect<T>::names();
        createHeaderButton();
        rebuild();
    }

    void refresh() { rebuild(); }

    void toggle() {
        _isOpen = !_isOpen;
        rebuild();
    }

    Button* getHeaderButton() { return _headerButton; }

private:
    T* _boundValue = nullptr;
    std::vector<std::string> _names;
    std::shared_ptr<Font> _font;
    bool _isOpen = false;
    Button* _headerButton = nullptr;
    std::vector<Button*> _optionButtons;

    void rebuild() {
        _optionButtons.clear();
        _children.clear();
        createHeaderButton();

        if (_isOpen) addOptions();

        if (_headerButton) {
            int idx = static_cast<int>(*_boundValue);
            bool valid = idx >= 0 && idx < static_cast<int>(_names.size());
            _headerButton->setText(valid ? _names[idx] : "?");
        }
    }

void createHeaderButton() {
        UITransform headerTr = _transform;
        headerTr.position = { 0,0 };

        _headerButton = _parentUI->registerNew<Button>(_name + "_header");
        _headerButton->setParentUI(_parentUI);

        _headerButton->create(
            _names.empty() ? "?" : _names[0],
            _font,
            [this]() { toggle(); },
            _style,
            headerTr
        );

        _parentUI->setParent(_headerButton, this);
    }

    void addOptions() {
        for (int i = 0; i < static_cast<int>(_names.size()); ++i) {
            UITransform optTr = _transform;
            optTr.position = { 0,0 };
            optTr.position.y += float(i + 1) * optTr.size.y;

            UIStyle optSt = _style;
            if (*_boundValue == static_cast<T>(i)) optSt.baseColor = _style.secondaryColor;

            int capturedIdx = i;

            Button* btn = _parentUI->registerNew<Button>(_name + "_opt_" + std::to_string(i));
            btn->setParentUI(_parentUI);

            btn->create(
                _names[i],
                _font,
                [this, capturedIdx]() {
                    if (!_boundValue) return;
                    *_boundValue = static_cast<T>(capturedIdx);
                    _isOpen = false;
                    rebuild();
                },
                optSt,
                optTr
            );

            _optionButtons.push_back(btn);
            _parentUI->setParent(btn, this);
        }
    }
};