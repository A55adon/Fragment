#pragma once

#include "core/DataTypes.h"
#include "modules/Graphics/Texture.h"

struct Mesh2D {
private:

    std::vector<std::array<vec2<float>, 3>> _mesh;
    std::shared_ptr<Texture> _texture;
public:
    void addTexture(std::string path) { _texture = std::make_shared<Texture>(path.c_str()); }
    Texture* getTexture() { return _texture.get(); }

    void setMesh(const std::vector<std::array<vec2<float>, 3>>& mesh) {
        _mesh = mesh;
    }
    const std::vector<std::array<vec2<float>, 3>>& getMesh() const {
        return _mesh;
    }
};