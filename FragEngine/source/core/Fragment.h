#pragma once

#include <chrono>
#include <functional>
#include <algorithm>
#include <unordered_map>

#include "modules/Graphics/Graphics.h"
#include "modules/Graphics/UI/UI.h"
#include "modules/IO/UserInput/Input.h"
#include "modules/Physics/Physics.h"
#include "modules/Graphics/UI/UITypes.h"
#include "core/SceneObject.h"
#include "core/Scene.h"
#include "modules/Monitoring/Monitor.h"

enum class EPhysicsUpdateState {
	STOPPED,
	CAPPED,
	SLOWED
};

enum class ERenderUpdateState {
	STOPPED,
	CAPPED,
	UNCAPPED
};

class Fragment {

public:
	Fragment();
	void runApp();
	void startApp();

	void setAppStartFn(std::function<void()> fn);
	void setAppDrawUpdateFn(std::function<void()> fn);
	void setAppLogicUpdateFn(std::function<void(float dt)> fn);
	void setEndFn(std::function<void()> fn);

	Graphics* initGraphics();

	Input* initUserInput();

	UI* initNewUI();

	Graphics* getGraphics() { return &graphics; }
	Input* getInput() { return &userInput; }

	void changeCursorFocus(bool focussed) {
		if (focussed) glfwSetInputMode(graphics.getWindow()->getRawWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		else glfwSetInputMode(graphics.getWindow()->getRawWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	void setGameState(EGameState newState) {
		gameState = newState;
		userInput.setGameState(newState);
	}

	EGameState getGameState() {
		return gameState;
	}

	// Clones the wanted scene from the register into the loaded scene
	void loadSceneByID(int id) {
		if (scenes.getByID(id) == nullptr) return;
		Scene* ref = scenes.getByID(id);
		loadedScene = Scene(); // reset scene
		loadedScene.activatePhysics();

		for (auto& obj : ref->getAllObjects().getAll())
		{
			SceneObject* newObject = loadedScene.getAllObjects().createNew(obj->getName(), loadedScene.getPhysics()); // add object directly to scene

			newObject->setPhysicsMesh(obj->getPhysicsMesh());
			newObject->setRenderMesh(obj->getRenderMesh());
			newObject->setPosition(obj->getPosition());
			newObject->setRotation(obj->getRotation());
			newObject->setScale(obj->getScale());
			newObject->initPhysics(obj->settings);
		}
		_loadedSceneID = id;
	}
	// Saves currently loaded scene and overrides the correstponding one in the register
	void saveScene() {
		ASSERT(scenes.getByID(_loadedSceneID), "No scene found with ID: " + std::to_string(_loadedSceneID));
		scenes.getByID(_loadedSceneID)->getAllObjects().reset();
		for (auto& obj : loadedScene.getAllObjects().getAll())
		{
			SceneObject* newObject = scenes.getByID(_loadedSceneID)->getAllObjects().createNew(obj->getName(), loadedScene.getPhysics()); // add object directly to scene

			newObject->setPhysicsMesh(obj->getPhysicsMesh());
			newObject->setRenderMesh(obj->getRenderMesh());
			newObject->setPosition(obj->getPosition());
			newObject->setRotation(obj->getRotation());
			newObject->setScale(obj->getScale());
			newObject->initPhysics(obj->settings);

		}

	}
	void saveSceneToID(int ID) {
		ASSERT(scenes.getByID(ID), "No scene found with ID: " + std::to_string(ID));
		scenes.getByID(ID)->getAllObjects().reset();
		for (auto& obj : loadedScene.getAllObjects().getAll())
		{
			SceneObject* newObject = scenes.getByID(ID)->getAllObjects().createNew(obj->getName(), loadedScene.getPhysics()); // add object directly to scene

			newObject->setPhysicsMesh(obj->getPhysicsMesh());
			newObject->setRenderMesh(obj->getRenderMesh());
			newObject->setPosition(obj->getPosition());
			newObject->setRotation(obj->getRotation());
			newObject->setScale(obj->getScale());
			newObject->initPhysics(obj->settings);
		}

	}

	int getLoadedSceneID() { return _loadedSceneID; }
	Scene* getLoadedScene() { return &loadedScene; }

	void end() {
		endFn();
	}

	void setDebugFPS(bool debug) {
		_debugFPS = debug;
	}

	Scene loadedScene;
	Register<Scene> scenes;


	EPhysicsUpdateState& getPhysicsUpdateState() { return _physicsUpdateState; }

	void setPhysicsTimeScale(float value) { _physicsTimeScale = value; }
	float& getPhysicsTimeScale() { return _physicsTimeScale; }

	float avgFPS = -1.f;
	float avgFrameTime = -1.f;
	float avgPhysicsTime = -1.f;
	float avgRenderTime = -1.f;
	float avgOtherTime = -1.f;

private:
	ERenderUpdateState renderUpdateState;
	EPhysicsUpdateState _physicsUpdateState;

	EGameState gameState = EGameState::GAME;

	bool _debugFPS = false;

	int _loadedSceneID = -1;

	int renderFPSCap = 60;        // fps
	int physicsFPSCap = 240;      // fps
	float _physicsTimeScale = 0.5f; // 50% speed — only used when SLOWED


	std::unordered_map<int, SceneObjectState> _sceneSnapshot;

	Input userInput;
	Graphics graphics;
	std::vector<std::unique_ptr<UI>> uis;

	std::function<void()> startFn;
	std::function<void()> drawUpdateFn;
	std::function<void()> endFn;
	std::function<void(float dt)> logicUpdateFn;
	bool appExit = false;
};