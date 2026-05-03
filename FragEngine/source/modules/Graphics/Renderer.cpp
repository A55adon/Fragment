#include "Renderer.h"

#include <limits>

namespace {
	struct ShadowBounds {
		vec3<float> min{
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max()
		};
		vec3<float> max{
			-std::numeric_limits<float>::max(),
			-std::numeric_limits<float>::max(),
			-std::numeric_limits<float>::max()
		};
		bool valid = false;
	};

	void includePoint(ShadowBounds& bounds, const vec3<float>& point)
	{
		bounds.min.x = std::min(bounds.min.x, point.x);
		bounds.min.y = std::min(bounds.min.y, point.y);
		bounds.min.z = std::min(bounds.min.z, point.z);
		bounds.max.x = std::max(bounds.max.x, point.x);
		bounds.max.y = std::max(bounds.max.y, point.y);
		bounds.max.z = std::max(bounds.max.z, point.z);
		bounds.valid = true;
	}

	vec3<float> transformPoint(const mat4& matrix, const vec3<float>& point)
	{
		return toVec3(matrix * vec4<float>(point.x, point.y, point.z, 1.0f));
	}

	vec3<float> getShadowTarget(const std::vector<std::unique_ptr<SceneObject>>& objects)
	{
		vec3<float> target{ 0.0f, 0.0f, 0.0f };
		for (const auto& obj : objects) {
			target += obj->getPosition();
		}
		target *= (1.0f / static_cast<float>(objects.size()));
		return target;
	}

	ShadowBounds getLightSpaceBounds(const std::vector<std::unique_ptr<SceneObject>>& objects, const mat4& lightView)
	{
		ShadowBounds bounds;
		for (const auto& obj : objects) {
			const mat4 model = obj->getModelMatrix();
			bool includedMeshVertex = false;

			for (const auto& tri : obj->getRenderMesh().triangles) {
				for (const auto& vertex : tri.vertices) {
					const vec3<float> worldPos = transformPoint(model, vertex.position);
					includePoint(bounds, transformPoint(lightView, worldPos));
					includedMeshVertex = true;
				}
			}

			if (!includedMeshVertex) {
				includePoint(bounds, transformPoint(lightView, obj->getPosition()));
			}
		}
		return bounds;
	}
}

