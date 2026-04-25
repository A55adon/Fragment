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

// TODO: 
// Add uniqueIdentifier keys for sceneobjcets so that when moving from one to another register the object can still be found
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
	void loadSceneByID(int ID) {
		ASSERT(scenes.getByID(ID), "Could not load scene with id: " + std::to_string(ID));
		Scene* ref = scenes.getByID(ID);
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

		for (auto& constraint : ref->getAxisConstraints()) {
			loadedScene.addAxisConstraint(constraint);
		}

		for (auto& constraint : loadedScene.getAxisConstraints()) {
			SceneObject* obj = loadedScene.getAllObjects().getByName(constraint.getObjectName());
			ASSERT(obj, "Invalid object to set as cached");
			constraint.setCachedObject(obj);
			loadedScene.getPhysics()->addAxisConstraint(constraint);
		}

		for (auto& constraint : ref->getHingeConstraints()) {
			loadedScene.addHingeConstraint(constraint);
		}

		for (auto& constraint : loadedScene.getHingeConstraints()) {
			SceneObject* obj1 = loadedScene.getAllObjects().getByName(constraint.getConnector1Name());
			ASSERT(obj1, "Invalid object to set as cached");
			constraint.setCachedConnector1(obj1);
			SceneObject* obj2 = loadedScene.getAllObjects().getByName(constraint.getConnector2Name());
			if (!obj2) { LOG("No object 2 found, using world anchor"); }
			constraint.setCachedConnector2(obj2);
			loadedScene.getPhysics()->addHingeConstraint(constraint);
		}

		_loadedSceneID = ID;
	}
	// Saves currently loaded scene and overrides the correstponding one in the register
	void saveScene() {
		ASSERT(scenes.getByID(_loadedSceneID), "No scene found with ID: " + std::to_string(_loadedSceneID));
		Scene* ref  = scenes.getByID(_loadedSceneID);
		ref->getAllObjects().reset();
		ref->getAxisConstraints().clear();
		for (auto& obj : loadedScene.getAllObjects().getAll())
		{
			SceneObject* newObject = ref->getAllObjects().createNew(obj->getName(), loadedScene.getPhysics()); // add object directly to scene

			newObject->setPhysicsMesh(obj->getPhysicsMesh());
			newObject->setRenderMesh(obj->getRenderMesh());
			newObject->setPosition(obj->getPosition());
			newObject->setRotation(obj->getRotation());
			newObject->setScale(obj->getScale());
			newObject->initPhysics(obj->settings);

		}
		for (auto& constraint : loadedScene.getAxisConstraints()) {
			ref->addAxisConstraint(constraint);
		}
	}

	void saveSceneToID(int ID) {
		ASSERT(scenes.getByID(ID), "No scene found with ID: " + std::to_string(ID));
		Scene* ref = scenes.getByID(ID);
		ref->getAllObjects().reset();
		ref->getAxisConstraints().clear();
		for (auto& obj : loadedScene.getAllObjects().getAll())
		{
			SceneObject* newObject = ref->getAllObjects().createNew(obj->getName(), loadedScene.getPhysics()); // add object directly to scene

			newObject->setPhysicsMesh(obj->getPhysicsMesh());
			newObject->setRenderMesh(obj->getRenderMesh());
			newObject->setPosition(obj->getPosition());
			newObject->setRotation(obj->getRotation());
			newObject->setScale(obj->getScale());
			newObject->initPhysics(obj->settings);
		}

		for (auto& constraint : loadedScene.getAxisConstraints()) {
			ref->addAxisConstraint(constraint);
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

	Input userInput;
	Graphics graphics;
	std::vector<std::unique_ptr<UI>> uis;

	std::function<void()> startFn;
	std::function<void()> drawUpdateFn;
	std::function<void()> endFn;
	std::function<void(float dt)> logicUpdateFn;
	bool appExit = false;
};