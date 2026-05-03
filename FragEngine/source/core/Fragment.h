#pragma once

#include <chrono>
#include <functional>
#include <algorithm>
#include <unordered_map>

#include "modules/Graphics/Graphics.h"
#include "modules/Graphics/UI/UI.h"
#include "modules/IO/UserInput/Input.h"
#include "modules/Physics/Physics.h"
#include "core/SceneObject.h"
#include "core/GlobalSceneObjectKeyRegister.h"
#include "core/Scene.h"
#include "modules/Monitoring/Monitor.h"
#include "core/Ray.h"

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
	void setAppFrameEndFn(std::function<void(float frameTimeMs)> fn);
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

	bool cursorInUI(UI* ui) {
		GLFWwindow* window = graphics.getWindow()->getRawWindow();
		ASSERT(window, "No valid window");
		double cursorXPos = 0; // in pixels
		double cursorYPos = 0; // in pixels
		glfwGetCursorPos(window, &cursorXPos, &cursorYPos);
		
		for (auto& el : ui->getRootElements()) {
			if (cursorInUIElement(el, cursorXPos, cursorYPos))
				return true; // hit 
		}
		return false;

	}
	bool cursorInUIElement(UIElement* el, float cursorXPos, float cursorYPos) { // might treat child positions wrong
		for (auto& child : el->getChildren())
			if (cursorInUIElement(child, cursorXPos, cursorYPos))
				return true; // hit in child

		Transform tr = el->getTransform();

		vec2<float> topLeft;
		topLeft.x = tr.getPositionPx().x - tr.getSizePx().x / 2.f;
		topLeft.y = tr.getPositionPx().y + tr.getSizePx().y / 2.f;
		topLeft.y = CFG_WINDOW_HEIGHT - topLeft.y;

		vec2<float> bottomRight;
		bottomRight.x = tr.getPositionPx().x + tr.getSizePx().x / 2.f;
		bottomRight.y = tr.getPositionPx().y - tr.getSizePx().y / 2.f;
		bottomRight.y = CFG_WINDOW_HEIGHT - bottomRight.y;

		if (cursorXPos < bottomRight.x && cursorXPos > topLeft.x) // X axis check
			if (cursorYPos < bottomRight.y && cursorYPos > topLeft.y) // Y axis check
				return true; // hit
		return false;
	}

	EPhysicsUpdateState& getPhysicsUpdateState() { return _physicsUpdateState; }

	void setPhysicsTimeScale(float value) { _physicsTimeScale = value; }
	float& getPhysicsTimeScale() { return _physicsTimeScale; }

	float avgFPS = -1.f;
	float avgFrameTime = -1.f;
	float avgPhysicsTime = -1.f;
	float avgRenderTime = -1.f;
	float avgOtherTime = -1.f;
	float lastFrameTimeMs = 0.f;
	float lastInputTimeMs = 0.f;
	float lastLogicTimeMs = 0.f;
	float lastPhysicsTimeMs = 0.f;
	float lastRenderTimeMs = 0.f;

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
	std::function<void(float frameTimeMs)> frameEndFn;
	bool appExit = false;
};
