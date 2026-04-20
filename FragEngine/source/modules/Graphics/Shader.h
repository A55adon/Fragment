#pragma once

#include "glad/glad.h"
#include "core/DataTypes.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
  

class Shader
{
public:
    unsigned int ID;
  
    Shader() = default;
    Shader(const char* vertexPath, const char* fragmentPath);
    void use();

    void setBool(const std::string &name, bool value) const;  
    void setInt(const std::string &name, int value) const;   
    void setFloat(const std::string &name, float value) const;
    void setVec3(const std::string& name, vec3<float> value) const;
    void setVec4(const std::string& name, vec4<float> value) const;
    void setMat4(const std::string &name, mat4 value) const;
};