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

DemoMode uiMode = DemoMode::Overview;
float exposureValue = 0.5f;
int sampleCount = 4;
std::vector<float> frameHistory(24, 0.0f);
std::vector<float> arcValues{ 16.0f, 8.0f, 4.0f };

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
	menuUI->setDebugDraw(true);

	Style panelStyle;
	panelStyle.setPrimaryColor(Color::fromHex("#1A1D22"));
	panelStyle.setSecondaryColor(Color::fromHex("#E9EEF5"));
	panelStyle.setBorder(true);
	panelStyle.setBorderWidth(0.004f);
	panelStyle.setBorderRadius(0.018f);
	panelStyle.setBorderColor(Color::fromHex("#3A4555"));

	Transform panelTr;
	panelTr.setPositionPx({ 60, 35 });
	panelTr.setSizePx({ 100, 50 });

	Rectangle* panel = menuUI->createRectangle(panelTr, panelStyle);
	panel->setDraggable(true);
	panel->setResizable(true);
	
	Style titleStyle = panelStyle;
	titleStyle.setPrimaryColor(Color::fromHex("#F7C873"));
	Transform titleTr;
	titleTr.setPositionPx({ 60, 135 });
	panelTr.setSizePx({ 100, 50 });
	Text* title = menuUI->createText("Example Text", titleTr, titleStyle, 24);
	title->setColor(titleStyle.getPrimaryColor());
	title->setDraggable(true);
	title->setResizable(true);
	
	Style buttonStyle = panelStyle;
	buttonStyle.setPrimaryColor(Color::fromHex("#273241"));
	buttonStyle.setSecondaryColor(Color::fromHex("#F4F7FB"));
	Transform buttonTr;
	buttonTr.setPositionPx({ 60, 235 });
	buttonTr.setSizePx({ 100, 50 });
	Button* button = menuUI->createButton("Push", []() {
		LOG("Button pressed");
		arcValues[0] += 1.0f;
		arcValues[1] = std::max(1.0f, arcValues[1] - 0.5f);
	}, buttonTr, buttonStyle, 18);
	button->setDraggable(true);
	button->setResizable(true);
	
	Transform inputTr;
	inputTr.setPositionPx({ 60, 335 });
	inputTr.setSizePx({ 100, 50 });
	menuUI->createInputField<int>(&sampleCount, inputTr, buttonStyle, 18);
	
	Transform sliderTr;
	sliderTr.setPositionPx({ 140, 435 });
	sliderTr.setSizePx({ 250, 50 });
	menuUI->createSlider<float>(&exposureValue, 0.0f, 1.0f, 0.05f, sliderTr, buttonStyle, 14);
	
	//!!!
	Transform dropdownTr;
	dropdownTr.setPositionPx({ 60, 535 });
	dropdownTr.setSizePx({ 100, 50 });
	menuUI->createDropdown<DemoMode>(
		&uiMode,
		{
			{ DemoMode::Overview, "Overview" }, 
			{ DemoMode::Physics, "Physics" },
			{ DemoMode::Rendering, "Rendering" }
		},
		dropdownTr,
		buttonStyle,
		16
	);
	Transform histogramTr;
	histogramTr.setPositionUS({ 0.78f, 0.26f });
	histogramTr.setSizeUS({ 0.16f, 0.14f });
	menuUI->createHistogram(&frameHistory, histogramTr, panelStyle);

	Transform chartTr;
	chartTr.setPositionUS({ 0.78f, 0.78f });
	chartTr.setSizeUS({ 0.11f, 0.11f });
	menuUI->createArcChart(
		&arcValues,
		{
			Color::fromHex("#F7C873"),
			Color::fromHex("#7AC7FF"),
			Color::fromHex("#7BE495")
		},
		chartTr,
		panelStyle
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

	engine.setDebugFPS(true);

	// Create Graphics
	graphics = engine.getGraphics();
	window = graphics->getWindow();
	window->setFullscreen();
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

void update(float dt) {

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

	frameHistory.erase(frameHistory.begin());
	frameHistory.push_back(std::max(0.0f, engine.avgFrameTime));

	arcValues[0] = engine.avgRenderTime;
	arcValues[1] = engine.avgPhysicsTime;
	arcValues[2] = engine.avgOtherTime;

	if (engine.getGameState() == EGameState::UI)
		menuUI->update(mouseX, mouseY, lmbDown);
	else
		gameUI->update(mouseX, mouseY, lmbDown);
}

void draw() {
	Renderer::get()->clear();

	Renderer::get()->drawScene(&engine.loadedScene, graphics->getCameras().getByName("MainCamera"), graphics->getLights().getAll());

	if (engine.getGameState() == EGameState::UI)
		Renderer::get()->drawUI(menuUI);
	else
		Renderer::get()->drawUI(gameUI);

	Renderer::get()->present();
	glfwSwapBuffers(window->getRawWindow());
	glfwPollEvents();
}

int main() {
	
	engine.setAppStartFn(start);
	engine.setAppDrawUpdateFn(draw);
	engine.setAppLogicUpdateFn(update);
	engine.setEndFn(end);

	engine.startApp();
	engine.runApp();


	return 0;
}

