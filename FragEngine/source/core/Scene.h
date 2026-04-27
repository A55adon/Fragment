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

	void addAxisConstraint(AxisConstraint constraint) { _axisConstraints.push_back(constraint); }
	std::vector<AxisConstraint>& getAxisConstraints() { return _axisConstraints; }

	void addHingeConstraint(HingeConstraint constraint) { _hingeConstraints.push_back(constraint); }
	std::vector<HingeConstraint>& getHingeConstraints() { return _hingeConstraints; }

private:
    inline static int nextID = 0;
	int _ID;
	std::string _name;

	Register<SceneObject> _sceneObjects;
	std::unique_ptr<Physics> _physics;

	std::vector<AxisConstraint> _axisConstraints;
	std::vector<HingeConstraint> _hingeConstraints;
};
