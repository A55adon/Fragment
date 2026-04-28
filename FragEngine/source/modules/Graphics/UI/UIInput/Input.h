#pragma once

#include "modules/Graphics/UI/UIInput/Button.h"
#include "glfw/glfw3.h"
#include <charconv>
#include <sstream>

template<typename T>
class InputField : public Button {
public:
    InputField() {
        setFocusTarget(true);
    }

    void bindValue(T* value) {
        _boundValue = value;
        if (_boundValue) {
            _buffer = toString(*_boundValue);
        }
        setLabel(_buffer);
    }

    void setOnValueChange(std::function<void(const T&)> fn) {
        _onValueChange = std::move(fn);
    }

    std::string getElementTypeName() const override { return "InputField"; }

    void update() override {
        Button::update();
        if (!isFocused()) return;

        GLFWwindow* window = glfwGetCurrentContext();
        if (!window) return;

        const double now = glfwGetTime();
        if (now - _lastPollTime < 0.04) return;
        _lastPollTime = now;

        if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS && !_buffer.empty()) {
            _buffer.pop_back();
            syncText();
            commit();
            return;
        }

        if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS && (_buffer.empty() || _buffer == "-")) {
            _buffer = "-";
            syncText();
            return;
        }

        for (int key = GLFW_KEY_0; key <= GLFW_KEY_9; ++key) {
            if (glfwGetKey(window, key) == GLFW_PRESS) {
                _buffer.push_back(static_cast<char>('0' + (key - GLFW_KEY_0)));
                syncText();
                commit();
                return;
            }
        }

        if constexpr (std::is_floating_point_v<T>) {
            if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS && _buffer.find('.') == std::string::npos) {
                _buffer.push_back('.');
                syncText();
            }
        }
    }

    void onClick() override {
        setFocused(true);
    }

private:
    static std::string toString(const T& value) {
        if constexpr (std::is_same_v<T, std::string>) {
            return value;
        }
        else {
            std::ostringstream oss;
            oss << value;
            return oss.str();
        }
    }

    void syncText() {
        setLabel(_buffer.empty() ? "_" : _buffer);
    }

    void commit() {
        if (!_boundValue) return;

        if constexpr (std::is_same_v<T, std::string>) {
            *_boundValue = _buffer;
            if (_onValueChange) _onValueChange(*_boundValue);
        }
        else {
            std::istringstream iss(_buffer);
            T parsed{};
            if (iss >> parsed) {
                *_boundValue = parsed;
                if (_onValueChange) _onValueChange(*_boundValue);
            }
        }
    }

    T* _boundValue = nullptr;
    std::string _buffer;
    std::function<void(const T&)> _onValueChange;
    double _lastPollTime = 0.0;
};
