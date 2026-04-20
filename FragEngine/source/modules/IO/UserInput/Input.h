#pragma once

#include <unordered_map>
#include <functional>
#include <vector>
#include <GLFW/glfw3.h>

#include "core/DataTypes.h"

class Input {
public:
    using KeyCallback = std::function<void(float dt)>;
    using MouseMoveCallback = std::function<void(float dx, float dy)>;
    using MouseButtonCallback = std::function<void(float x, float y)>;
    using MouseButtonReleaseCallback = std::function<void()>;
    using KeyHeldCallback = std::function<void(float dt)>;
    using KeyPressedCallback = std::function<void()>;
    using KeyReleasedCallback = std::function<void()>;

    double mouseX = 0.0;   // absolute cursor position in pixels (always current)
    double mouseY = 0.0;

    void init(GLFWwindow* window) {
        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, keyCallbackDispatch);
        glfwSetCursorPosCallback(window, mouseMoveDispatch);
        glfwSetMouseButtonCallback(window, mouseButtonDispatch);
    }

    // Register a mouse-move callback that fires only in `activeState`.
    // Callbacks receive (dx, dy) — frame delta, not absolute position.
    // Defaults to EGameState::GAME so existing camera bindings need no changes.
    // Multiple calls ADD callbacks; they never replace each other.
    void setMouseMoveAction(MouseMoveCallback action,
        EGameState activeState = EGameState::GAME)
    {
        mouseMoveEntries.push_back({ std::move(action), activeState });
    }

    void setMouseButtonAction(int button, MouseButtonCallback action) {
        mouseButtonActions[button].push_back(action);
    }

    void setMouseButtonReleaseAction(int button, MouseButtonReleaseCallback action) {
        mouseButtonReleaseActions[button].push_back(action);
    }

    void setKeyHeld(int key, KeyHeldCallback     action) { keyHeldActions[key] = action; }
    void setKeyPressed(int key, KeyPressedCallback  action) { keyPressedActions[key] = action; }
    void setKeyReleased(int key, KeyReleasedCallback action) { keyReleasedActions[key] = action; }

    void update(float dt)
    {
        for (auto& [key, state] : currentKeys)
        {
            bool wasDown = previousKeys[key];
            bool isDown = state;

            if (isDown) {
                auto it = keyHeldActions.find(key);
                if (it != keyHeldActions.end()) it->second(dt);
            }
            if (isDown && !wasDown) {
                auto it = keyPressedActions.find(key);
                if (it != keyPressedActions.end()) it->second();
            }
            if (!isDown && wasDown) {
                auto it = keyReleasedActions.find(key);
                if (it != keyReleasedActions.end()) it->second();
            }
        }
        previousKeys = currentKeys;
    }

    void setGameState(EGameState state)
    {
        currentGameState = state;
        firstMouse = true;   // suppress a jump-delta when switching states
    }

private:
    struct MouseMoveEntry {
        MouseMoveCallback callback;
        EGameState        activeState;
    };

    std::unordered_map<int, KeyCallback>              keyActions;
    std::unordered_map<int, bool>                     currentKeys;
    std::unordered_map<int, bool>                     previousKeys;
    std::unordered_map<int, KeyHeldCallback>          keyHeldActions;
    std::unordered_map<int, KeyPressedCallback>       keyPressedActions;
    std::unordered_map<int, KeyReleasedCallback>      keyReleasedActions;

    std::unordered_map<int, std::vector<MouseButtonCallback>>        mouseButtonActions;
    std::unordered_map<int, std::vector<MouseButtonReleaseCallback>> mouseButtonReleaseActions;

    std::vector<MouseMoveEntry> mouseMoveEntries;

    EGameState currentGameState = EGameState::GAME;

    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    bool   firstMouse = true;

    // ── GLFW static dispatchers ──────────────────────────────────────────────

    static void keyCallbackDispatch(GLFWwindow* window,
        int key, int /*scancode*/,
        int action, int /*mods*/)
    {
        auto* h = static_cast<Input*>(glfwGetWindowUserPointer(window));
        if (!h) return;
        if (action == GLFW_PRESS)   h->currentKeys[key] = true;
        else if (action == GLFW_RELEASE) h->currentKeys[key] = false;
    }

    static void mouseMoveDispatch(GLFWwindow* window, double xpos, double ypos)
    {
        auto* h = static_cast<Input*>(glfwGetWindowUserPointer(window));
        if (!h) return;

        // Always keep absolute position current so sliders can read h->mouseX.
        h->mouseX = xpos;
        h->mouseY = ypos;

        if (h->firstMouse) {
            h->lastMouseX = xpos;
            h->lastMouseY = ypos;
            h->firstMouse = false;
            return;   // skip the large first-frame delta
        }

        float dx = static_cast<float>(xpos - h->lastMouseX);
        float dy = static_cast<float>(ypos - h->lastMouseY);
        h->lastMouseX = xpos;
        h->lastMouseY = ypos;

        // Dispatch only to callbacks registered for the current game state.
        for (auto& entry : h->mouseMoveEntries)
            if (entry.activeState == h->currentGameState)
                entry.callback(dx, dy);
    }

    static void mouseButtonDispatch(GLFWwindow* window,
        int button, int action, int /*mods*/)
    {
        auto* h = static_cast<Input*>(glfwGetWindowUserPointer(window));
        if (!h) return;

        if (action == GLFW_PRESS) {
            auto it = h->mouseButtonActions.find(button);
            if (it != h->mouseButtonActions.end())
                for (auto& cb : it->second)
                    cb(static_cast<float>(h->mouseX),
                        static_cast<float>(h->mouseY));
        }
        else if (action == GLFW_RELEASE) {
            auto it = h->mouseButtonReleaseActions.find(button);
            if (it != h->mouseButtonReleaseActions.end())
                for (auto& cb : it->second)
                    cb();
        }
    }
};