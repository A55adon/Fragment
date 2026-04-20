#include "modules/Graphics/UI/UI.h"
#include "modules/Graphics/UI/UIObject.h"
#include "modules/IO/UserInput/Input.h"

#include <cmath>
#include <algorithm>

UI::UI(Input* uiInput, Window* window) : _uiInput(uiInput), _window(window)
{
    _uiInput->setMouseButtonAction(GLFW_MOUSE_BUTTON_LEFT,
        [this](float x, float y) { handleClick(x, y); });
}

UI::~UI() {}

void UI::setParent(UIObject* child, UIObject* parent)
{
    for (auto it = objects.begin(); it != objects.end(); ++it)
    {
        if (it->get() == child) {
            parent->getChildren().push_back(std::move(*it));
            objects.erase(it);
            return;
        }
    }
    WARN("setParent: child not found in top-level UI objects");
}

void UI::handleClick(float mouseX, float mouseY)
{
    for (auto it = objects.rbegin(); it != objects.rend(); ++it)
        if ((*it)->handleClickRecursive(mouseX, mouseY))
            break;
}

void UI::update()
{
    if (!_window) return;

    GLFWwindow* win = _window->getRawWindow();
    if (!win) return;

    // Current cursor position in screen pixels
    double mx, my;
    glfwGetCursorPos(win, &mx, &my);
    float mouseX = static_cast<float>(mx);
    float mouseY = static_cast<float>(my);

    bool lmbDown = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    // ---- Button released ----
    if (!lmbDown) {
        _dragTarget = nullptr;
        _resizeTarget = nullptr;
        _resizeEdgeMask = 0;
        _lmbWasDown = false;
        _lastMouseX = mouseX;
        _lastMouseY = mouseY;
        return;
    }

    // ---- Button just pressed this frame — find interaction target ----
    if (!_lmbWasDown) {
        _lmbWasDown = true;
        _lastMouseX = mouseX;
        _lastMouseY = mouseY;
        _dragTarget = nullptr;
        _resizeTarget = nullptr;
        _resizeEdgeMask = 0;

        for (auto it = objects.rbegin(); it != objects.rend(); ++it) {
            int edgeMask = 0;
            UIObject* hit = (*it)->findInteractionTarget(mouseX, mouseY, { 0.f, 0.f }, edgeMask);
            if (hit) {
                if (edgeMask != 0 && hit->isResizable)
                    _resizeTarget = hit, _resizeEdgeMask = edgeMask;
                else if (hit->isDraggable)
                    _dragTarget = hit;
                break;
            }
        }
        return;
    }

    // ---- Button held — compute pixel delta and dispatch ----
    float dx = mouseX - _lastMouseX;
    float dy = mouseY - _lastMouseY;
    _lastMouseX = mouseX;
    _lastMouseY = mouseY;

    if (dx == 0.f && dy == 0.f) return;
    if (!_dragTarget && !_resizeTarget) return;

    // Convert pixel delta to 0-100 UI space
    int winW = 1, winH = 1;
    glfwGetWindowSize(win, &winW, &winH);
    float dxUI = (winW > 0) ? (dx / static_cast<float>(winW)) * 100.f : 0.f;
    float dyUI = (winH > 0) ? (dy / static_cast<float>(winH)) * 100.f : 0.f;

    if (_resizeTarget)
        _resizeTarget->onResize(dxUI, dyUI, _resizeEdgeMask);
    else if (_dragTarget)
        _dragTarget->onDrag(dxUI, dyUI);
}
