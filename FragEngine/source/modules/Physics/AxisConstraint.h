#pragma once

#include "core/DataTypes.h"
#include "core/GlobalSceneObjectKeyRegister.h"

class SceneObject;

// Constraint on one object (world-anchored)
class AxisConstraint {
public:
    AxisConstraint(std::string objKey) : _objectKey(objKey) {}

    // Pivot position (relative to object center)
    void setPivotPosition(vec3<float> pos)          { _pivotPointPosition = pos; }
    vec3<float> getPivotPosition() const            { return _pivotPointPosition; }

    // Pivot rotation (relative to object center)
    void setPivotRotation(vec3<float> rot)          { _pivotPointRotation = rot; }
    vec3<float> getPivotRotation() const            { return _pivotPointRotation; }

    // Rotation limits per axis
    void setRotationMax(vec3<float> max)            { _constraintRotationAxisMax = max; }
    void setRotationMin(vec3<float> min)            { _constraintRotationAxisMin = min; }
    void setRotationLimits(vec3<float> min, vec3<float> max) {
        _constraintRotationAxisMin = min;
        _constraintRotationAxisMax = max;
    }
    vec3<float> getRotationMax() const              { return _constraintRotationAxisMax; }
    vec3<float> getRotationMin() const              { return _constraintRotationAxisMin; }

    // Lock a single rotation axis
    void lockRotationX() { _constraintRotationAxisMin.x = _constraintRotationAxisMax.x = 0.f; }
    void lockRotationY() { _constraintRotationAxisMin.y = _constraintRotationAxisMax.y = 0.f; }
    void lockRotationZ() { _constraintRotationAxisMin.z = _constraintRotationAxisMax.z = 0.f; }
    void lockAllRotation() { lockRotationX(); lockRotationY(); lockRotationZ(); }

    // Free a single rotation axis
    void freeRotationX() { _constraintRotationAxisMin.x = -INFINITY; _constraintRotationAxisMax.x = INFINITY; }
    void freeRotationY() { _constraintRotationAxisMin.y = -INFINITY; _constraintRotationAxisMax.y = INFINITY; }
    void freeRotationZ() { _constraintRotationAxisMin.z = -INFINITY; _constraintRotationAxisMax.z = INFINITY; }
    void freeAllRotation() { freeRotationX(); freeRotationY(); freeRotationZ(); }

    // Movement limits per axis
    void setMovementMax(vec3<float> max)            { _constraintMovementAxisMax = max; }
    void setMovementMin(vec3<float> min)            { _constraintMovementAxisMin = min; }
    void setMovementLimits(vec3<float> min, vec3<float> max) {
        _constraintMovementAxisMin = min;
        _constraintMovementAxisMax = max;
    }
    vec3<float> getMovementMax() const              { return _constraintMovementAxisMax; }
    vec3<float> getMovementMin() const              { return _constraintMovementAxisMin; }

    // Lock a single movement axis
    void lockMovementX() { _constraintMovementAxisMin.x = _constraintMovementAxisMax.x = 0.f; }
    void lockMovementY() { _constraintMovementAxisMin.y = _constraintMovementAxisMax.y = 0.f; }
    void lockMovementZ() { _constraintMovementAxisMin.z = _constraintMovementAxisMax.z = 0.f; }
    void lockAllMovement() { lockMovementX(); lockMovementY(); lockMovementZ(); }

    // Free a single movement axis
    void freeMovementX() { _constraintMovementAxisMin.x = -INFINITY; _constraintMovementAxisMax.x = INFINITY; }
    void freeMovementY() { _constraintMovementAxisMin.y = -INFINITY; _constraintMovementAxisMax.y = INFINITY; }
    void freeMovementZ() { _constraintMovementAxisMin.z = -INFINITY; _constraintMovementAxisMax.z = INFINITY; }
    void freeAllMovement() { freeMovementX(); freeMovementY(); freeMovementZ(); }

    // Get state of individual axes
    bool isRotationLockedX() const { return _constraintRotationAxisMin.x == _constraintRotationAxisMax.x; }
    bool isRotationLockedY() const { return _constraintRotationAxisMin.y == _constraintRotationAxisMax.y; }
    bool isRotationLockedZ() const { return _constraintRotationAxisMin.z == _constraintRotationAxisMax.z; }

    bool isMovementLockedX() const { return _constraintMovementAxisMin.x == _constraintMovementAxisMax.x; }
    bool isMovementLockedY() const { return _constraintMovementAxisMin.y == _constraintMovementAxisMax.y; }
    bool isMovementLockedZ() const { return _constraintMovementAxisMin.z == _constraintMovementAxisMax.z; }

    std::string getObjectKey() const { return _objectKey; }

    SceneObject* getCachedObject() const { return GlobalSceneObjectKeyRegister::getObjByKey(_objectKey); }

private:
    std::string _objectKey = "";

    vec3<float> _pivotPointPosition = { 0, 0, 0 };
    vec3<float> _pivotPointRotation = { 0, 0, 0 };

    vec3<float> _constraintRotationAxisMax = {  INFINITY,  INFINITY,  INFINITY };
    vec3<float> _constraintRotationAxisMin = { -INFINITY, -INFINITY, -INFINITY };
    vec3<float> _constraintMovementAxisMax = {  INFINITY,  INFINITY,  INFINITY };
    vec3<float> _constraintMovementAxisMin = { -INFINITY, -INFINITY, -INFINITY };
};