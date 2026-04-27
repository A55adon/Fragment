#include "Fragment.h"

Fragment::Fragment()
{
	//loadedScene->getAllObjects().getAll().reserve(5000);
	scenes.getAll().reserve(10);
	renderUpdateState = ERenderUpdateState::UNCAPPED;
	_physicsUpdateState = EPhysicsUpdateState::CAPPED;
}

void Fragment::runApp()
{
	using clock = std::chrono::high_resolution_clock;

	auto lastTime = clock::now();
	auto lastRenderTime = clock::now();
	auto lastPhysicsTime = clock::now();

	// Debug timing variables
	auto lastDebugPrintTime = clock::now();
	int frameCount = 0;
	float totalFrameTime = 0.0f;
	float totalPhysicsTime = 0.0f;
	float totalRenderTime = 0.0f;
	float totalOtherTime = 0.0f;

	while (true)
	{
		auto currentTime = clock::now();
		std::chrono::duration<float> delta = currentTime - lastTime;
		float dt = delta.count();
		lastTime = currentTime;

		if (!getGraphics()->getWindow()->shouldRun())
		{
			WARN("Window closed");
			return;
		}

		// other
		auto otherStart = clock::now();
		userInput.update(dt);
		

		logicUpdateFn(dt);
		auto otherEnd = clock::now();
		float otherTime = std::chrono::duration<float>(otherEnd - otherStart).count();

		// physics
		float physicsTime = 0.0f;
		if (_loadedSceneID != -1 && loadedScene.getPhysics() != nullptr)
		{
			auto physicsStart = clock::now();
			switch (_physicsUpdateState)
			{
			case EPhysicsUpdateState::STOPPED:
				break;

			case EPhysicsUpdateState::SLOWED:
			{
				float scaledDt = dt * _physicsTimeScale;
				loadedScene.getPhysics()->update(scaledDt);
				break;
			}

			case EPhysicsUpdateState::CAPPED:
			{
				float target = 1.0f / (float)_physicsFPSCap;
				std::chrono::duration<float> physDelta = currentTime - lastPhysicsTime;
				if (physDelta.count() >= target)
				{
					loadedScene.getPhysics()->update(target);
					lastPhysicsTime = currentTime;
				}
				break;
			}
			}
			auto physicsEnd = clock::now();
			physicsTime = std::chrono::duration<float>(physicsEnd - physicsStart).count();
		}

		// render
		float renderTime = 0.0f;
		switch (renderUpdateState)
		{
		case ERenderUpdateState::STOPPED:
			break;

		case ERenderUpdateState::CAPPED:
		{
			float target = 1.0f / (float)_renderFPSCap;
			std::chrono::duration<float> renderDelta = currentTime - lastRenderTime;
			if (renderDelta.count() >= target)
			{
				auto renderStart = clock::now();
				drawUpdateFn();
				auto renderEnd = clock::now();
				renderTime = std::chrono::duration<float>(renderEnd - renderStart).count();
				lastRenderTime = currentTime;
			}
			break;
		}

		case ERenderUpdateState::UNCAPPED:
		{
			auto renderStart = clock::now();
			drawUpdateFn();
			auto renderEnd = clock::now();
			renderTime = std::chrono::duration<float>(renderEnd - renderStart).count();
			break;
		}
	}

		frameCount++;
		totalFrameTime += dt;
		totalPhysicsTime += physicsTime;
		totalRenderTime += renderTime;
		totalOtherTime += otherTime;

		auto debugNow = clock::now();
		std::chrono::duration<float> debugDelta = debugNow - lastDebugPrintTime;
		if (_debugFPS && debugDelta.count() >= .5f)
		{
			avgFPS = frameCount / debugDelta.count();
			avgFrameTime = (totalFrameTime / frameCount) * 1000.0f; 
			avgPhysicsTime = (totalPhysicsTime / frameCount) * 1000.0f;
			avgRenderTime = (totalRenderTime / frameCount) * 1000.0f;
			avgOtherTime = (totalOtherTime / frameCount) * 1000.0f;

			//std::cout << "=== Performance Debug ===" << std::endl;
			//std::cout << "FPS: " << avgFPS << std::endl;
			//std::cout << "Frame Time: " << avgFrameTime << " ms" << std::endl;
			//std::cout << "  - Physics: " << avgPhysicsTime << " ms ("
			//	<< (avgPhysicsTime / avgFrameTime * 100.0f) << "%)" << std::endl;
			//std::cout << "  - Render: " << avgRenderTime << " ms ("
			//	<< (avgRenderTime / avgFrameTime * 100.0f) << "%)" << std::endl;
			//std::cout << "  - Other (Input/UI/Logic): " << avgOtherTime << " ms ("
			//	<< (avgOtherTime / avgFrameTime * 100.0f) << "%)" << std::endl;
			//std::cout << "=========================" << std::endl;

			frameCount = 0;
			totalFrameTime = 0.0f;
			totalPhysicsTime = 0.0f;
			totalRenderTime = 0.0f;
			totalOtherTime = 0.0f;
			lastDebugPrintTime = debugNow;
		}
	}
}

