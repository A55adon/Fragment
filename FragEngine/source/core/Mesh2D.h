#pragma once

#include "core/DataTypes.h"
#include "modules/Graphics/Texture.h"
#include <array>
struct Vertex2D {
    vec2<float> position;
    Color color;
};
struct Triangle2D {
    std::array<Vertex2D, 3> vertices;
};

struct Mesh2D {
private:
    std::vector<Triangle2D> _mesh;
    std::shared_ptr<Texture> _texture;
public:
    void addTexture(std::string path) { _texture = std::make_shared<Texture>(path.c_str()); }
    Texture* getTexture() { return _texture.get(); }

    void setMesh(std::vector<Triangle2D>& mesh) {
        _mesh = mesh;
    }

    std::vector<Triangle2D>& getTriangles() {
        return _mesh;
    }
};