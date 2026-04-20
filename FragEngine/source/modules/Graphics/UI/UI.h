#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <algorithm>
#include "core/DataTypes.h"
#include "modules/Graphics/UI/UIObject.h"

class Input;
class Window;

class UI { // UISpace should be 0 - 100
public:
    UI(Input* uiInput, Window* window);
    ~UI();

    template<typename T = UIObject>
    T* registerNew(const std::string& name = "") {
        static_assert(std::is_base_of_v<UIObject, T>, "T must derive from UIObject");
        auto obj = std::make_unique<T>(this);
        T* ptr = obj.get();
        ptr->setName(name);
        objects.push_back(std::move(obj));
        return ptr;
    }

    void remove(UIObject* obj) {
        if (!obj) return;

        for (auto it = objects.begin(); it != objects.end(); ++it) {
            if (it->get() == obj) {
                objects.erase(it);
                return;
            }
        }
    }

    void update();

    void setParent(UIObject* child, UIObject* parent);
    void handleClick(float mouseX, float mouseY);

    std::vector<std::unique_ptr<UIObject>> objects;

    Input* _uiInput;
    Window* _window;
private:
 
    UIObject* _dragTarget = nullptr;
    UIObject* _resizeTarget = nullptr;
    int       _resizeEdgeMask = 0;

    bool  _lmbWasDown = false;
    float _lastMouseX = 0.f;
    float _lastMouseY = 0.f;
};