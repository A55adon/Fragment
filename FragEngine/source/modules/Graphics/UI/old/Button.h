#pragma once

#include "modules/Graphics/UI/UIObject.h"
#include "modules/Graphics/UI/Font.h"
#include "modules/Graphics/UI/Rect.h"
#include "modules/Graphics/UI/Text.h"

class Button : public UIObject {
public:
    using UIObject::UIObject;

    void create(std::string text = "Button", std::shared_ptr<Font> font = std::make_shared<Font>(), std::function<void()> onClick = {}, UIStyle style = UIStyle(), UITransform transform = UITransform())
    {
        _transform = transform;

        UITransform tr;
        tr.position = { 0,0 };
        tr.size = transform.size;

        rect = _parentUI->registerNew<Rect>(_name + "_rect_child");
        rect->setParentUI(_parentUI);
        rect->create(tr, style);
        rect->setOnClick(onClick);

        _parentUI->setParent(rect, this);

        txt = _parentUI->registerNew<Text>(_name + "_text_child");
        txt->setParentUI(_parentUI);
        txt->setFont(font);
        txt->setPositionPercent(tr.position.x, tr.position.y + (font->getSize() * (100.f / _parentUI->_window->getHeight()) / 2.5f));
        txt->setAlignment(Text::Alignment::Center);
        txt->setText(text);

        _parentUI->setParent(txt, this);
    }

    void onResize(float dx, float dy, int edgeMask) override {}
    void onDrag(float dx, float dy) override {
        UITransform tr;
        tr.position.x = dx;
        tr.position.y = dy;
        rect->create(tr, _style);

        txt->setPositionPercent(tr.position.x, tr.position.y);
    }

    void setText(std::string text) {
        if(txt)
            txt->setText(text);
    }

    Text* getText() { return txt; }
    Rect* getRect() { return rect; }

private:
    Rect* rect;
    Text* txt;
};