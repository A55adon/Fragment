#pragma once

#include "core/SceneObject.h"

class Scene {
public:
	Scene() : _ID(nextID++) {}

	bool operator==(const Scene& other) const {
		return this->_ID == other._ID;
	}

	// Setters & Getters
	void setName(std::string name) { _name = name; }
	std::string getName() { return _name; }

	int getID() const { return _ID; } // ID shouldn't be set manually

	Register<SceneObject>& getAllObjects() { return _sceneObjects; }
	void setAllObjects(Register<SceneObject> objects) {
		_sceneObjects = std::move(objects);
	}

	void activatePhysics() { _physics = std::make_unique<Physics>(); } // creates new Physics
	void deactivatePhysics() { _physics.reset(); } // deletes Physics

	Physics* getPhysics() { return _physics.get(); }

private:
    inline static int nextID = 0;
	int _ID;
	std::string _name;
	Register<SceneObject> _sceneObjects;
	std::unique_ptr<Physics> _physics;
	// player/camera
};