void Fragment::startApp()
{
	startFn();
}

void Fragment::setAppStartFn(std::function<void()> fn)
{
	startFn = fn;
}

void Fragment::setAppDrawUpdateFn(std::function<void()> fn)
{
	drawUpdateFn = fn;
}

void Fragment::setAppLogicUpdateFn(std::function<void(float dt)> fn)
{
	logicUpdateFn = fn;
}

void Fragment::setEndFn(std::function<void()> fn)
{
	endFn = fn;
}

Graphics* Fragment::getGraphics()
{
	return &graphics;
}

Input* Fragment::initUserInput()
{
	userInput.init(graphics.getWindow()->getRawWindow());
	return &userInput;
}

UI* Fragment::createNewUI()
{
	uis.push_back(std::make_unique<UI>());
	return uis.back().get();
}

void Fragment::loadSceneByID(int ID) {
	ASSERT(scenes.getByID(ID), "Could not load scene with id: " + std::to_string(ID));
	Scene* ref = scenes.getByID(ID);
	loadedScene = Scene();
	loadedScene.activatePhysics();

	std::unordered_map<SceneObject*, std::string> keyMap; // old ref obj -> new key

	for (auto& obj : ref->getAllObjects().getAll()) {
		SceneObject* newObject = loadedScene.getAllObjects().createNew(obj->getName(), loadedScene.getPhysics());
		// Copy properties
		newObject->setPhysicsMesh(obj->getPhysicsMesh());
		newObject->setRenderMesh(obj->getRenderMesh());
		newObject->setPosition(obj->getPosition());
		newObject->setRotation(obj->getRotation());
		newObject->setScale(obj->getScale());
		newObject->initPhysics(obj->settings);

		std::string newKey = GlobalSceneObjectKeyRegister::registerObject(newObject);
		keyMap[obj.get()] = newKey;
	}

	for (auto& constraint : ref->getAxisConstraints()) {
		AxisConstraint newConstraint = constraint;
		SceneObject* oldObj = constraint.getCachedObject(); // from reference scene
		if (oldObj && keyMap.count(oldObj)) {
			newConstraint.setObjectKey(keyMap[oldObj]);
		}
		loadedScene.addAxisConstraint(newConstraint);
		// Get reference to the just-added constraint
		AxisConstraint& addedConstraint = loadedScene.getAxisConstraints().back();
		loadedScene.getPhysics()->addAxisConstraint(addedConstraint);
	}

	// Copy and remap hinge constraints
	for (auto& constraint : ref->getHingeConstraints()) {
		HingeConstraint newConstraint = constraint;
		SceneObject* old1 = constraint.getCachedConnector1();
		if (old1 && keyMap.count(old1))
			newConstraint.setConnector1Key(keyMap[old1]);

		if (!newConstraint.isWorldAnchored()) {
			SceneObject* old2 = constraint.getCachedConnector2();
			if (old2 && keyMap.count(old2))
				newConstraint.setConnector2Key(keyMap[old2]);
		}
		loadedScene.addHingeConstraint(newConstraint);
		// Get reference to the just-added constraint
		HingeConstraint& addedConstraint = loadedScene.getHingeConstraints().back();
		loadedScene.getPhysics()->addHingeConstraint(addedConstraint);
	}

	_loadedSceneID = ID;
}

