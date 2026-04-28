#include "Renderer.h"

void Renderer::initialize(Window* window)
{
	// Check if window is valid
	ASSERT(window, "Window not vaild at Renderer initialization");
	_window = window;
	_resolution = { window->getWidth(), window->getHeight() };

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

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 3, nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoordinate));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
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

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
}

void Renderer::setCustomResolution(vec2<int> size)
{
	_resolution = size;
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

void Renderer::drawScene(Scene* scene, Camera* camera, std::vector<std::unique_ptr<LightSource>>& lights)
{
	ASSERT(scene, "Tried to draw invalid scene");
	ASSERT(camera, "Tried to draw with invalid camera");

	if (!lights.empty()) {
		renderShadowPass(scene, lights[0].get());
	}
	if (isCustomResolution())
		glBindFramebuffer(GL_FRAMEBUFFER, _resolutionFBO);
	glViewport(0, 0, _resolution.x, _resolution.y);

	_modelShader.use();
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _shadowMap);

	_modelShader.setInt("shadowMap", 1);
	_modelShader.setMat4("lightSpaceMatrix", _lightSpaceMatrix);
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
		drawUIObject(uiEl, { 0,0 });
	}

	// Restore state after UI drawing:
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void Renderer::drawUIObject(UIElement* uiElement, vec2<float> parentPos) {
	vec2<float> worldPos = parentPos + uiElement->getTransform().getPosition();


	//Draw
	for (auto& t : uiElement->getMesh2D().getTriangles())
	{
		_uiShader.use();

		if (uiElement->getMesh2D().getTexture())
		{
			uiElement->getMesh2D().getTexture()->bind(0);

			_uiShader.setInt("uTexture", 0);
			_uiShader.setInt("useTexture", 1);
		}
		else
		{
			_uiShader.setInt("useTexture", 0);
		}

		Vertex2D verts[3];
		for (int i = 0; i < 3; ++i)
		{
			verts[i] = t.vertices[i];
			verts[i].position.x += parentPos.x;
			verts[i].position.y += parentPos.y;
		}

		glBindVertexArray(_uiVAO);
		glBindBuffer(GL_ARRAY_BUFFER, _uiVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);
	}
	for (auto& child : uiElement->getChildren()) {
		drawUIObject(child, worldPos);
	}
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::renderShadowPass(Scene* scene, LightSource* light)
{
	float near = 1.0f, far = 100.0f;

	mat4 lightProj = mat4::ortho(-20.f, 20.f, -20.f, 20.f, near, far);
	mat4 lightView = mat4::lookAt(light->position,
		vec3<float>(0, 0, 0),
		vec3<float>(0, 1, 0));

	_lightSpaceMatrix = lightProj * lightView;

	_shadowShader.use();
	_shadowShader.setMat4("lightSpaceMatrix", _lightSpaceMatrix);

	glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);
	glBindFramebuffer(GL_FRAMEBUFFER, _shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	glCullFace(GL_FRONT);

	for (auto& obj : scene->getAllObjects().getAll()) {

		_shadowShader.setMat4("model", obj->getModelMatrix());

		glBindVertexArray(_modelVAO);
		glBindBuffer(GL_ARRAY_BUFFER, _modelVBO);

		for (auto& tri : obj->getRenderMesh().triangles) {
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tri.vertices), tri.vertices);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}

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
	glViewport(0, 0, _window->getWidth(), _window->getHeight());
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	_resolutionShader.use();
	glBindVertexArray(_resolutionVAO);
	glBindTexture(GL_TEXTURE_2D, _FBOTexture);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
}
