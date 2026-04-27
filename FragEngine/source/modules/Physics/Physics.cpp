#include "core/SceneObject.h"
#include "core/DefaultFunctions.h"
#include "modules/Physics/Physics.h"
#include <thread>
#include <cstdarg>


static int sRefCount = 0;

Physics::Physics()
{
    if (sRefCount == 0) {
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::Trace = [](const char* fmt, ...) {
            va_list args; va_start(args, fmt);
            vprintf(fmt, args); va_end(args); printf("\n");
            };
        JPH::RegisterTypes();
    }
    sRefCount++;

    tempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
    jobSystem = new JPH::JobSystemThreadPool(
        JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
        std::thread::hardware_concurrency() - 1);
    physicsSystem.Init(1024, 0, 1024, 1024,
        bpLayerInterface, objVsBPFilter, objLayerPairFilter);
    physicsSystem.SetGravity(JPH::Vec3(0, -9.81f, 0));
}

Physics::~Physics()
{
    JPH::BodyInterface& bi = physicsSystem.GetBodyInterface();
    for (auto id : _anchorBodies) {
        bi.RemoveBody(id);
        bi.DestroyBody(id);
    }

    delete tempAllocator;
    delete jobSystem;

    sRefCount--;
    if (sRefCount == 0) {
        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
    }
}

void Physics::update(float dt)
{
    physicsSystem.Update(dt, 1, tempAllocator, jobSystem);
}

void Physics::addAxisConstraint(AxisConstraint& constraint)
{
    SceneObject* obj = constraint.getCachedObject();
    ASSERT(obj, "AxisConstraint has no cached object, call setCachedObject() first!");

    JPH::BodyInterface& bodyInterface = physicsSystem.GetBodyInterface();
    JPH::BodyID bodyID = obj->getBodyID();
    ASSERT(!bodyID.IsInvalid(), "SceneObject has no physics body, call initPhysics() first");

    vec3<float> objPos = obj->getPosition();
    vec3<float> pivotPos = constraint.getPivotPosition();
    JPH::RVec3 anchorWorldPos(
        objPos.x + pivotPos.x,
        objPos.y + pivotPos.y,
        objPos.z + pivotPos.z
    );

    JPH::BodyCreationSettings anchorSettings(
        new JPH::SphereShape(0.01f), // tiny invisible anchor
        anchorWorldPos,
        JPH::Quat::sIdentity(),
        JPH::EMotionType::Static,
        Layers::CONSTRAINT_ANCHOR
    );
    anchorSettings.mIsSensor = true;

    JPH::BodyID anchorID = bodyInterface.CreateAndAddBody(anchorSettings, JPH::EActivation::DontActivate);
    _anchorBodies.push_back(anchorID); // store for cleanup — add this vector to Physics.h

    // Set up the SixDOF constraint
    JPH::SixDOFConstraintSettings settings;
    settings.mSpace = JPH::EConstraintSpace::WorldSpace;
    settings.mPosition1 = anchorWorldPos;
    settings.mPosition2 = anchorWorldPos;

    settings.mAxisX1 = JPH::Vec3::sAxisX();
    settings.mAxisY1 = JPH::Vec3::sAxisY();
    settings.mAxisX2 = JPH::Vec3::sAxisX();
    settings.mAxisY2 = JPH::Vec3::sAxisY();

    // Helper to configure one axis
    auto configureAxis = [&](JPH::SixDOFConstraintSettings::EAxis axis, float mn, float mx) {
        if (mn == mx)
            settings.MakeFixedAxis(axis);
        else if (mn <= -1e30f && mx >= 1e30f)
            settings.MakeFreeAxis(axis);
        else
            settings.SetLimitedAxis(axis, mn, mx);
        };

    vec3<float> movMin = constraint.getMovementMin();
    vec3<float> movMax = constraint.getMovementMax();
    configureAxis(JPH::SixDOFConstraintSettings::EAxis::TranslationX, movMin.x, movMax.x);
    configureAxis(JPH::SixDOFConstraintSettings::EAxis::TranslationY, movMin.y, movMax.y);
    configureAxis(JPH::SixDOFConstraintSettings::EAxis::TranslationZ, movMin.z, movMax.z);

    vec3<float> rotMin = constraint.getRotationMin();
    vec3<float> rotMax = constraint.getRotationMax();
    configureAxis(JPH::SixDOFConstraintSettings::EAxis::RotationX, rotMin.x, rotMax.x);
    configureAxis(JPH::SixDOFConstraintSettings::EAxis::RotationY, rotMin.y, rotMax.y);
    configureAxis(JPH::SixDOFConstraintSettings::EAxis::RotationZ, rotMin.z, rotMax.z);

    JPH::BodyID bodies[2] = { anchorID, bodyID };
    JPH::BodyLockMultiWrite locks(physicsSystem.GetBodyLockInterface(), bodies, 2);

    JPH::Body* anchorBody = locks.GetBody(0);
    JPH::Body* objBody = locks.GetBody(1);
    ASSERT(anchorBody && objBody, "Failed to lock physics bodies");

    JPH::Constraint* joltConstraint = settings.Create(*anchorBody, *objBody);
    physicsSystem.AddConstraint(joltConstraint);
    _constraints.push_back(joltConstraint);
}

