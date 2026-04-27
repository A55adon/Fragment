#pragma once 

#include "core/DataTypes.h"

class SceneObject;

class GlobalSceneObjectKeyRegister {
public:
    static std::string generateNewKey();
    static std::string registerObject(SceneObject* obj);
    static void unregisterObject(SceneObject* obj);
    static std::string getKeyByObj(SceneObject* obj);
    static SceneObject* getObjByKey(const std::string& key);
};