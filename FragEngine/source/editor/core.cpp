#include "editor/core.h"

void setupUI()
{
	editorUIStyle.setBorder(true);
	editorUIStyle.setBorderColor(Color::fromHex("#1E1E1E"));
	editorUIStyle.setBorderEdges(5);
	editorUIStyle.setBorderRadius(0.0f);
	editorUIStyle.setBorderWidth(0.01f);
	editorUIStyle.setPrimaryColor(Color::fromHex("#37404C"));
	editorUIStyle.setSecondaryColor(Color::fromHex("#1E242D"));
	editorUIStyle.setTertiaryColor(Color::fromHex("#2B3441"));

	editorUI = editorEngine->createNewUI();
	editorUI->setDebugDraw(true);

	vec2<float> viewportSize = { 70,70 }; //US

	////// Left side

	//// Hirarchy Panel
	Transform hirarchyPanelTr;
	hirarchyPanelTr.setSizeUS({ (100.f - viewportSize.x) / 2.f, 70.f });
	hirarchyPanelTr.setPositionUS({ (100.f - viewportSize.x) / 4.f, 100.f - hirarchyPanelTr.getSizeUS().y / 2.f });
	Rectangle* hirarchyPanel = editorUI->createRectangle(hirarchyPanelTr, editorUIStyle);

	// Add Object Btn
	std::unique_ptr<Button> addObjBtn = std::make_unique<Button>();
	addObjBtn->setStyle(editorUIStyle);
	addObjBtn->setOnClick([]() { LOG("test"); });
	Transform addObjBtnTr;
	addObjBtnTr.setPositionUS({50.f, 50.f});
	addObjBtnTr.setSizeUS({10.f,10.f});
	addObjBtn->setTransform(addObjBtnTr);
	addObjBtn->setFont(editorUI->getDefaultFont(18));
	addObjBtn->setLabel("+");
	addObjBtn->rebuild();
	hirarchyPanel->addChild(std::move(addObjBtn));

	//// SceneList
	Transform sceneListPanelTr;
	sceneListPanelTr.setSizeUS({ (100.f - viewportSize.x) / 2.f, 100.f - hirarchyPanelTr.getSizeUS().y});
	sceneListPanelTr.setPositionUS({ (100.f - viewportSize.x) / 4.f, sceneListPanelTr.getSizeUS().y / 2.f});
	Rectangle* sceneListPanel = editorUI->createRectangle(sceneListPanelTr, editorUIStyle);

	////// Right side
	
	//// Inspector
	Transform inspectorPanelTr;
	inspectorPanelTr.setSizeUS({ (100.f - viewportSize.x) / 2.f, 100.f });
	inspectorPanelTr.setPositionUS({ 100.f - (inspectorPanelTr.getSizeUS().x / 2.f), inspectorPanelTr.getSizeUS().y / 2.f });
	Rectangle* inspectorPanel = editorUI->createRectangle(inspectorPanelTr, editorUIStyle);
	
	////// Top side
	
	//// Stats
	Transform statsPanelTr;
	statsPanelTr.setSizeUS({ viewportSize.x, (100.f - viewportSize.y) / 2.f });
	statsPanelTr.setPositionUS({ 100.f / 2.f, 100.f - statsPanelTr.getSizeUS().y / 2.f});
	Rectangle* statsPanel = editorUI->createRectangle(statsPanelTr, editorUIStyle);
	
}

void startEditor()
{
	LOG("Editor started");

	editorGraphics = editorEngine->getGraphics();
	editorWindow = editorGraphics->getWindow();

	editorWindow->setSize({1200, 800});
	editorWindow->setCursorFocus(false);
	Renderer::get()->setCustomResolution({1200, 800});

	editorCamera = editorGraphics->initNewCamera("EditorCamera", editorWindow->getAspect());
	editorCamera->setPosition(vec3<float>(0,0,5.f));

	editorLight = editorGraphics->initNewLight("EditorLight");
	editorLight->position = vec3<float>(0.f, 2.f, 3.f);
	editorLight->emissionStrength = 5.f;

	setupUI();

	PhysicsSettings cfg;
	cfg.isStatic = true;

	SceneObject* defaultObject = editorEngine->createNewSceneObject();
	defaultObject->getRenderMesh().createCube();
	defaultObject->getRenderMesh().setColor(Color::Blue);
	defaultObject->setPosition(vec3<float>(0, 0, 0));
	defaultObject->setScale(vec3<float>(1.f, 1.f, 1.f));
	defaultObject->initPhysics(cfg);
	

	Scene* defaultScene = editorEngine->saveToNewScene("defaultScene");
	int defaultSceneID = defaultScene->getID();

	editorEngine->loadSceneByID(defaultSceneID);
}

void updateEditor(float dt)
{
	for (auto& o : editorEngine->loadedScene.getAllObjects().getAll())
		o->syncFromPhysics();

	GLFWwindow* win = editorWindow->getRawWindow();
	double mx, my;
	glfwGetCursorPos(win, &mx, &my);
	bool lmbDown = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	editorUI->update(static_cast<float>(mx), static_cast<float>(my), lmbDown);
}

void drawEditor() 
{
	Renderer::get()->clear();

	Renderer::get()->drawScene(&editorEngine->loadedScene, editorCamera, editorGraphics->getLights().getAll());

	Renderer::get()->drawUI(editorUI);

	Renderer::get()->present();
	glfwSwapBuffers(editorWindow->getRawWindow());
	glfwPollEvents();
}


void endEditor() 
{
	LOG("Editor ended");

}

void start() {
	editorEngine = new Fragment();
	editorEngine->setAppStartFn(startEditor);
	editorEngine->setAppDrawUpdateFn(drawEditor);
	editorEngine->setAppLogicUpdateFn(updateEditor);
	editorEngine->setEndFn(endEditor);
	editorEngine->startApp();
	editorEngine->runApp();
	delete editorEngine;
}