void Physics::addHingeConstraint(HingeConstraint& constraint)
{
    JPH::BodyInterface& bodyInterface = physicsSystem.GetBodyInterface();

    // Get first object
    SceneObject* obj1 = constraint.getCachedConnector1();
    ASSERT(obj1, "HingeConstraint has no cached connector1, call setCachedConnector1() first!");

    JPH::BodyID body1ID = obj1->getBodyID();
    ASSERT(!body1ID.IsInvalid(), "SceneObject connector1 has no physics body, call initPhysics() first");

    JPH::BodyID body2ID;
    SceneObject* obj2 = nullptr;

    if (constraint.isWorldAnchored()) {
        // Create static anchor body at the world position of point1
        vec3<float> obj1Pos = obj1->getPosition();
        vec3<float> point1 = constraint.getPoint1();
        JPH::RVec3 anchorWorldPos(
            obj1Pos.x + point1.x,
            obj1Pos.y + point1.y,
            obj1Pos.z + point1.z
        );

        JPH::BodyCreationSettings anchorSettings(
            new JPH::SphereShape(0.01f),
            anchorWorldPos,
            JPH::Quat::sIdentity(),
            JPH::EMotionType::Static,
            Layers::CONSTRAINT_ANCHOR
        );
        anchorSettings.mIsSensor = true;

        body2ID = bodyInterface.CreateAndAddBody(anchorSettings, JPH::EActivation::DontActivate);
        _anchorBodies.push_back(body2ID);
    }
    else {
        // Get second object
        obj2 = constraint.getCachedConnector2();
        ASSERT(obj2, "HingeConstraint has no cached connector2, call setCachedConnector2() first!");

        body2ID = obj2->getBodyID();
        ASSERT(!body2ID.IsInvalid(), "SceneObject connector2 has no physics body, call initPhysics() first");
    }

    // Set up the hinge constraint
    JPH::HingeConstraintSettings settings;
    settings.mSpace = JPH::EConstraintSpace::WorldSpace;

    // Convert local points to world space
    vec3<float> obj1Pos = obj1->getPosition();
    vec3<float> point1 = constraint.getPoint1();
    settings.mPoint1 = JPH::Vec3(
        obj1Pos.x + point1.x,
        obj1Pos.y + point1.y,
        obj1Pos.z + point1.z
    );

    if (constraint.isWorldAnchored()) {
        settings.mPoint2 = settings.mPoint1; // Same position for world anchor
    }
    else {
        vec3<float> obj2Pos = obj2->getPosition();
        vec3<float> point2 = constraint.getPoint2();
        settings.mPoint2 = JPH::Vec3(
            obj2Pos.x + point2.x,
            obj2Pos.y + point2.y,
            obj2Pos.z + point2.z
        );
    }

    // Set hinge axis
    vec3<float> hingeAxis1 = constraint.getHingeAxis1();
    vec3<float> normalAxis1 = constraint.getNormalAxis1();
    settings.mHingeAxis1 = JPH::Vec3(hingeAxis1.x, hingeAxis1.y, hingeAxis1.z);
    settings.mNormalAxis1 = JPH::Vec3(normalAxis1.x, normalAxis1.y, normalAxis1.z);

    if (constraint.isWorldAnchored()) {
        settings.mHingeAxis2 = settings.mHingeAxis1;
        settings.mNormalAxis2 = settings.mNormalAxis1;
    }
    else {
        vec3<float> hingeAxis2 = constraint.getHingeAxis2();
        vec3<float> normalAxis2 = constraint.getNormalAxis2();
        settings.mHingeAxis2 = JPH::Vec3(hingeAxis2.x, hingeAxis2.y, hingeAxis2.z);
        settings.mNormalAxis2 = JPH::Vec3(normalAxis2.x, normalAxis2.y, normalAxis2.z);
    }

    // Set angle limits
    if (constraint.isLocked()) {
        settings.mLimitsMin = 0.0f;
        settings.mLimitsMax = 0.0f;
    }
    else if (!constraint.isFree()) {
        settings.mLimitsMin = constraint.getMinAngle();
        settings.mLimitsMax = constraint.getMaxAngle();
    }

    // Configure motor in settings (BEFORE creating constraint)
    if (constraint.isMotorEnabled()) {
        JPH::MotorSettings motorSettings;
        motorSettings.mMinForceLimit = -constraint.getMotorMaxTorque();
        motorSettings.mMaxForceLimit = constraint.getMotorMaxTorque();
        motorSettings.mMinTorqueLimit = -constraint.getMotorMaxTorque();
        motorSettings.mMaxTorqueLimit = constraint.getMotorMaxTorque();
        settings.mMotorSettings = motorSettings;
    }

    // Lock bodies and create constraint
    JPH::BodyID bodies[2] = { body1ID, body2ID };
    JPH::BodyLockMultiWrite locks(physicsSystem.GetBodyLockInterface(), bodies, 2);

    JPH::Body* joltBody1 = locks.GetBody(0);
    JPH::Body* joltBody2 = locks.GetBody(1);
    ASSERT(joltBody1 && joltBody2, "Failed to lock physics bodies");

    JPH::Constraint* joltConstraint = settings.Create(*joltBody1, *joltBody2);

    // Set motor state AFTER creation (this is supported)
    if (constraint.isMotorEnabled()) {
        JPH::HingeConstraint* hinge = static_cast<JPH::HingeConstraint*>(joltConstraint);
        hinge->SetMotorState(JPH::EMotorState::Velocity);
        hinge->SetTargetAngularVelocity(constraint.getMotorTargetVelocity());
    }

    physicsSystem.AddConstraint(joltConstraint);
    _constraints.push_back(joltConstraint);
}