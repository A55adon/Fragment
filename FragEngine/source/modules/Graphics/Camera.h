#pragma once

#include "core/DataTypes.h"

class Camera {
public:  
    Camera(float aspect = 4.f/3.f) : _ID(nextID++) {
        _projection = mat4::perspective(
            70.0f * 3.1415926f / 180.0f,
            aspect,
            0.1f,
            100.0f
        );
    }

    bool operator==(const Camera& other) const {
        return this->_ID == other._ID;
    }

    mat4 getViewMatrix() {
        mat4 translation = mat4::translate({ -_position.x, -_position.y, -_position.z });
        mat4 rotX = mat4::rotateX(-_rotation.x);
        mat4 rotY = mat4::rotateY(-_rotation.y);
        // rotZ is not used for FPS
        return rotX * rotY * translation;
    }

    vec3<float> getForward()
    {
        float yaw = _rotation.y;
        float pitch = _rotation.x;

        vec3<float> f;
        f.x = sinf(yaw) * cosf(pitch);
        f.y = sinf(pitch);
        f.z = -cosf(yaw) * cosf(pitch);

        // Optional normalisation (already unit length if pitch/yaw are correct)
        float len = sqrtf(f.x * f.x + f.y * f.y + f.z * f.z);
        if (len > 0.0001f) {
            f.x /= len; f.y /= len; f.z /= len;
        }
        return f;
    }

    vec3<float> getRight()
    {
        vec3<float> f = getForward();
        vec3<float> up{ 0.0f, 1.0f, 0.0f };
        vec3<float> r;
        r.x = f.y * up.z - f.z * up.y;
        r.y = f.z * up.x - f.x * up.z;
        r.z = f.x * up.y - f.y * up.x;
        float len = sqrtf(r.x * r.x + r.y * r.y + r.z * r.z);
        r.x /= len; r.y /= len; r.z /= len;
        return r;
    }

    void setPosition(vec3<float> pos) { _position = pos; }
    vec3<float> getPosition() { return _position; }

    void setRotation(vec3<float> rot) { _rotation = rot; }
    vec3<float> getRotation() { return _rotation; }

    void setName(const std::string& name) { _name = name; }
    std::string getName() const { return _name; }
    int getID() const { return _ID; }

    mat4 getProjection() { return _projection; }

private:
    inline static int nextID = 0;
    int _ID = -1;
    std::string _name;
	vec3<float> _position{ 0,0,0 };
	vec3<float> _rotation{ 0,0,0 };
    mat4 _projection;
};