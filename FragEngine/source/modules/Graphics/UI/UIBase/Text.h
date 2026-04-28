#pragma once

#include "modules/Graphics/UI/UIElement.h"
#include "modules/Graphics/UI/UIBase/Font.h"

class Text : public UIElement {
public:
    enum class Alignment {
        Left,
        Center,
        Right
    };

    Text() {
        setHitTestVisible(false);
    }

    void setText(const std::string& text);
    const std::string& getText() const { return _text; }

    void setFont(const std::shared_ptr<UIFont>& font);
    std::shared_ptr<UIFont> getFont() const { return _font; }

    void setFontSize(int size);

    void setAlignment(Alignment alignment) {
        _alignment = alignment;
        rebuild();
    }

    void setPaddingPx(float px) {
        _paddingPx = px;
        rebuild();
    }

    void setColor(const Color& color) {
        _color = color;
        rebuild();
    }

    float getTextWidthPx() const { return _textWidthPx; }
    float getTextHeightPx() const { return _textHeightPx; }

    std::string getElementTypeName() const override { return "Text"; }
    void rebuild() override;

private:
    std::string _text;
    std::shared_ptr<UIFont> _font;
    Alignment _alignment = Alignment::Left;
    float _paddingPx = 0.0f;
    float _textWidthPx = 0.0f;
    float _textHeightPx = 0.0f;
    Color _color = Color::White;
};
