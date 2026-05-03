#pragma once

#include "core/DataTypes.h"
#include "core/SceneObject.h"

struct Ray {
    vec3<float> origin;
    vec3<float> direction;
};

// Unproject mouse coords into a world-space ray
inline Ray screenToRay(float mouseX, float mouseY,
    float screenW, float screenH,
    const mat4& view, const mat4& projection)
{
    // Convert mouse to NDC
    float ndcX = (2.0f * mouseX / screenW) - 1.0f;
    float ndcY = 1.0f - (2.0f * mouseY / screenH);  // flip Y

    // Inverse matrices
    // You'll need to add mat4::inverse() — see below
    mat4 invProj = mat4::inverse(projection);
    mat4 invView = mat4::inverse(view);

    // Unproject near and far points
    vec4<float> rayClipNear = { ndcX, ndcY, -1.0f, 1.0f };
    vec4<float> rayClipFar = { ndcX, ndcY,  1.0f, 1.0f };

    vec4<float> rayViewNear = invProj * rayClipNear;
    vec4<float> rayViewFar = invProj * rayClipFar;

    // Perspective divide
    rayViewNear = rayViewNear * (1.0f / rayViewNear.w);
    rayViewFar = rayViewFar * (1.0f / rayViewFar.w);

    vec4<float> rayWorldNear = invView * rayViewNear;
    vec4<float> rayWorldFar = invView * rayViewFar;

    vec3<float> origin = { rayWorldNear.x, rayWorldNear.y, rayWorldNear.z };
    vec3<float> end = { rayWorldFar.x,  rayWorldFar.y,  rayWorldFar.z };

    vec3<float> dir = normalize(end - origin);
    return { origin, dir };
}

// Ray vs AABB intersection (slab method)
inline bool rayIntersectsAABB(const Ray& ray,
    const vec3<float>& aabbMin,
    const vec3<float>& aabbMax,
    float& outT)
{
    float tMin = 0.0f;
    float tMax = std::numeric_limits<float>::max();

    for (int i = 0; i < 3; ++i)
    {
        float origin = (i == 0) ? ray.origin.x : (i == 1) ? ray.origin.y : ray.origin.z;
        float dir = (i == 0) ? ray.direction.x : (i == 1) ? ray.direction.y : ray.direction.z;
        float mn = (i == 0) ? aabbMin.x : (i == 1) ? aabbMin.y : aabbMin.z;
        float mx = (i == 0) ? aabbMax.x : (i == 1) ? aabbMax.y : aabbMax.z;

        if (std::abs(dir) < 1e-8f) {
            if (origin < mn || origin > mx) return false;
        }
        else {
            float t1 = (mn - origin) / dir;
            float t2 = (mx - origin) / dir;
            if (t1 > t2) std::swap(t1, t2);
            tMin = std::max(tMin, t1);
            tMax = std::min(tMax, t2);
            if (tMin > tMax) return false;
        }
    }

    outT = tMin;
    return true;
}
