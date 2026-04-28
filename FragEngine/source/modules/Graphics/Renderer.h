#pragma once

#include "core/DataTypes.h"
#include "core/DefaultFunctions.h"
#include "core/SceneObject.h"
#include "core/Scene.h"
#include "modules/Graphics/Window.h"
#include "modules/Graphics/Shader.h"
#include "modules/Graphics/Camera.h"
#include "modules/Graphics/LightSource.h"
#include "modules/Graphics/UI/UI.h"
#include "core/config.h"
#include "core/Mesh2D.h"
#include "modules/Graphics/UI/UIElement.h"

class Renderer {
public:
	static Renderer* get() {
		static Renderer instance;
		return &instance;
	}

	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	void initialize(Window* window);

	void setCustomResolution(vec2<int> size);

	// Drawing functions
	void drawScene(Scene* scene, Camera* camera, std::vector<std::unique_ptr<LightSource>>& lights);
	void drawUI(UI* ui);
	void clear();
	void present();

private:
	Renderer() = default;
	~Renderer() = default;

	void drawUIObject(UIElement* uiElement, vec2<float> parentPos);
	void renderShadowPass(Scene* scene, LightSource* light);

	inline bool isCustomResolution() { 
		return _resolution != vec2<int>( _window->getWidth(), _window->getHeight() ); 
	}

	Window* _window;


	// Normal 3D Mesh rendering
	GLuint _modelVAO, _modelVBO;
	Shader _modelShader;

	// UI rendering
	GLuint _uiVAO, _uiVBO;
	Shader _uiShader;

	// Custom resolution shader
	vec2<int> _resolution;
	GLuint _resolutionVAO, _resolutionVBO, _resolutionFBO, _resolutionRBO;
	GLuint _FBOTexture;
	Shader _resolutionShader;

	// Shaddow map
	GLuint _shadowFBO;
	GLuint _shadowMap;
	Shader _shadowShader;

	mat4 _lightSpaceMatrix;
	const unsigned int SHADOW_SIZE = CFG_SHADOW_TEXTURE_SIZE;
};