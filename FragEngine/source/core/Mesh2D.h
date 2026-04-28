#pragma once

#include "core/DataTypes.h"
#include "modules/Graphics/Texture.h"
#include <array>
struct Vertex2D {
    Vertex2D() = default;
    Vertex2D(vec2<float> pos, Color col) : position(pos), texCoordinate(0.0f, 0.0f), color(col) {}
    Vertex2D(vec2<float> pos, vec2<float> tex, Color col) : position(pos), texCoordinate(tex), color(col) {}

    vec2<float> position;
    vec2<float> texCoordinate{ 0.0f, 0.0f };
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
    void setTexture(const std::shared_ptr<Texture>& texture) { _texture = texture; }

    void setMesh(std::vector<Triangle2D>& mesh) {
        _mesh = mesh;
    }

    std::vector<Triangle2D>& getTriangles() {
        return _mesh;
    }
};