void Renderer::initialize(Window* window)
{
	// Check if window is valid
	ASSERT(window, "Window not vaild at Renderer initialization");
	_window = window;
	_resolution = { window->getWidth(), window->getHeight() };
	CFG_WINDOW_WIDTH = static_cast<float>(_resolution.x);
	CFG_WINDOW_HEIGHT = static_cast<float>(_resolution.y);

	// Check if glad function loading works
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		ERR("Failed to initialize GLAD");
		return;
	}

	if (CFG_GL_ENABLE_MULTISAMPLING)
		glEnable(GL_MULTISAMPLE);

	if (CFG_GL_ENABLE_DEPTH_TEST)
		glEnable(GL_DEPTH_TEST);

	// Model buffers setup
	glGenVertexArrays(1, &_modelVAO);
	glGenBuffers(1, &_modelVBO);

	glBindVertexArray(_modelVAO);
	glBindBuffer(GL_ARRAY_BUFFER, _modelVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 3, nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoordinate));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	_modelShader = Shader("source/modules/Graphics/model.vert", "source/modules/Graphics/model.frag");

	// UI buffers setup
	glGenVertexArrays(1, &_uiVAO);
	glGenBuffers(1, &_uiVBO);

	glBindVertexArray(_uiVAO);
	glBindBuffer(GL_ARRAY_BUFFER, _uiVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex2D) * 3, nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, position));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, texCoordinate));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, color));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	_uiShader = Shader("source/modules/Graphics/UI/UI.vert", "source/modules/Graphics/UI/UI.frag");

	// Custom resolution buffers setup
	glGenVertexArrays(1, &_resolutionVAO);
	glGenBuffers(1, &_resolutionVBO);
	glGenFramebuffers(1, &_resolutionFBO);
	glGenTextures(1, &_FBOTexture);
	glGenRenderbuffers(1, &_resolutionRBO);

	glBindVertexArray(_resolutionVAO);
	glBindBuffer(GL_ARRAY_BUFFER, _resolutionVBO);
	glBindFramebuffer(GL_FRAMEBUFFER, _resolutionFBO);
	glBindTexture(GL_TEXTURE_2D, _FBOTexture);
	glBindRenderbuffer(GL_RENDERBUFFER, _resolutionRBO);

	// Displayed texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _resolution.x, _resolution.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, CFG_CUSTOM_RESOLUTION_FILTERING);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, CFG_CUSTOM_RESOLUTION_FILTERING);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _FBOTexture, 0);

	// Depth renderbuffer
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _resolution.x, _resolution.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _resolutionRBO);

	/// Fullscreen quad
	float quadVerts[] = {
	-1, -1,  0, 0,
	 1, -1,  1, 0,
	 1,  1,  1, 1,
	-1, -1,  0, 0,
	 1,  1,  1, 1,
	-1,  1,  0, 1,
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	_resolutionShader = Shader("source/modules/Graphics/resolution.vert", "source/modules/Graphics/resolution.frag");

	// Shadow mapping
	glGenFramebuffers(1, &_shadowFBO);

	glGenTextures(1, &_shadowMap);
	glBindTexture(GL_TEXTURE_2D, _shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_SIZE, SHADOW_SIZE, 0,
		GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, _shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, _shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	_shadowShader = Shader("source/modules/Graphics/shadow.vert", "source/modules/Graphics/shadow.frag");

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glLineWidth(CFG_GL_LINEWIDTH);

	LOG("GL config result: " + std::to_string(glGetError()));
}

void Renderer::setCustomResolution(vec2<int> size)
{
	_resolution = size;
	CFG_WINDOW_WIDTH = static_cast<float>(_resolution.x);
	CFG_WINDOW_HEIGHT = static_cast<float>(_resolution.y);
	glGenVertexArrays(1, &_resolutionVAO);
	glGenBuffers(1, &_resolutionVBO);
	glGenFramebuffers(1, &_resolutionFBO);
	glGenTextures(1, &_FBOTexture);
	glGenRenderbuffers(1, &_resolutionRBO);

	glBindVertexArray(_resolutionVAO);
	glBindBuffer(GL_ARRAY_BUFFER, _resolutionVBO);
	glBindFramebuffer(GL_FRAMEBUFFER, _resolutionFBO);
	glBindTexture(GL_TEXTURE_2D, _FBOTexture);
	glBindRenderbuffer(GL_RENDERBUFFER, _resolutionRBO);

	// Displayed texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _resolution.x, _resolution.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, CFG_CUSTOM_RESOLUTION_FILTERING);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, CFG_CUSTOM_RESOLUTION_FILTERING);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _FBOTexture, 0);

	// Depth renderbuffer
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _resolution.x, _resolution.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _resolutionRBO);

	/// Fullscreen quad
	float quadVerts[] = {
	-1, -1,  0, 0,
	 1, -1,  1, 0,
	 1,  1,  1, 1,
	-1, -1,  0, 0,
	 1,  1,  1, 1,
	-1,  1,  0, 1,
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	_resolutionShader = Shader("source/modules/Graphics/resolution.vert", "source/modules/Graphics/resolution.frag");

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

vec4<int> Renderer::getPresentationViewport() const
{
	int windowWidth = _window->getWidth();
	int windowHeight = _window->getHeight();

	if (_resolution.x <= 0 || _resolution.y <= 0 || windowWidth <= 0 || windowHeight <= 0) {
		return { 0, 0, windowWidth, windowHeight };
	}

	const float renderAspect = static_cast<float>(_resolution.x) / static_cast<float>(_resolution.y);
	const float windowAspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

	int viewportWidth = windowWidth;
	int viewportHeight = windowHeight;

	if (windowAspect > renderAspect) {
		viewportWidth = static_cast<int>(std::round(static_cast<float>(windowHeight) * renderAspect));
	}
	else {
		viewportHeight = static_cast<int>(std::round(static_cast<float>(windowWidth) / renderAspect));
	}

	const int viewportX = (windowWidth - viewportWidth) / 2;
	const int viewportY = (windowHeight - viewportHeight) / 2;
	return { viewportX, viewportY, viewportWidth, viewportHeight };
}

vec2<float> Renderer::mapWindowToRenderCoordinates(float mouseX, float mouseY) const
{
	const auto viewport = getPresentationViewport();
	const int windowHeight = _window->getHeight();
	const float viewportTop = static_cast<float>(windowHeight - viewport.y - viewport.w);

	if (viewport.z <= 0 || viewport.w <= 0) {
		return { mouseX, mouseY };
	}

	const float localX = std::clamp(mouseX - static_cast<float>(viewport.x), 0.0f, static_cast<float>(viewport.z));
	const float localY = std::clamp(mouseY - viewportTop, 0.0f, static_cast<float>(viewport.w));
	const float flippedY = static_cast<float>(viewport.w) - localY;

	return {
		(localX / static_cast<float>(viewport.z)) * static_cast<float>(_resolution.x),
		(flippedY / static_cast<float>(viewport.w)) * static_cast<float>(_resolution.y)
	};
}

void Renderer::drawScene(Scene* scene, Camera* camera, std::vector<std::unique_ptr<LightSource>>& lights)
{
	ASSERT(scene, "Tried to draw invalid scene");
	ASSERT(camera, "Tried to draw with invalid camera");

	if (!lights.empty()) {
		renderShadowPass(scene, lights[0].get(), camera);
	}
	if (isCustomResolution())
		glBindFramebuffer(GL_FRAMEBUFFER, _resolutionFBO);
	glViewport(0, 0, _resolution.x, _resolution.y);

	_modelShader.use();
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _shadowMap);

	_modelShader.setInt("shadowMap", 1);
	_modelShader.setMat4("lightSpaceMatrix", _lightSpaceMatrix);
	_modelShader.setVec3("shadowLightDirection", _shadowLightDirection);
	// Tell the shader how many light there should be
	_modelShader.setInt("lightCount", lights.size());

	// Send lighting data to shader
	for (size_t i = 0; i < lights.size(); i++) {
		std::string index = std::to_string(i);

		_modelShader.setVec3("lights[" + index + "].position", lights[i]->position);

		_modelShader.setVec3(
			"lights[" + index + "].color",
			vec3<float>(
				lights[i]->lightColor.r,
				lights[i]->lightColor.g,
				lights[i]->lightColor.b
			)
		);

		_modelShader.setFloat("lights[" + index + "].emission", lights[i]->emissionStrength);
	}

	for (auto& obj : scene->getAllObjects().getAll()) {

		_modelShader.setMat4("model", obj->getModelMatrix());
		_modelShader.setMat4("view", camera->getViewMatrix());
		_modelShader.setMat4("projection", camera->getProjection());
		_modelShader.setVec3("viewPos", camera->getPosition());

		glBindVertexArray(_modelVAO);
		glBindBuffer(GL_ARRAY_BUFFER, _modelVBO);
		for (auto& triangle : obj->getRenderMesh().triangles) {
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CCW);

			if (obj->getRenderMesh().texture)
			{
				obj->getRenderMesh().texture->bind(0);
				_modelShader.setInt("uTexture", 0);
				_modelShader.setInt("useTexture", 1);
			}
			else {
				_modelShader.setInt("useTexture", 0);
			}

			_modelShader.setVec3("faceNormal", triangle.normal);

			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(triangle.vertices), triangle.vertices);

			if (CFG_WIREMESHMODE)
				glDrawArrays(GL_LINE_LOOP, 0, 3);
			else
				glDrawArrays(GL_TRIANGLES, 0, 3);

		}
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

void Renderer::drawUI(UI* ui)
{
	
	ASSERT(_uiShader.ID > 0, "Invalid uiShader");

	if (isCustomResolution())
		glBindFramebuffer(GL_FRAMEBUFFER, _resolutionFBO);
	else
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, _resolution.x, _resolution.y);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	_uiShader.use();
	for (auto& uiEl : ui->getRootElements()) {
		uiEl->draw({ 0,0 }, &_uiShader, _uiVBO, _uiVAO);
	}

	// Restore state after UI drawing:
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}
void Renderer::renderShadowPass(Scene* scene, LightSource* light, Camera* camera)
{
	(void)camera;

	const auto& objects = scene->getAllObjects().getAll();
	if (objects.empty()) {
		return;
	}

	vec3<float> sceneTarget = getShadowTarget(objects);
	vec3<float> lightForward = (sceneTarget - light->position).normalized();
	if (lightForward.lengthSquared() < 1e-6f) {
		lightForward = vec3<float>{ -0.35f, -1.0f, -0.25f }.normalized();
		sceneTarget = light->position + lightForward;
	}

	vec3<float> up{ 0.0f, 1.0f, 0.0f };
	if (std::fabs(lightForward.dot(up)) > 0.95f) {
		up = { 0.0f, 0.0f, 1.0f };
	}

	_shadowLightDirection = -lightForward;

	mat4 lightView = mat4::lookAt(
		light->position,
		sceneTarget,
		up
	);

	ShadowBounds bounds = getLightSpaceBounds(objects, lightView);
	if (!bounds.valid) {
		return;
	}

	const float width = bounds.max.x - bounds.min.x;
	const float height = bounds.max.y - bounds.min.y;
	const float depth = bounds.max.z - bounds.min.z;
	const float xyMargin = std::max(0.5f, std::max(width, height) * 0.05f);
	const float zMargin = std::max(1.0f, depth * 0.10f);

	float left = bounds.min.x - xyMargin;
	float right = bounds.max.x + xyMargin;
	float bottom = bounds.min.y - xyMargin;
	float top = bounds.max.y + xyMargin;
	float nearPlane = -bounds.max.z - zMargin;
	float farPlane = -bounds.min.z + zMargin;

	if (right - left < 1.0f) {
		const float center = (left + right) * 0.5f;
		left = center - 0.5f;
		right = center + 0.5f;
	}
	if (top - bottom < 1.0f) {
		const float center = (bottom + top) * 0.5f;
		bottom = center - 0.5f;
		top = center + 0.5f;
	}
	if (farPlane - nearPlane < 1.0f) {
		const float center = (nearPlane + farPlane) * 0.5f;
		nearPlane = center - 0.5f;
		farPlane = center + 0.5f;
	}

	mat4 lightProj = mat4::ortho(left, right, bottom, top, nearPlane, farPlane);

	_lightSpaceMatrix = lightProj * lightView;

	_shadowShader.use();
	_shadowShader.setMat4("lightSpaceMatrix", _lightSpaceMatrix);

	glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);
	glBindFramebuffer(GL_FRAMEBUFFER, _shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	glDisable(GL_CULL_FACE);

	for (auto& obj : scene->getAllObjects().getAll()) {

		_shadowShader.setMat4("model", obj->getModelMatrix());

		glBindVertexArray(_modelVAO);
		glBindBuffer(GL_ARRAY_BUFFER, _modelVBO);

		for (auto& tri : obj->getRenderMesh().triangles) {
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tri.vertices), tri.vertices);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	if (isCustomResolution())
		glBindFramebuffer(GL_FRAMEBUFFER, _resolutionFBO);
	else
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, _resolution.x, _resolution.y);
}

void Renderer::clear()
{
	if (isCustomResolution()) {
		glBindFramebuffer(GL_FRAMEBUFFER, _resolutionFBO);
		glViewport(0, 0, _resolution.x, _resolution.y);
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::present()
{
	if (!isCustomResolution()) return;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	const auto viewport = getPresentationViewport();
	glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	_resolutionShader.use();
	_resolutionShader.setInt("screenTexture", 0);
	glBindVertexArray(_resolutionVAO);
	glActiveTexture(GL_TEXTURE0);     
	glBindTexture(GL_TEXTURE_2D, _FBOTexture);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
}
