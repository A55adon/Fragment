#pragma once

#include "core/DataTypes.h"
#include <string>

class LightSource {
public:
    LightSource() : _ID(nextID++) {}

    int getID() const { return _ID; }

    const std::string& getName() const { return _name; }
    void setName(const std::string& name) { _name = name; }

    vec3<float> position{ 0,0,0 };
    float emissionStrength = 1.0f;
    Color lightColor;

private:
    inline static int nextID = 0;

    int _ID;
    std::string _name;
};