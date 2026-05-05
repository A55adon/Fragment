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

	Transform hirarchyPanelTr;
	hirarchyPanelTr.setPositionUS({ 7.5f, 50 });
	hirarchyPanelTr.setSizeUS({ 15, 100 });
	Rectangle* hirarchyPanel = editorUI->createRectangle(hirarchyPanelTr, editorUIStyle);
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

	editorLight = editorGraphics->initNewLight("EditorLight");
	editorLight->position = vec3<float>(0.f, 2.f, 0.f);

	setupUI();
}

void updateEditor(float dt)
{
	for (auto& o : editorEngine->loadedScene.getAllObjects().getAll())
		o->syncFromPhysics();
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