void Fragment::saveScene() {
	ASSERT(scenes.getByID(_loadedSceneID), "No scene found with ID: " + std::to_string(_loadedSceneID));
	saveSceneToID(_loadedSceneID);
}
void Fragment::saveSceneToID(int ID) {
	ASSERT(scenes.getByID(ID), "No scene found with ID: " + std::to_string(ID));
	Scene* ref = scenes.getByID(ID);
	ref->getAllObjects().reset();
	ref->getAxisConstraints().clear();
	ref->getHingeConstraints().clear();

	// Build a map from old pointer -> new key
	std::unordered_map<SceneObject*, std::string> keyMap;

	for (auto& obj : loadedScene.getAllObjects().getAll()) {
		SceneObject* newObject = ref->getAllObjects().createNew(obj->getName(), loadedScene.getPhysics());
		// Copy properties
		newObject->setPhysicsMesh(obj->getPhysicsMesh());
		newObject->setRenderMesh(obj->getRenderMesh());
		newObject->setPosition(obj->getPosition());
		newObject->setRotation(obj->getRotation());
		newObject->setScale(obj->getScale());
		newObject->initPhysics(obj->settings);

		// Register the new object
		std::string newKey = GlobalSceneObjectKeyRegister::registerObject(newObject);
		keyMap[obj.get()] = newKey;
	}

	for (auto& constraint : loadedScene.getAxisConstraints()) {
		AxisConstraint newConstraint = constraint; // copy
		SceneObject* oldObj = constraint.getCachedObject(); // resolves old key
		if (oldObj && keyMap.count(oldObj)) {
			newConstraint.setObjectKey(keyMap[oldObj]);
		}
		ref->addAxisConstraint(newConstraint);
	}

	// Copy hinge constraints with updated keys
	for (auto& constraint : loadedScene.getHingeConstraints()) {
		HingeConstraint newConstraint = constraint;

		SceneObject* old1 = constraint.getCachedConnector1();
		if (old1 && keyMap.count(old1))
			newConstraint.setConnector1Key(keyMap[old1]);

		if (!newConstraint.isWorldAnchored()) {
			SceneObject* old2 = constraint.getCachedConnector2();
			if (old2 && keyMap.count(old2))
				newConstraint.setConnector2Key(keyMap[old2]);
		}
		ref->addHingeConstraint(newConstraint);
	}

	loadedScene.getAllObjects().reset();
	loadedScene.getAxisConstraints().clear();
	loadedScene.getHingeConstraints().clear();
}

Scene* Fragment::saveToNewScene(std::string name)
{
	Scene* tempScene = createNewScene(name);
	saveSceneToID(tempScene->getID());
	return tempScene;
}

Scene* Fragment::createNewScene(std::string name)
{
	if (name.empty()) { // if no name give, return a scene with name "scene" followed by the scene ID
		Scene* tempScene = scenes.createNew("Scene");
		tempScene->setName("Scene" + std::to_string(tempScene->getID()));
		return tempScene;
	}

	return scenes.createNew(name);
}

SceneObject* Fragment::createNewSceneObject(std::string name)
{
	if (name.empty()) { // if no name give, return a sceneObject with name "sceneObject" followed by the scene ID
		SceneObject* tempSceneObject = loadedScene.getAllObjects().createNew<SceneObject>("SceneObject", loadedScene.getPhysics());
		tempSceneObject->setName("SceneObject" + std::to_string(tempSceneObject->getID()));
		GlobalSceneObjectKeyRegister::registerObject(tempSceneObject);
		return tempSceneObject;
	}

	return loadedScene.getAllObjects().createNew<SceneObject>(name, loadedScene.getPhysics());
}


