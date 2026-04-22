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
    SceneObject* obj = constraint.getObject(); // you'll need getObject() on AxisConstraint
    ASSERT(obj, "AxisConstraint has no object");

    JPH::BodyInterface& bodyInterface = physicsSystem.GetBodyInterface();
    JPH::BodyID bodyID = obj->getBodyID();
    ASSERT(!bodyID.IsInvalid(), "SceneObject has no physics body — call initPhysics() first");

    // Build a static anchor body at the pivot world position
    // (pivot is relative to the object, so offset by object position)
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

    std::cout << "Constraint created between anchor " << anchorID.GetIndexAndSequenceNumber()
        << " and body " << bodyID.GetIndexAndSequenceNumber() << std::endl;
    std::cout << "Constraint enabled: " << joltConstraint->GetEnabled() << std::endl;

    std::cout << "Object position: " << objPos.x << ", " << objPos.y << ", " << objPos.z << std::endl;
    std::cout << "Anchor position: " << anchorWorldPos.GetX() << ", " << anchorWorldPos.GetY() << ", " << anchorWorldPos.GetZ() << std::endl;
}

