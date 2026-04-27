#pragma once

#include "core/DefaultFunctions.h"
#include "core/config.h"

#include "glfw/glfw3.h"

class Window {
public:
	Window(vec2<int> size = vec2<int>(800, 600), std::string title = generateProjectName()) {

		ASSERT(glfwInit(), "Could not initialize GLFW");

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, CFG_GLFW_CONTEXT_VERSION_MAJOR);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, CFG_GLFW_CONTEXT_VERSION_MINOR);
		glfwWindowHint(GLFW_OPENGL_PROFILE, CFG_GLFW_OPENGL_PROFILE);

		glfwWindowHint(GLFW_SAMPLES, CFG_GLFW_SAMPLES);

		window = glfwCreateWindow(size.x, size.y, title.c_str(), nullptr, nullptr);
		if (!window)
			ERR("Could not create GLFWwindow");

		glfwMakeContextCurrent(window);
		glfwSwapInterval(1); // v-sync
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	};

	vec2<int> setFullscreen() {
		int count;
		GLFWmonitor** monitors = glfwGetMonitors(&count);

		GLFWmonitor* secondMonitor = monitors[1];
		const GLFWvidmode* mode = glfwGetVideoMode(secondMonitor);

		glfwSetWindowMonitor(
			window,
			secondMonitor,
			0,
			0,
			mode->width,
			mode->height,
			mode->refreshRate
		);

		return vec2<int>(mode->width, mode->height);
	}

	void setSize(vec2<int> size) {
		glfwSetWindowSize(window, size.x, size.y);
	}

	int getWidth() {
		int width;
		int height;

		glfwGetWindowSize(window, &width, &height);
		return width;
	}
	int getHeight() {
		int width;
		int height;

		glfwGetWindowSize(window, &width, &height);
		return height;
	}

	void setCursorFocus(bool focussed) {
		if (focussed) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	float getAspect() {
		return getWidth() / getHeight();
	}

	void setTitle(std::string title) {
		glfwSetWindowTitle(window, title.c_str());
	}

	void setVSync(bool vsync) {
		glfwSwapInterval(vsync ? 1 : 0);
	}

	bool shouldRun() { return !glfwWindowShouldClose(window); }



	GLFWwindow* getRawWindow() { 
		ASSERT(window, "Window is nullptr");
		return window; 
	};
private:
	GLFWwindow* window;
};