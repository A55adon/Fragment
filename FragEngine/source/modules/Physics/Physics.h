#pragma once

#include "modules/Physics/AxisConstraint.h"
#include "modules/Physics/HingeConstraint.h"

#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyLockMulti.h>
#include <Jolt/Physics/Constraints/SixDOFConstraint.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>

namespace Layers {
    static constexpr JPH::ObjectLayer NON_MOVING = 0;
    static constexpr JPH::ObjectLayer MOVING = 1;
    static constexpr JPH::ObjectLayer CONSTRAINT_ANCHOR = 2;
    static constexpr JPH::uint        NUM_LAYERS = 3;
}

namespace BroadPhaseLayers {
    static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
    static constexpr JPH::BroadPhaseLayer MOVING(1);
}

class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
public:
    bool ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const override {
        if (a == Layers::CONSTRAINT_ANCHOR || b == Layers::CONSTRAINT_ANCHOR)
            return false;
        switch (a) {
        case Layers::NON_MOVING: return b == Layers::MOVING;
        case Layers::MOVING:     return true;
        default:                 return false;
        }
    }
};

class BPLayerInterfaceImpl : public JPH::BroadPhaseLayerInterface {
public:
    BPLayerInterfaceImpl() {
        mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
        mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
        mObjectToBroadPhase[Layers::CONSTRAINT_ANCHOR] = BroadPhaseLayers::NON_MOVING;
    }
    JPH::uint GetNumBroadPhaseLayers() const override { return 2; }
    JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override {
        JPH_ASSERT(layer < Layers::NUM_LAYERS);
        return mObjectToBroadPhase[layer];
    }
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override {
        switch ((JPH::BroadPhaseLayer::Type)layer) {
        case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING: return "NON_MOVING";
        case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:     return "MOVING";
        default: return "UNKNOWN";
        }
    }
#endif
private:
    JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
};

class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
    bool ShouldCollide(JPH::ObjectLayer layer, JPH::BroadPhaseLayer bpLayer) const override {
        if (layer == Layers::CONSTRAINT_ANCHOR)
            return false; 
        switch (layer) {
        case Layers::NON_MOVING: return bpLayer == BroadPhaseLayers::MOVING;
        case Layers::MOVING:     return true;
        default:                 return false;
        }
    }
};


class Physics {
public:
    Physics();
    ~Physics();

    void update(float dt);
    void addAxisConstraint(AxisConstraint& constraint);

    void addHingeConstraint(HingeConstraint& constraint);

    JPH::PhysicsSystem& getSystem() { return physicsSystem; }
    std::vector<JPH::Ref<JPH::Constraint>>& getConstraints() { return _constraints; }

private:
    BPLayerInterfaceImpl                bpLayerInterface;
    ObjectVsBroadPhaseLayerFilterImpl   objVsBPFilter;
    ObjectLayerPairFilterImpl           objLayerPairFilter;

    JPH::TempAllocatorImpl* tempAllocator = nullptr;
    JPH::JobSystemThreadPool* jobSystem = nullptr;
    JPH::PhysicsSystem                  physicsSystem;

    std::vector<JPH::Ref<JPH::Constraint>> _constraints; // keeps constraints alive
    std::vector<JPH::BodyID> _anchorBodies;

};