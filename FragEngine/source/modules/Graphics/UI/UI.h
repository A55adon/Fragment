#pragma once

#include "core/config.h"
#include "core/DataTypes.h"
#include "modules/Graphics/UI/UIElement.h"
#include "modules/Graphics/UI/UIBase/Rectangle.h"
#include "modules/Graphics/UI/UIBase/Text.h"
#include "modules/Graphics/UI/UIInput/Button.h"
#include "modules/Graphics/UI/UIInput/Input.h"
#include "modules/Graphics/UI/UIInput/Slider.h"
#include "modules/Graphics/UI/UIInput/Dropdown.h"
#include "modules/Graphics/UI/UIStatistics/Histogram.h"
#include "modules/Graphics/UI/UIStatistics/ArcChart.h"
#include <array>
#include <unordered_map>
// UI API
/*
Derivative: UIElement (transform, style, (animationhandler), drag/resize) + debugdraw()->objectname, scale, pos, etc
Types:
- Base:
    - Rectangle(Rounded) ->()
    - Text -> (text, (font))
- Input:
    - Button ->(onClick)
    - Dropdown<Enum> ->(optionsEnum, toChangeEnumValue) + onValueChange()
    - Input field<datatype> ->(toChangeValue) + onValueChange()
    - Slider<datatype> -> (toChangeValue, min/max, step) + onValueChange()
- Statistics:
    - Histogram (line with past values)<datatype> -> (toShowVector)
    - Pie/Arc chart<datatype> -> (toShowData ? like vector or sum, colors for each data)

Factory:
    create[ObjectName](Args...,Transform,Style);

Transform:
    - position ->(%/px)
    - size ->(%/px)
Style:
    - primaryColor
    - secondaryColor
    - tertiaryColor

    - border bool
    - border width
    - border radius
    - border color

UI -> UIElement -> Base -> Input
                        -> Statistics
*/


class UI {
public:
    UI() = default;

    UIElement* createUIElement(Transform transform = Transform(), Style style = Style());
    Rectangle* createRectangle(Transform transform = Transform(), Style style = Style());
    Text* createText(const std::string& text, Transform transform = Transform(), Style style = Style(), int fontSize = 18);
    Button* createButton(const std::string& label, std::function<void()> onClick, Transform transform = Transform(), Style style = Style(), int fontSize = 18);

    template<typename T>
    InputField<T>* createInputField(T* value, Transform transform = Transform(), Style style = Style(), int fontSize = 18) {
        auto field = std::make_unique<InputField<T>>();
        field->setTransform(transform);
        field->setStyle(style);
        field->setFont(getDefaultFont(fontSize));
        field->bindValue(value);
        field->rebuild();
        InputField<T>* ptr = field.get();
        _rootElements.push_back(std::move(field));
        return ptr;
    }

    template<typename T>
    Slider<T>* createSlider(T* value, T minValue, T maxValue, T step, Transform transform = Transform(), Style style = Style(), int fontSize = 16) {
        auto slider = std::make_unique<Slider<T>>();
        slider->setTransform(transform);
        slider->setStyle(style);
        slider->setRange(minValue, maxValue, step);
        slider->bindValue(value);
        slider->setFont(getDefaultFont(fontSize));
        slider->rebuild();
        Slider<T>* ptr = slider.get();
        _rootElements.push_back(std::move(slider));
        return ptr;
    }

    template<typename T>
    Dropdown<T>* createDropdown(T* value, std::vector<std::pair<T, std::string>> options, Transform transform = Transform(), Style style = Style(), int fontSize = 18) {
        auto dropdown = std::make_unique<Dropdown<T>>();
        dropdown->setTransform(transform);
        dropdown->setStyle(style);
        dropdown->bindValue(value);
        dropdown->setOptions(std::move(options));
        dropdown->setFont(getDefaultFont(fontSize));
        dropdown->rebuild();
        Dropdown<T>* ptr = dropdown.get();
        _rootElements.push_back(std::move(dropdown));
        return ptr;
    }

    Histogram* createHistogram(const std::vector<float>* values, Transform transform = Transform(), Style style = Style());
    ArcChart* createArcChart(const std::vector<float>* values, const std::vector<Color>& colors, Transform transform = Transform(), Style style = Style(), const std::vector<std::string>& labels = {});

    void update(float mouseX, float mouseY, bool lmbDown);

    std::vector<UIElement*> getRootElements();

    void setDebugDraw(bool dd) { _debugDraw = dd; }
    void debugDraw();
    std::shared_ptr<UIFont> getDefaultFont(int size = 18);
private:
    std::vector<std::unique_ptr<UIElement>> _rootElements;
    std::vector<std::unique_ptr<UIElement>> _debugOverlayElements;
    std::unordered_map<int, std::shared_ptr<UIFont>> _fonts;

    bool _debugDraw = false;

    // Interaction
    UIElement* _dragTarget = nullptr;
    UIElement* _resizeTarget = nullptr;
    UIElement* _pressTarget = nullptr;
    UIElement* _focusedElement = nullptr;
    int _resizeEdgeMask = 0;

    bool _lmbWasDown = false;
    float _lastMouseX = 0.0f;
    float _lastMouseY = 0.0f;
};
