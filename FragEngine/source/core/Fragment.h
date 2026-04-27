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
#include "core/GlobalSceneObjectKeyRegister.h"
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
	void end() { endFn(); }

	void setAppStartFn(std::function<void()> fn);
	void setAppDrawUpdateFn(std::function<void()> fn);
	void setAppLogicUpdateFn(std::function<void(float dt)> fn);
	void setEndFn(std::function<void()> fn);

	// Modules 
	Graphics* getGraphics();
	Input* initUserInput();
	Input* getInput() { return &userInput; }

	UI* createNewUI();

	
	// Gamestate
	void setGameState(EGameState newState) {
		gameState = newState;
		userInput.setGameState(newState);
	}

	EGameState getGameState() {
		return gameState;
	}

	// Scene Management
	void loadSceneByID(int ID);	// Clones the desired scene from the register into the loaded scene

	void saveScene(); // Saves currently loaded scene and overrides the corresponding one in the register

	void saveSceneToID(int ID);

	Scene* saveToNewScene(std::string name = "");

	int getLoadedSceneID() { return _loadedSceneID; }
	Scene* getLoadedScene() { return &loadedScene; }
	Scene* createNewScene(std::string name = "");

	void addAxisConstraint(AxisConstraint constraint) { loadedScene.addAxisConstraint(constraint); }
	void addHingeConstraint(HingeConstraint constraint) { loadedScene.addHingeConstraint(constraint); }

	Scene loadedScene;
	Register<Scene> scenes;

	// Scene Objects
	SceneObject* createNewSceneObject(std::string name = "");


	// Misc
	void setDebugFPS(bool debug) { _debugFPS = debug; }



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

	int _renderFPSCap = 60;          // fps
	int _physicsFPSCap = 240;        // fps
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