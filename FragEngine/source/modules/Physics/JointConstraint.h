#pragma once

#include "core/DataTypes.h"
#include "core/SceneObject.h"

// hingepoint relative to obj
// motor?
// Constraints

class JointConstraint {
public:
private:
	vec3<float> _jointPosition;
	quat _jointRotation;

	vec3<float> _jointRotationFreedom; // eg. 45,45,0 means like 45deg in each dir from start pos and 0 is locked z rotation

	SceneObject* _connector1;
	SceneObject* _connector2;
};