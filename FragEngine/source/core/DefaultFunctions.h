#pragma once

#include <string>
#include <iostream>
#include "core/config.h"

#define LOG(msg){std::cout << "[LOG] " << msg << std::endl;}
#define WARN(msg){std::clog << "[WARING] " << msg << std::endl;}
#define ERR(msg){std::cerr << "[ERROR] " << msg << std::endl;}

#define ASSERT(x, msg) \
    if (!(x)) { \
        std::cerr << "ASSERT FAILED: " << msg << "\n"; \
        std::abort(); \
    }

inline std::string generateProjectName() {
    return std::format("{} ({}.{}.{})",
        CFG_PROJECT_NAME,
        CFG_PROJECT_VERSION_MAJOR,
        CFG_PROJECT_VERSION_MINOR,
        CFG_PROJECT_VERSION_FIX);
}


inline vec3<float> cross(const vec3<float>& a, const vec3<float>& b)
{
    return vec3<float>(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

template<typename T>
vec3<T> toVec3(const vec4<T>& v) {
    if (v.w != 0) return vec3<T>(v.x / v.w, v.y / v.w, v.z / v.w);
    return vec3<T>(v.x, v.y, v.z);
}

// mat4 * vec4
inline vec4<float> operator*(const mat4& m, const vec4<float>& v) {
    vec4<float> r;
    r.x = m.v[0][0] * v.x + m.v[0][1] * v.y + m.v[0][2] * v.z + m.v[0][3] * v.w;
    r.y = m.v[1][0] * v.x + m.v[1][1] * v.y + m.v[1][2] * v.z + m.v[1][3] * v.w;
    r.z = m.v[2][0] * v.x + m.v[2][1] * v.y + m.v[2][2] * v.z + m.v[2][3] * v.w;
    r.w = m.v[3][0] * v.x + m.v[3][1] * v.y + m.v[3][2] * v.z + m.v[3][3] * v.w;
    return r;
}

template<typename T>
inline T dot(const vec3<T>& a, const vec3<T>& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

template<typename T>
inline vec3<T> normalize(const vec3<T>& v) {
    T len = sqrt(dot(v, v));
    if (len < 1e-8) return vec3<T>(0, 0, 0);
    return vec3<T>(v.x / len, v.y / len, v.z / len);
}