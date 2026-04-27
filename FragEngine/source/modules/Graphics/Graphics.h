#pragma once

#include <string>

#include "core/DataTypes.h"
#include "core/DefaultFunctions.h"
#include "core/SceneObject.h"

#include "modules/Graphics/Shader.h"
#include "modules/Graphics/Window.h"
#include "modules/Graphics/Camera.h"
#include "modules/Graphics/LightSource.h"
#include "modules/Graphics/Texture.h"
#include "modules/Graphics/UI/UI.h"
#include "modules/Graphics/Renderer.h"

class Graphics {
public:
	Graphics() : _window(Window()) {
		Renderer::get()->initialize(&_window);
	};
	~Graphics() {
		glfwTerminate();
	};

	Camera* initNewCamera(std::string name) { return _cameras.createNew(name); }
	LightSource* initNewLight(std::string name) { return _lightSources.createNew(name); }

	Register<LightSource>& getLights() { return _lightSources; }
	Register<Camera>& getCameras() { return _cameras; }
	Window* getWindow() { return &_window; }

private:
	Register<Camera> _cameras;
	Window _window;
	Register<LightSource> _lightSources;

	vec2<int> _renderResolution{ 800, 600 };
	bool _useCustomResolution = false;

};