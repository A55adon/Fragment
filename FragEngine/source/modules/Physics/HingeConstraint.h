#pragma once

#include "core/DataTypes.h"

class SceneObject;

class HingeConstraint {
public:
    HingeConstraint(std::string obj1Name, std::string obj2Name = "")
        : _connector1Name(obj1Name), _connector2Name(obj2Name) {
    }

    std::string getConnector1Name() const { return _connector1Name; }
    std::string getConnector2Name() const { return _connector2Name; }
    bool isWorldAnchored() const { return _connector2Name == ""; }

    void setPoint1(vec3<float> pos) { _point1 = pos; }
    void setPoint2(vec3<float> pos) { _point2 = pos; }
    vec3<float> getPoint1() const { return _point1; }
    vec3<float> getPoint2() const { return _point2; }

    void setHingeAxis1(vec3<float> axis) { _hingeAxis1 = axis.normalized(); }
    void setHingeAxis2(vec3<float> axis) { _hingeAxis2 = axis.normalized(); }
    vec3<float> getHingeAxis1() const { return _hingeAxis1; }
    vec3<float> getHingeAxis2() const { return _hingeAxis2; }

    void setNormalAxis1(vec3<float> axis) { _normalAxis1 = axis.normalized(); }
    void setNormalAxis2(vec3<float> axis) { _normalAxis2 = axis.normalized(); }
    vec3<float> getNormalAxis1() const { return _normalAxis1; }
    vec3<float> getNormalAxis2() const { return _normalAxis2; }

    void setAngleLimits(float minAngle, float maxAngle) {
        _minAngle = minAngle;
        _maxAngle = maxAngle;
    }
    void setMinAngle(float angle) { _minAngle = angle; }
    void setMaxAngle(float angle) { _maxAngle = angle; }
    float getMinAngle() const { return _minAngle; }
    float getMaxAngle() const { return _maxAngle; }

    void lockRotation() { _minAngle = _maxAngle = 0.f; }
    void freeRotation() { _minAngle = -INFINITY; _maxAngle = INFINITY; }
    bool isLocked() const { return _minAngle == _maxAngle; }
    bool isFree() const { return _minAngle <= -1e30f && _maxAngle >= 1e30f; }

    void enableMotor(bool enable) { _motorEnabled = enable; }
    void setMotorTargetVelocity(float velocity) { _motorTargetVelocity = velocity; }
    void setMotorMaxTorque(float torque) { _motorMaxTorque = torque; }

    bool isMotorEnabled() const { return _motorEnabled; }
    float getMotorTargetVelocity() const { return _motorTargetVelocity; }
    float getMotorMaxTorque() const { return _motorMaxTorque; }

    void setCachedConnector1(SceneObject* obj) { _cachedConnector1 = obj; }
    void setCachedConnector2(SceneObject* obj) { _cachedConnector2 = obj; }
    SceneObject* getCachedConnector1() const { return _cachedConnector1; }
    SceneObject* getCachedConnector2() const { return _cachedConnector2; }

private:
    std::string _connector1Name = "";
    std::string _connector2Name = "";  

    SceneObject* _cachedConnector1 = nullptr;
    SceneObject* _cachedConnector2 = nullptr;

    vec3<float> _point1 = { 0, 0, 0 };
    vec3<float> _point2 = { 0, 0, 0 };

    vec3<float> _hingeAxis1 = { 1, 0, 0 };
    vec3<float> _hingeAxis2 = { 1, 0, 0 };

    vec3<float> _normalAxis1 = { 0, 1, 0 };  
    vec3<float> _normalAxis2 = { 0, 1, 0 };

    float _minAngle = -INFINITY;
    float _maxAngle = INFINITY;

    bool _motorEnabled = false;
    float _motorTargetVelocity = 0.f;  // ra
    float _motorMaxTorque = 0.f;       // N⋅m
};