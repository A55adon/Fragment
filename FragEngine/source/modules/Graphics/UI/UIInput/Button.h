#pragma once

#include "modules/Graphics/UI/UIBase/Rectangle.h"
#include "modules/Graphics/UI/UIBase/Text.h"

class Button : public Rectangle {
public:
    void setLabel(const std::string& label) {
        _label = label;
        ensureLabel();
        _textChild->setText(label);
        layoutLabel();
    }

    void setFont(const std::shared_ptr<UIFont>& font) {
        _font = font;
        ensureLabel();
        _textChild->setFont(font);
        layoutLabel();
    }

    void setOnClick(std::function<void()> onClick) {
        _onClick = std::move(onClick);
    }

    std::string getElementTypeName() const override { return "Button"; }

    void rebuild() override {
        Rectangle::rebuild();
        ensureLabel();
        layoutLabel();
    }

    void onClick() override {
        if (_onClick) _onClick();
    }

protected:
    void ensureLabel() {
        if (_textChild) return;

        auto child = std::make_unique<Text>();
        child->setColor(_style.getSecondaryColor());
        child->setAlignment(Text::Alignment::Center);
        _textChild = static_cast<Text*>(addChild(std::move(child)));
        if (_font) _textChild->setFont(_font);
    }

    void layoutLabel() {
        if (!_textChild) return;

        Transform textTransform;
        textTransform.setPosition({ 0.0f, 0.0f });
        textTransform.setSizePx(_transform.getSizePx());
        _textChild->setTransform(textTransform);
        _textChild->setColor(_style.getSecondaryColor());
        _textChild->setPaddingPx(10.0f);
        if (_font) _textChild->setFont(_font);
        _textChild->setText(_label);
    }

    std::string _label = "Button";
    std::shared_ptr<UIFont> _font;
    Text* _textChild = nullptr;
    std::function<void()> _onClick;
};
