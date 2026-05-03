#include <iostream>

#include "core/Fragment.h"

Fragment engine;

Window* window;
Graphics* graphics;
UI* menuUI;
UI* gameUI;
Camera* camera;

LightSource* light1;

Input* userInput;
SceneObject* cube2;
int catapultSceneID;
int hingeSceneID;

float speed = 3.0f;

enum class DemoMode {
	Overview,
	Physics,
	Rendering
};

Button* button2;

DemoMode uiMode = DemoMode::Overview;
float exposureValue = 0.5f;
int sampleCount = 4;
std::vector<float> frameHistory(1000, 0.0f);
std::vector<float> arcValues{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

struct FramePerf {
	float inputUpdateMs = 0.0f;
	float userLogicUpdateMs = 0.0f;
	float uiUpdateMs = 0.0f;
	float physicsUpdateMs = 0.0f;
	float sceneDrawMs = 0.0f;
	float uiDrawMs = 0.0f;
	float presentMs = 0.0f;
	float engineOverheadMs = 0.0f;
};

FramePerf currentFramePerf;

void refreshArcChart(float frameTimeMs) {
	currentFramePerf.inputUpdateMs = engine.lastInputTimeMs;
	currentFramePerf.physicsUpdateMs = engine.lastPhysicsTimeMs;

	const float measured =
		currentFramePerf.inputUpdateMs +
		currentFramePerf.userLogicUpdateMs +
		currentFramePerf.uiUpdateMs +
		currentFramePerf.physicsUpdateMs +
		currentFramePerf.sceneDrawMs +
		currentFramePerf.uiDrawMs +
		currentFramePerf.presentMs;

	currentFramePerf.engineOverheadMs = std::max(0.0f, frameTimeMs - measured);

	arcValues[0] = currentFramePerf.inputUpdateMs;
	arcValues[1] = currentFramePerf.userLogicUpdateMs;
	arcValues[2] = currentFramePerf.uiUpdateMs;
	arcValues[3] = currentFramePerf.physicsUpdateMs;
	arcValues[4] = currentFramePerf.sceneDrawMs;
	arcValues[5] = currentFramePerf.uiDrawMs;
	arcValues[6] = currentFramePerf.presentMs;
	arcValues[7] = currentFramePerf.engineOverheadMs;

	frameHistory.erase(frameHistory.begin());
	frameHistory.push_back(std::max(0.0f, frameTimeMs));
}

void onWKeyHeld(float dt) {
	vec3<float> pos = camera->getPosition();
	vec3<float> f = camera->getForward();
	vec3<float> forwardXZ{ f.x, 0.0f, f.z };
	float len = sqrtf(forwardXZ.x * forwardXZ.x + forwardXZ.z * forwardXZ.z);
	if (len > 0.0001f) { forwardXZ.x /= len; forwardXZ.z /= len; }
	pos = pos + forwardXZ * speed * dt;
	camera->setPosition(pos);
}
void onSKeyHeld(float dt) {
	vec3<float> pos = camera->getPosition();
	vec3<float> f = camera->getForward();
	vec3<float> forwardXZ{ f.x, 0.0f, f.z };
	float len = sqrtf(forwardXZ.x * forwardXZ.x + forwardXZ.z * forwardXZ.z);
	if (len > 0.0001f) { forwardXZ.x /= len; forwardXZ.z /= len; }
	pos = pos - forwardXZ * speed * dt;
	camera->setPosition(pos);
}
void onAKeyHeld(float dt) {
	vec3<float> pos = camera->getPosition();
	vec3<float> r = camera->getRight();
	vec3<float> rightXZ{ r.x, 0.0f, r.z };
	float len = sqrtf(rightXZ.x * rightXZ.x + rightXZ.z * rightXZ.z);
	if (len > 0.0001f) { rightXZ.x /= len; rightXZ.z /= len; }
	pos = pos - rightXZ * speed * dt;
	camera->setPosition(pos);
}
void onDKeyHeld(float dt) {
	vec3<float> pos = camera->getPosition();
	vec3<float> r = camera->getRight();
	vec3<float> rightXZ{ r.x, 0.0f, r.z };
	float len = sqrtf(rightXZ.x * rightXZ.x + rightXZ.z * rightXZ.z);
	if (len > 0.0001f) { rightXZ.x /= len; rightXZ.z /= len; }
	pos = pos + rightXZ * speed * dt;
	camera->setPosition(pos);
}
void onShiftKeyHeld(float dt) {
	vec3<float> pos = camera->getPosition();
	pos.y -= speed * dt;
	camera->setPosition(pos);
}
void onSpaceKeyHeld(float dt) {
	vec3<float> pos = camera->getPosition();
	pos.y += speed * dt;
	camera->setPosition(pos);
}
void onControlKeyPressed() {
	speed = 6.f;
}
void onControlKeyReleased() {
	speed = 3.f;
}
void onEscapeKeyPressed() {
	if (engine.getGameState() == EGameState::UI) {
		engine.setGameState(EGameState::GAME);
		window->setCursorFocus(true);
	}
	else if (engine.getGameState() == EGameState::GAME) {
		engine.setGameState(EGameState::UI);
		window->setCursorFocus(false);
	}
}
void on1KeyPressed() {
	LOG("1 pressed, exiting");
	engine.end();
}

// Receives (dx, dy) deltas — fires only in GAME state (Input default).
void onMouseMove(float dx, float dy) {
	float sensitivity = 0.005f;

	vec3<float> rot = camera->getRotation();

	rot.y -= dx * sensitivity * -1;   // positive dx = mouse right = look left (negated above)
	rot.x -= dy * sensitivity * -1;

	const float maxPitch = 1.499f;
	if (rot.x > maxPitch) rot.x = maxPitch;
	if (rot.x < -maxPitch) rot.x = -maxPitch;

	camera->setRotation(rot);
}

void createUI() {
	//menuUI->setDebugDraw(true);

	Style defaultStyle;
	defaultStyle.setPrimaryColor(Color::fromHex("#1A1D22"));
	defaultStyle.setSecondaryColor(Color::fromHex("#E9EEF5"));
	defaultStyle.setTertiaryColor(Color::fromHex("#6D6E70"));
	defaultStyle.setBorder(true);
	defaultStyle.setBorderWidth(0.004f);
	defaultStyle.setBorderRadius(0.018f);
	defaultStyle.setBorderColor(Color::fromHex("#3A4555"));

	// Background panel
	Transform panelTr;
	panelTr.setPositionPx({ 230, 723 });
	panelTr.setSizePx({ 455, 134 });

	Rectangle* panel = menuUI->createRectangle(panelTr, defaultStyle);
	//panel->setDraggable(true);
	//panel->setResizable(true);
	
	//Style titleStyle = defaultStyle;
	//titleStyle.setPrimaryColor(Color::fromHex("#F7C873"));
	//Transform titleTr;
	//titleTr.setPositionPx({ 60, 135 });
	//panelTr.setSizePx({ 100, 50 });
	//Text* title1 = menuUI->createText("Example TextabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ß´+#-.,<^°!§$%&/()=?`*'_:;~{}³²²@<>|€][\n", titleTr, titleStyle, 24);

	// Reset / Switch buttons
	Style buttonStyle = defaultStyle;
	buttonStyle.setPrimaryColor(Color::fromHex("#273241"));
	buttonStyle.setSecondaryColor(Color::fromHex("#F4F7FB"));

	Transform buttonResetTr;
	buttonResetTr.setPositionPx({ 66, 696 });
	buttonResetTr.setSizePx({ 100, 50 });
	Button* buttonReset = menuUI->createButton("Reset", []() {
		engine.loadSceneByID(engine.getLoadedSceneID());
	}, buttonResetTr, buttonStyle, 18);
	//buttonReset->setDraggable(true);
	//buttonReset->setResizable(true);

	Transform buttonSwitchTr = buttonResetTr;
	buttonSwitchTr.setPositionPx({ 180, 696 });
	button2 = menuUI->createButton("Switch scene", []() {
		if (engine.getLoadedSceneID() == catapultSceneID) {
			engine.loadSceneByID(hingeSceneID);
		}
		else engine.loadSceneByID(catapultSceneID);
	}, buttonSwitchTr, buttonStyle, 18);
	//button2->setDraggable(true);
	//button2->setResizable(true);
	
	//Transform inputTr;
	//inputTr.setPositionPx({ 60, 335 });
	//inputTr.setSizePx({ 100, 50 });
	//menuUI->createInputField<int>(&sampleCount, inputTr, buttonStyle, 18);
	
	Transform sliderTr;
	sliderTr.setPositionPx({ 171, 758 });
	sliderTr.setSizePx({ 313, 32 });
	Slider<float>* slider = menuUI->createSlider<float>(&engine.getPhysicsTimeScale(), 0.0f, 1.0f, 0.00f, sliderTr, buttonStyle, 14);

	//slider->setDraggable(true);
	//slider->setResizable(true);

	Transform dropdownTr;
	dropdownTr.setPositionPx({ 390, 750 });
	dropdownTr.setSizePx({ 100, 50 });
	Dropdown<EPhysicsUpdateState>* dropdown = menuUI->createDropdown<EPhysicsUpdateState>(
		&engine.getPhysicsUpdateState(),
		{
			{ EPhysicsUpdateState::CAPPED, "Capped" }, 
			{ EPhysicsUpdateState::SLOWED, "Slowed" },
			{ EPhysicsUpdateState::STOPPED, "Stopped" }
		},
		dropdownTr,
		buttonStyle,
		16
	);

	//dropdown->setDraggable(true);
	//dropdown->setResizable(true);

	Transform histogramTr;
	histogramTr.setPositionUS({ 0.60f, 0.90f });
	histogramTr.setSizePx({ 400, 100 });
	gameUI->createHistogram(&frameHistory, histogramTr, defaultStyle);

	Transform chartTr;
	chartTr.setPositionUS({ 0.90f, 0.90f });
	chartTr.setSizePx({ 100, 100 });
	gameUI->createArcChart(
		&arcValues,
		{
			Color::fromHex("#F7C873"),
			Color::fromHex("#7AC7FF"),
			Color::fromHex("#7BE495"),
			Color::fromHex("#A0A9B8"),
			Color::fromHex("#F08A5D"),
			Color::fromHex("#4ECDC4"),
			Color::fromHex("#C7F464"),
			Color::fromHex("#FF6B6B")
		},
		chartTr,
		defaultStyle,
		{
			"Input",
			"Logic",
			"UI update",
			"Physics",
			"3D draw",
			"UI draw",
			"Present",
			"Overhead"
		}
	);
}

void createScenes() {

	// Catapult + single object axis Constraints

	PhysicsSettings cfg;
	cfg.restitution = 0.f;

	SceneObject* stand = engine.createNewSceneObject();
	stand->getRenderMesh().createCube();
	stand->getRenderMesh().setColor(Color::Red);
	stand->setPosition(vec3<float>(0, 1, 0));
	stand->setScale(vec3<float>(1, 0.25f, 0.25f));
	stand->initPhysics(cfg);

	AxisConstraint standConstraint = AxisConstraint(GlobalSceneObjectKeyRegister::getKeyByObj(stand));
	standConstraint.lockAllRotation();
	standConstraint.lockMovementX();
	standConstraint.lockMovementZ();

	engine.addAxisConstraint(standConstraint);

	SceneObject* plate = engine.createNewSceneObject();
	plate->getRenderMesh().createCube();
	plate->getRenderMesh().setColor(Color::Green);
	plate->setPosition(vec3<float>(0, 2, 2));
	plate->setScale(vec3<float>(1, 0.25f, 7.f));
	plate->initPhysics(cfg);

	AxisConstraint plateConstraint = AxisConstraint(GlobalSceneObjectKeyRegister::getKeyByObj(plate));
	plateConstraint.lockRotationY();
	plateConstraint.lockRotationZ();
	plateConstraint.lockMovementX();
	plateConstraint.lockMovementZ();

	engine.addAxisConstraint(plateConstraint);

	SceneObject* payload = engine.createNewSceneObject();
	payload->getRenderMesh().createCube();
	payload->getRenderMesh().setColor(Color::Blue);
	payload->setPosition(vec3<float>(0, 3, 5));
	payload->setScale(vec3<float>(0.5f, 0.5f, 0.5f));
	payload->initPhysics(cfg);

	AxisConstraint payloadConstraint = AxisConstraint(GlobalSceneObjectKeyRegister::getKeyByObj(payload));
	payloadConstraint.lockAllRotation();
	payloadConstraint.lockMovementX();
	payloadConstraint.lockMovementZ();

	engine.addAxisConstraint(payloadConstraint);

	cfg.mass = 40;

	SceneObject* weight = engine.createNewSceneObject();
	weight->getRenderMesh().createCube();
	weight->getRenderMesh().setColor(Color::Red);
	weight->setPosition(vec3<float>(0, 14, -1));
	weight->initPhysics(cfg);

	AxisConstraint weightConstraint = AxisConstraint(GlobalSceneObjectKeyRegister::getKeyByObj(weight));
	weightConstraint.lockAllRotation();
	weightConstraint.lockMovementX();
	weightConstraint.lockMovementZ();
	engine.addAxisConstraint(weightConstraint);

	cfg.isStatic = true;

	SceneObject* floorPlate = engine.createNewSceneObject();
	floorPlate->getRenderMesh().createCube();
	floorPlate->getRenderMesh().setColor(Color::White);
	floorPlate->setScale(vec3<float>(105, 1, 105));
	floorPlate->initPhysics(cfg);

	Scene* catapultScene = engine.saveToNewScene("catapultScene");
	catapultSceneID = catapultScene->getID();

	// Cubes with hinges
	cfg = PhysicsSettings();
	cfg.restitution = 0.f;

	cfg.isStatic = true;

	SceneObject* cube1 = engine.createNewSceneObject();
	cube1->getRenderMesh().createCube();
	cube1->getRenderMesh().setColor(Color::Green);
	cube1->setPosition(vec3<float>(0, 1, 0));
	cube1->setScale(vec3<float>(1, 1, 1));
	cube1->initPhysics(cfg);

	cfg.isStatic = false;

	SceneObject* cube2 = engine.createNewSceneObject();
	cube2->getRenderMesh().createCube();
	cube2->getRenderMesh().setColor(Color::Blue);
	cube2->setPosition(vec3<float>(0, 1, 1));
	cube2->setScale(vec3<float>(0.5f, 1, 0.5f));
	cube2->initPhysics(cfg);

	HingeConstraint hinge(GlobalSceneObjectKeyRegister::getKeyByObj(cube1), GlobalSceneObjectKeyRegister::getKeyByObj(cube2));
	hinge.setPoint1({ 0, 0, 0.5f });  
	hinge.setPoint2({ 0, 0, -1.5f }); 

	hinge.setHingeAxis1({ 1, 0, 0 });
	hinge.setHingeAxis2({ 1, 0, 0 });

	hinge.setNormalAxis1({ 1, 0, 0 });
	hinge.setNormalAxis2({ 1, 0, 0 });

	//hinge.setAngleLimits(0, 1.57f);

	hinge.enableMotor(true);
	hinge.setMotorTargetVelocity(2.0f);
	hinge.setMotorMaxTorque(100.0f);

	engine.addHingeConstraint(hinge);

	Scene* hingeScene = engine.saveToNewScene("HingeScene");
	hingeSceneID = hingeScene->getID();
}

void createInput() {
	userInput->setKeyHeld(GLFW_KEY_W, onWKeyHeld);
	userInput->setKeyHeld(GLFW_KEY_A, onAKeyHeld);
	userInput->setKeyHeld(GLFW_KEY_S, onSKeyHeld);
	userInput->setKeyHeld(GLFW_KEY_D, onDKeyHeld);
	userInput->setKeyHeld(GLFW_KEY_LEFT_SHIFT, onShiftKeyHeld);
	userInput->setKeyHeld(GLFW_KEY_SPACE, onSpaceKeyHeld);

	userInput->setKeyPressed(GLFW_KEY_1, on1KeyPressed);
	userInput->setKeyPressed(GLFW_KEY_ESCAPE, onEscapeKeyPressed);

	userInput->setKeyPressed(GLFW_KEY_LEFT_CONTROL, onControlKeyPressed);
	userInput->setKeyReleased(GLFW_KEY_LEFT_CONTROL, onControlKeyReleased);

	userInput->setMouseMoveAction(onMouseMove);
}

void end() {
	LOG("End Called");
	engine.loadedScene.deactivatePhysics();
	engine.loadedScene = Scene();
	engine.scenes.reset();
	exit(0);
}

void start() {
	std::cout << "Program Started" << std::endl;

	//engine.setDebugFPS(true);

	// Create Graphics
	graphics = engine.getGraphics();
	window = graphics->getWindow();
	window->setSize({ 1200,800 });
	//window->setFullscreen();
	window->setVSync(false);
	Renderer::get()->setCustomResolution(vec2<int>(window->getWidth(), window->getHeight()));

	//LOG("Aspect ratio:" + std::to_string(graphics->getAspect()));
	
	// Create a Camera
	camera = graphics->initNewCamera("MainCamera", window->getAspect());
	camera->setPosition(vec3<float>(4, 1, 0));
	camera->setRotation(vec3<float>(0, -1.6, 0));


	// Add a lightsource
	light1 = graphics->initNewLight("Main light");
	light1->position = vec3<float>(3, 2, 0);
	light1->lightColor = Color(1, 0.9f, 0.9f, 1);
	light1->emissionStrength = 1.f;

	// Create the UI
	menuUI = engine.createNewUI();
	gameUI = engine.createNewUI();
	createUI();

	// Init Keyboard IO
	userInput = engine.initUserInput();
	// Bind functions to keypresses
	createInput();

	engine.setGameState(EGameState::UI);
	window->setCursorFocus(false);

	createScenes();
	engine.loadSceneByID(catapultSceneID);
	LOG(CFG_GET_WINDOW_ASPECT());
}


SceneObject* pickSceneObject(float mouseX, float mouseY)
{
	Ray ray = screenToRay(
		mouseX, mouseY,
		CFG_WINDOW_WIDTH, CFG_WINDOW_HEIGHT,
		camera->getViewMatrix(),
		camera->getProjection()
	);

	SceneObject* closest = nullptr;
	float closestT = std::numeric_limits<float>::max();

	for (auto& obj : engine.loadedScene.getAllObjects().getAll()) {
		auto [aabbMin, aabbMax] = getWorldAABB(obj.get());
		float t;
		if (rayIntersectsAABB(ray, aabbMin, aabbMax, t) && t < closestT) {
			closestT = t;
			closest = obj.get();
		}
	}
	return closest;  // nullptr if nothing hit
}


void update(float dt) {
	auto logicStart = std::chrono::high_resolution_clock::now();

	for (auto& o : engine.loadedScene.getAllObjects().getAll())
		o->syncFromPhysics();

	double mx, my;
	glfwGetCursorPos(window->getRawWindow(), &mx, &my);

	bool lmbDown = glfwGetMouseButton(window->getRawWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

	const auto uiMouse = Renderer::get()->mapWindowToRenderCoordinates(
		static_cast<float>(mx),
		static_cast<float>(my)
	);
	float mouseX = uiMouse.x;
	float mouseY = uiMouse.y;

	auto logicEnd = std::chrono::high_resolution_clock::now();
	currentFramePerf.userLogicUpdateMs = std::chrono::duration<float, std::milli>(logicEnd - logicStart).count();

	auto uiUpdateStart = std::chrono::high_resolution_clock::now();

	if (engine.getGameState() == EGameState::UI)
		menuUI->update(mouseX, mouseY, lmbDown);
	else
		gameUI->update(mouseX, mouseY, lmbDown);

	auto uiUpdateEnd = std::chrono::high_resolution_clock::now();
	currentFramePerf.uiUpdateMs = std::chrono::duration<float, std::milli>(uiUpdateEnd - uiUpdateStart).count();

	static int counter = 0;
	//if (engine.cursorInUI(menuUI))
	//	LOG("hit " << counter++);

	if (engine.getGameState() == EGameState::UI) {
		if (!engine.cursorInUI(menuUI)) {
			SceneObject* obj = pickSceneObject(mx,my);
			if (obj) LOG("hit over " << obj->getRenderMesh().triangles[0].vertices[0].color.toHex());
		}
	}
}

void draw() {
	auto sceneStart = std::chrono::high_resolution_clock::now();
	Renderer::get()->clear();

	Renderer::get()->drawScene(&engine.loadedScene, graphics->getCameras().getByName("MainCamera"), graphics->getLights().getAll());
	auto sceneEnd = std::chrono::high_resolution_clock::now();
	currentFramePerf.sceneDrawMs = std::chrono::duration<float, std::milli>(sceneEnd - sceneStart).count();

	auto uiDrawStart = std::chrono::high_resolution_clock::now();

	if (engine.getGameState() == EGameState::UI)
		Renderer::get()->drawUI(menuUI);
	else
		Renderer::get()->drawUI(gameUI);

	auto uiDrawEnd = std::chrono::high_resolution_clock::now();
	currentFramePerf.uiDrawMs = std::chrono::duration<float, std::milli>(uiDrawEnd - uiDrawStart).count();

	auto presentStart = std::chrono::high_resolution_clock::now();

	Renderer::get()->present();
	glfwSwapBuffers(window->getRawWindow());
	glfwPollEvents();

	auto presentEnd = std::chrono::high_resolution_clock::now();
	currentFramePerf.presentMs = std::chrono::duration<float, std::milli>(presentEnd - presentStart).count();
}

int main() {
	
	engine.setAppStartFn(start);
	engine.setAppDrawUpdateFn(draw);
	engine.setAppLogicUpdateFn(update);
	engine.setAppFrameEndFn(refreshArcChart);
	engine.setEndFn(end);

	engine.startApp();
	engine.runApp();


	return 0;
}

