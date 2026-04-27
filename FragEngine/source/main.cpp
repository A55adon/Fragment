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

//REFLECT_EXISTING_ENUM(EPhysicsUpdateState, STOPPED, CAPPED, SLOWED);

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
	// Exit button (top-left)
	//UIObject* exitButton = menuUI->registerNewUIObject();
	//exitButton->setPosition({ 3.f, 2.f });
	//exitButton->createRoundedRect({ 4.f, 2.5f }, 1.f, 10, Color(0.89f, 0.025f, 0.06f, 1));
	//exitButton->setOnClick([]() { exit(EXIT_SUCCESS); });
	//
	//UIObject* exitLabel = exitButton->registerNewChildUIObject();
	//exitLabel->setPosition({ -1.5f, 0.6f });
	//exitLabel->createText("res/Roboto-Black.ttf", "Exit", 22, Color::White);
	//
	//UIObject* loadScene1 = menuUI->registerNewUIObject();
	//loadScene1->setPosition({ 3.f, 20.f });
	//loadScene1->createRoundedRect({ 4.f, 2.5f }, 1.f, 10, Color::Blue);
	//loadScene1->setOnClick([]() { 
	//	engine.loadSceneByID(engine.scenes.getByID(scene1ID)->getID());
	//});
	//UIObject* loadScene2 = menuUI->registerNewUIObject();
	//loadScene2->setPosition({ 3.f, 25.f });
	//loadScene2->createRoundedRect({ 4.f, 2.5f }, 1.f, 10, Color::Red);
	//loadScene2->setOnClick([]() {
	//	engine.loadSceneByID(engine.scenes.getByID(scene2ID)->getID());
	//});
	//
	//UIObject* resetScene1 = menuUI->registerNewUIObject();
	//resetScene1->setPosition({ 20.f, 20.f });
	//resetScene1->createRoundedRect({ 4.f, 2.5f }, 1.f, 10, Color::Blue);
	//resetScene1->setOnClick([]() {
	//	engine.restoreSceneDefault(scene1ID);
	//});
	//UIObject* resetScene2 = menuUI->registerNewUIObject();
	//resetScene2->setPosition({ 20.f, 25.f });
	//resetScene2->createRoundedRect({ 4.f, 2.5f }, 1.f, 10, Color::Red);
	//resetScene2->setOnClick([]() {
	//	engine.restoreSceneDefault(scene2ID);
	//});


	//UIStyle style;

	//UITransform tr1;
	//tr1.position = { 10, 10 };
	//tr1.size = { 10, 10 };
	//UITransform tr2;
	//tr2.position = { 30, 30 };
	//tr2.size = { 20, 6 };
	//UITransform tr3;
	//tr3.position = { 30, 60 };
	//tr3.size = { 80, 10 };
	//UITransform tr4;
	//tr4.position = { 5,5 };

	//Rect* obj1 = menuUI->registerNew<Rect>("Test1");
	//obj1->create(tr1,style);
	//obj1->isDraggable = true;
	//
	//style.borderWidth = .3f;
	//style.cornerRoundness = 1.5f;
	//
	//Rect* obj2 = menuUI->registerNew<Rect>("Test2");
	//obj2->create(tr2, style);
	//obj2->isResizable = true;
	//
	//style.borderWidth = .4f;
	//style.cornerRoundness = 2.5f;
	//
	//Rect* obj3 = menuUI->registerNew<Rect>("Test3");
	//obj3->create(tr3, style);
	//obj3->isDraggable = true;
	//obj3->isResizable = true;

	//Rect* rectObj = menuUI->registerNew<Rect>("recttest");
	//rectObj->create(tr1, UIStyle());


	//
	//smallFont->setColor(Color::Black);
	//mediumFont->setColor(Color::White);
	//largeFont->setColor(Color::Blue);
	//
	//auto createText = [&](std::shared_ptr<Font> font, const std::string& text, float x, float y) -> Text* {
	//	Text* txt = menuUI->registerNew<Text>("Text");
	//	txt->setFont(font);						
	//	txt->setPositionPercent(x, y);  		
	//	txt->setText(text);						
	//	return txt;
	//	};

	//createText(smallFont, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", 5.0f, 15.0f);
	//
	//createText(mediumFont, "0123456789!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~", 5.0f, 40.0f);
	//
	//createText(largeFont, "Large Text Example", 5.0f, 78.0f);
	//
	//auto titleFont = std::make_shared<Font>("res/Roboto-Black.ttf", 36);
	//titleFont->setColor(Color{ 255, 100, 50, 255 }); // Orange
	//
	//createText(titleFont,
	//	"My Game Title",
	//	100.0f, 40.0f);

	//auto smallFont = std::make_shared<Font>("res/Roboto-Black.ttf", 12);
	//auto mediumFont = std::make_shared<Font>("res/Roboto-Black.ttf", 24);
	//auto largeFont = std::make_shared<Font>("res/Roboto-Black.ttf", 48);

	/*Button* btn = menuUI->registerNew<Button>("testbtn");
	btn->create("Reset", mediumFont, [] { engine.restoreSceneDefault(engine.scenes.getByName("scene1")->getID()); }, UIStyle(), tr2);
	btn->isDraggable = true;	btn->isResizable = true;*/

	//auto getCorrectPos = [](UITransform trIn, float padding = 1.f) {
	//	UITransform tr = trIn;
	//	float aspect = graphics->getWindow()->getAspect();
	//	float halfW = trIn.size.x / (2.f * aspect); // actual NDC-corrected half-width
	//	float halfH = trIn.size.y / 2.f;
	//
	//	if (trIn.position.x - halfW < 0)
	//		tr.position.x = halfW + padding / aspect;
	//
	//	if (trIn.position.y - halfH < 0)
	//		tr.position.y = halfH + padding;
	//
	//	if (trIn.position.x + halfW > 100)
	//		tr.position.x = 100.f - (halfW + padding / aspect);
	//
	//	if (trIn.position.y + halfH > 100)
	//		tr.position.y = 100.f - (halfH + padding);
	//
	//	return tr;
	//};
	//
	//auto getCorrectSize = [](UITransform trIn, UIStyle style, Text* text, float padding = 1.f) {
	//	UITransform tr = trIn;
	//	float aspect = graphics->getWindow()->getAspect();
	//
	//	vec2<float> worldPos = trIn.position + text->getTransform().position;
	//	auto textBounds = text->getBoundsPixels(worldPos);
	//
	//	vec2<float> textSizePx = { textBounds.y - textBounds.x, textBounds.w - textBounds.z };
	//
	//	float winW = (float)graphics->getWindow()->getWidth();
	//	float winH = (float)graphics->getWindow()->getHeight();
	//
	//	// X needs aspect correction because text mesh verts are already aspect-scaled
	//	vec2<float> textSizeUI = {
	//		textSizePx.x / winW * 100.f * aspect,
	//		textSizePx.y / winH * 100.f
	//	};
	//
	//	tr.size.x = ((textSizeUI.x + 2.f * padding) + style.borderWidth * 2);
	//	tr.size.y = ((textSizeUI.y + 2.f * padding) + style.borderWidth * 2);
	//
	//	return tr;
	//};
	//
	//auto autoCorrect = [getCorrectPos, getCorrectSize](UIObject* obj, Text* txt, float padding = 1.f) {
	//	UITransform tr = obj->getTransform();
	//	tr = getCorrectSize(tr, obj->getStyle(), txt, padding);
	//	tr = getCorrectPos(tr, padding);
	//	return tr;
	//};
	//
	//float padding = 2.f;
	//
	//Dropdown<EPhysicsUpdateState>* drdwn = menuUI->registerNew<Dropdown<EPhysicsUpdateState>>("dropdown");
	//UITransform tr = UITransform{ { 0,0 },{ 0,0 } };
	//drdwn->create(engine.getPhysicsUpdateState(), tr, UIStyle(), mediumFont);
	//UITransform trC = autoCorrect(drdwn, drdwn->getHeaderButton()->getText(), padding);
	//drdwn->create(engine.getPhysicsUpdateState(), trC, UIStyle(), mediumFont);
	////LOG(trC.position.x);
	////LOG(trC.position.y);
	//
	////LOG(trC.size.x);
	////LOG(trC.size.y);
	//Button* btn = menuUI->registerNew<Button>("Btn");
	//btn->create("Reset", mediumFont);
	//UITransform trBtnC = autoCorrect(btn, btn->getText(), padding);
	//
	//float aspect = graphics->getWindow()->getAspect();
	//float dropRightEdge = drdwn->getTransform().position.x + drdwn->getTransform().size.x / (2.f * aspect);
	//float btnHalfW = trBtnC.size.x / (2.f * aspect);
	//
	//UITransform trBtn = trBtnC;
	//trBtn.position.x = dropRightEdge + padding / aspect + btnHalfW;
	//trBtn.position.y = drdwn->getTransform().position.y; // align vertically with dropdown
	//trBtn.size.y = drdwn->getTransform().size.y;
	//btn->create("Reset", mediumFont, []() { engine.loadSceneByID(engine.getLoadedSceneID()); }, UIStyle(), trBtn);
	//
	//float value = 0.5f;
	//
	//Slider<float>* sl = menuUI->registerNew<Slider<float>>("Slider");
	//sl->create(engine.getPhysicsTimeScale(), {0.f, 5.f}, UITransform{{50, 5}, {30, 4}});
	////sl->onValueChange = [](float v) { engine.setPhysicsTimeScale(v); };
	//
	//Button* btnTS = menuUI->registerNew<Button>("Btn");
	//btnTS->create("ToggleScene", mediumFont, []() {
	//	if (engine.getLoadedSceneID() == catapultSceneID)
	//		engine.loadSceneByID(hingeSceneID);
	//	else
	//		engine.loadSceneByID(catapultSceneID);
	//	}, UIStyle(), UITransform{ {7.f,20.f}, { 18.f,8.f } });
	//
	//
	//auto makeStat = [&](const std::string& label, float y,
	//	std::function<float()> getter)
	//	{
	//		Text* txt = gameUI->registerNew<Text>("stat_" + label);
	//		txt->setFont(smallFont);
	//		txt->setParentUI(menuUI);
	//		txt->setPositionPercent(92.f, y);
	//		txt->setText(label + ": 0");
	//
	//		static std::vector<std::unique_ptr<VarCapture<float>>> caps;
	//
	//		auto cap = std::make_unique<VarCapture<float>>(getter, std::chrono::milliseconds(200), false);
	//
	//		cap->onEntry = [txt, label](const float& v)
	//			{
	//				txt->setText(label + ": " + std::format("{:.2f}", v));
	//			};
	//
	//		caps.emplace_back(std::move(cap));
	//	};
	//
	//makeStat("FPS",         2.f, []() { return engine.avgFPS;         });
	//makeStat("Frame(ms)",   4.f, []() { return engine.avgFrameTime;   });
	//makeStat("Physics(ms)", 6.f, []() { return engine.avgPhysicsTime; });
	//makeStat("Render(ms)",  8.f, []() { return engine.avgRenderTime;  });
	//makeStat("Other(ms)",   10.f,[]() { return engine.avgOtherTime;  });
	//
	//Slider<float>* sl2 = menuUI->registerNew<Slider<float>>("Slider2");
	//sl2->create(bufferedValue, { 0.f, 100.f }, UITransform{ {50, 15}, {30, 4} });
	//sl2->onValueChange = [](float v) { buff << v << "\n"; };
	//buff.setOnEntry([](const std::string chunk) {
	//	std::cout << "[captured]" << chunk << std::endl;
	//});

	//Button* btn3= menuUI->registerNew<Button>("btn");
	//btn3->create("Dump", mediumFont, []() { buff.dumpToFile("dump.txt"); }, UIStyle(), UITransform{ {50, 20}, { 10,5 } });


	//Slider<float>* sl2 = menuUI->registerNew<Slider<float>>("Slider2");
	//sl2->create(bufferedValue, { 0.f, 100.f }, UITransform{ {50, 15}, {30, 4} });
	////sl2->onValueChange = [](float v) { cap.push(v); };
	////buff.setOnEntry([](const std::string chunk) {
	////	std::cout << "[captured]" << chunk << std::endl;
	////});

	//Button* btn3 = menuUI->registerNew<Button>("btn");
	//btn3->create("Dump", mediumFont, []() {  }, UIStyle(), UITransform{ {50, 20}, {10,5} });
	//cap->onEntry = [](test v) {
	//	std::cout << ".";
	//};


	//vec2<float> worldPos = drdwn->getTransform().position
	//	+ drdwn->getHeaderButton()->getTransform().position
	//	+ drdwn->getHeaderButton()->getText()->getTransform().position;

	//auto bounds = drdwn->getHeaderButton()->getText()->getBoundsPixels(worldPos);
	//LOG("Bounds:" << bounds);


	//Button* btn = menuUI->registerNew<Button>("btn");
	//btn->create("Press", mediumFont, []() { LOG("test"); }, UIStyle(), UITransform{ {50,50},{0,0} });
	//btn->create("Press", mediumFont, []() { LOG("test2"); }, UIStyle(), getCorrectSize(btn->getTransform(), UIStyle(), btn->getText()));

	//Button* btn2 = menuUI->registerNew<Button>("btn2");
	//btn2->create("PressPress", mediumFont, []() { LOG("test"); }, UIStyle(), UITransform{ {50,60},{0,0} });
	//btn2->create("PressPress", mediumFont, []() { LOG("test2"); }, UIStyle(), getCorrectSize(btn2->getTransform(), UIStyle(), btn2->getText()));

	//Button* btn3 = menuUI->registerNew<Button>("btn3");
	//btn3->create("P", mediumFont, []() { LOG("test"); }, UIStyle(), UITransform{ {50,70},{0,0} });
	//btn3->create("P", mediumFont, []() { LOG("test2"); }, UIStyle(), getCorrectSize(btn3->getTransform(), UIStyle(), btn3->getText()));

	//Button* btn = menuUI->registerNew<Button>("btn");
	//btn->create("Toggle()", mediumFont, [drdwn] { drdwn->toggle(); }, UIStyle(), UITransform {{50,50}});
	//Button* btn2 = menuUI->registerNew<Button>("btn2");
	//btn2->create("Clear()", mediumFont, [drdwn] { drdwn->clear(); }, UIStyle(), UITransform{ {50,60} });
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
	Renderer::get()->setCustomResolution(vec2<int>(1920,1680));

	//LOG("Aspect ratio:" + std::to_string(graphics->getAspect()));
	
	// Create a Camera
	camera = graphics->initNewCamera("MainCamera");
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

	engine.setGameState(EGameState::GAME);

	createScenes();
	//engine.loadSceneByID(catapultSceneID);
	LOG(CFG_GET_WINDOW_ASPECT());
}

void update(float dt) {

	for (auto& o : engine.loadedScene.getAllObjects().getAll())
		o->syncFromPhysics();

	double mx, my;
	glfwGetCursorPos(window->getRawWindow(), &mx, &my);

	bool lmbDown = glfwGetMouseButton(window->getRawWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

	float mouseX = static_cast<float>(mx);
	float mouseY = static_cast<float>(my);

	if (engine.getGameState() == EGameState::UI)
		menuUI->update(mouseX, mouseY, lmbDown);
	else
		gameUI->update(mouseX, mouseY, lmbDown);
}

void draw() {
	Renderer::get()->clear();

	Renderer::get()->drawScene(&engine.loadedScene, graphics->getCameras().getByName("MainCamera"), graphics->getLights().getAll());

	//if (engine.getGameState() == EGameState::UI)
	//	Renderer::get()->drawUI(menuUI);
	//else
	//	Renderer::get()->drawUI(gameUI);

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

