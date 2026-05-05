#include "core/SceneObjectKeyRegister.h"
#include <unordered_map>
#include <sstream>

static std::unordered_map<std::string, SceneObject*> s_keys;
static std::unordered_map<SceneObject*, std::string> s_reverse;
static size_t s_counter = 0;

std::string SceneObjectKeyRegister::generateNewKey() {
    std::stringstream ss;
    ss << "obj_" << s_counter++;
    return ss.str();
}

std::string SceneObjectKeyRegister::getKeyByObj(SceneObject* obj) {
    auto it = s_reverse.find(obj);
    if (it != s_reverse.end()) {
        return it->second;
    }
    return "";
}

std::string SceneObjectKeyRegister::registerObject(SceneObject* obj) {
    std::string key = generateNewKey();
    s_keys[key] = obj;
    s_reverse[obj] = key;
    return key;
}

void SceneObjectKeyRegister::unregisterObject(SceneObject* obj) {
    auto it = s_reverse.find(obj);
    if (it != s_reverse.end()) {
        s_keys.erase(it->second);
        s_reverse.erase(it);
    }
}

SceneObject* SceneObjectKeyRegister::getObjByKey(const std::string& key) {
    auto it = s_keys.find(key);
    if (it != s_keys.end()) {
        return it->second;
    }
    return nullptr;
}