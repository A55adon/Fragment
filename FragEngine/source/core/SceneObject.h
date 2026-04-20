#pragma once

#include "core/Mesh.h"
#include "modules/Physics/Physics.h"

struct PhysicsSettings {
    float friction = 1.f;    // 0 - 1
    float restitution = 0.f; // 0 - 1
    float mass = 5.f;        // in kg
    bool  isStatic = false;
};

struct SceneObjectState {
    std::string     name = "";
    vec3<float>     position = vec3<float>();
    vec3<float>     scale = vec3<float>(1, 1, 1);
    quat            rotation = quat(0, 0, 0, 1);
    PhysicsSettings physicsSettings = PhysicsSettings();
    Color           color = Color::Red;
};

class SceneObject {
public:
    SceneObject() = default;
    SceneObject(Physics* physics) : _ID(nextID++), _physics(physics) {}

    // Only allow moving and disallow copying because of the Jolt physics body
    SceneObject(const SceneObject&) = delete;
    SceneObject& operator=(const SceneObject&) = delete;
    SceneObject(SceneObject&& o) noexcept
        : _ID(o._ID), _physics(o._physics), _name(std::move(o._name)),
        _renderMesh(std::move(o._renderMesh)), _physicsMesh(std::move(o._physicsMesh)),
        _position(o._position), _rotation(o._rotation), _scale(o._scale),
        model(o.model), _bodyID(o._bodyID)
    {}
    SceneObject& operator=(SceneObject&&) = delete;

    ~SceneObject() {
        if (_physics && !_bodyID.IsInvalid())
            destroyPhysics(_physics->getSystem());
    }

    bool operator==(const SceneObject& other) const {
        return this->_ID == other._ID;
    }

    void restoreFromState(const SceneObjectState& state) {
        _name = state.name;
        _position = state.position;
        _rotation = state.rotation;
        _scale = state.scale;

        if (_physics) {
            if (!_bodyID.IsInvalid())
                destroyPhysics(_physics->getSystem());
            _buildBody(state.physicsSettings);
        }
    }

    mat4 getModelMatrix() { // translate * rotate * scale
        return mat4::translate(_position) * mat4::fromQuaternion(_rotation) * mat4::scale(_scale);
    }

    // Setters
    void setName(std::string name) { _name = name; }
    void setPosition(vec3<float> position) { _position = position; }
    void setScale(vec3<float> scale) { _scale = scale; }
    void setRotation(quat rotation) { _rotation = rotation; }
    void setRenderMesh(Mesh mesh) { _renderMesh = mesh; }
    void setPhysicsMesh(Mesh mesh) { _physicsMesh = mesh; }

    // Getters
    int getID() const { return _ID; }
    std::string& getName() { return _name; }
    vec3<float>& getPosition() { return _position; }
    vec3<float>& getScale() { return _scale; }
    quat& getRotation() { return _rotation; }
    Mesh& getRenderMesh() { return _renderMesh; }
    Mesh& getPhysicsMesh() { return _physicsMesh; }
    JPH::BodyID getBodyID() const { return _bodyID; }

    // Physics
    void initPhysics(const PhysicsSettings& settings = PhysicsSettings{}) // Should only be called once after position/scale are set
    {
        this->settings = settings;
        _buildBody(settings);
    }

    void syncFromPhysics()
    {
        if (_bodyID.IsInvalid()) return;

        JPH::BodyInterface& bodyInterface = _physics->getSystem().GetBodyInterface();

        JPH::RVec3 pos = bodyInterface.GetPosition(_bodyID);
        JPH::Quat  rot = bodyInterface.GetRotation(_bodyID);

        _position = { pos.GetX(), pos.GetY(), pos.GetZ() };
        _rotation = { rot.GetX(), rot.GetY(), rot.GetZ(), rot.GetW() };

        model = getModelMatrix();
    }

    // Call on destruction / scene unload
    void destroyPhysics(JPH::PhysicsSystem& physicsSystem)
    {
        if (_bodyID.IsInvalid()) return;
        JPH::BodyInterface& bodyInterface = physicsSystem.GetBodyInterface();
        bodyInterface.RemoveBody(_bodyID);
        bodyInterface.DestroyBody(_bodyID);
        _bodyID = JPH::BodyID();
    }

    PhysicsSettings settings;
private:
    // Creates/recreates the Jolt body from current _position/_scale/_rotation + given settings.
    void _buildBody(const PhysicsSettings& settings)
    {
        if (!_physics) return;

        JPH::BodyInterface& bodyInterface = _physics->getSystem().GetBodyInterface();

        JPH::Vec3 halfExtents(_scale.x / 2, _scale.y / 2, _scale.z / 2);
        JPH::BoxShapeSettings shapeSettings(halfExtents);
        shapeSettings.mConvexRadius = 0.02f;

        auto shapeResult = shapeSettings.Create();
        JPH::ShapeRefC shape = shapeResult.Get();

        JPH::RVec3 pos(_position.x, _position.y, _position.z);
        JPH::Quat  rot(_rotation.x, _rotation.y, _rotation.z, _rotation.w);

        JPH::EMotionType motionType = settings.isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;
        JPH::ObjectLayer layer = settings.isStatic ? Layers::NON_MOVING : Layers::MOVING;

        JPH::BodyCreationSettings joltSettings(shape, pos, rot, motionType, layer);
        joltSettings.mFriction = settings.friction;
        joltSettings.mRestitution = settings.restitution;

        if (!settings.isStatic) {
            joltSettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
            joltSettings.mMassPropertiesOverride.mMass = settings.mass;
        }

        _bodyID = bodyInterface.CreateAndAddBody(joltSettings, JPH::EActivation::Activate);
    }

    // Identifying
    inline static int nextID = 0;
    int _ID = -1;
    std::string _name = "";

    vec3<float> _position;
    quat _rotation;
    vec3<float> _scale = { 1.f, 1.f, 1.f };

    // Rendering
    Mesh _renderMesh;
    mat4 model = mat4::identity();

    // Physics
    Physics* _physics = nullptr;
    Mesh _physicsMesh; // optional — if not set, render mesh is used
    JPH::BodyID _bodyID = JPH::BodyID();
};