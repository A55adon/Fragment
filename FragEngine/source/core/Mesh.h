#pragma once

#include <vector>
#include <filesystem>
#include <fstream>
#include <string>
#include <limits>

#include "core/DataTypes.h"
#include "core/DefaultFunctions.h"
#include "modules/Graphics/Texture.h"


// Render mesh
class Mesh {
public:
	Mesh() {}

    void setColor(Color c) { 
        for (auto& t : triangles)
            for (auto& v : t.vertices)
                v.color = c;
    }

    void recalculateNormals() {
        for (auto& t : triangles) {
            vec3<float> a = t.vertices[1].position - t.vertices[0].position;
            vec3<float> b = t.vertices[2].position - t.vertices[0].position;
            t.normal = normalize(cross(a, b));
        }
    }

    void loadFromFile(std::filesystem::path path) {
        std::ifstream file(path);
        if (!file) {
            WARN("Could not open file " + path.string());
            return;
        }

        triangles.clear();

        std::vector<vec3<float>> positions;
        std::vector<vec3<float>> normals;
        std::vector<vec2<float>> texCoordinates;

        std::string line;

        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string prefix;
            ss >> prefix;

            if (prefix == "v") {
                float x, y, z;
                ss >> x >> y >> z;
                positions.emplace_back(x, y, z);
            }
            else if (prefix == "vn") {
                float x, y, z;
                ss >> x >> y >> z;
                normals.emplace_back(x, y, z);
            } 
            else if (prefix == "vt") {
                float s, t;
                ss >> s >> t;
                texCoordinates.emplace_back(s, t);
            }
            else if (prefix == "f") {
                std::string v1, v2, v3;
                ss >> v1 >> v2 >> v3;

                Triangle tri;

                auto parseVertex = [&](const std::string& token, int idx) {
                    int posIndex = 0;
                    int texIndex = 0;   // <-- add this
                    int normIndex = 0;

                    size_t firstSlash = token.find('/');
                    size_t lastSlash = token.rfind('/');

                    if (firstSlash == std::string::npos) {
                        posIndex = std::stoi(token);
                    }
                    else {
                        posIndex = std::stoi(token.substr(0, firstSlash));

                        // Extract texture index from between the slashes
                        if (lastSlash != firstSlash) {
                            std::string texStr = token.substr(firstSlash + 1, lastSlash - firstSlash - 1);
                            if (!texStr.empty())
                                texIndex = std::stoi(texStr);
                            normIndex = std::stoi(token.substr(lastSlash + 1));
                        }
                        else {
                            // Format is v/vt (no normal)
                            std::string texStr = token.substr(firstSlash + 1);
                            if (!texStr.empty())
                                texIndex = std::stoi(texStr);
                        }
                    }

                    tri.vertices[idx].position = positions[posIndex - 1];
                    tri.vertices[idx].color = Color(1, 1, 1, 1);
                    // Use texIndex instead of posIndex
                    if (texIndex > 0)
                        tri.vertices[idx].texCoordinate = texCoordinates[texIndex - 1];

                    if (normIndex > 0)
                        tri.normal = normals[normIndex - 1];
                };

                parseVertex(v1, 0);
                parseVertex(v2, 1);
                parseVertex(v3, 2);

                if (tri.normal.x == 0 && tri.normal.y == 0 && tri.normal.z == 0) {
                    vec3<float> a = tri.vertices[1].position - tri.vertices[0].position;
                    vec3<float> b = tri.vertices[2].position - tri.vertices[0].position;
                    tri.normal = normalize(cross(a, b));
                }

                triangles.push_back(tri);
            }
        }

        file.close();
    }

    void addTexture(std::string path) {
        texture = std::make_shared<Texture>(path.c_str());
    }

    void createCube() {
        triangles.clear();
        texture = nullptr;

        Vertex v[8];

        v[0].position = { -0.5f, -0.5f, -0.5f };
        v[1].position = { 0.5f, -0.5f, -0.5f };
        v[2].position = { 0.5f,  0.5f, -0.5f };
        v[3].position = { -0.5f,  0.5f, -0.5f };

        v[4].position = { -0.5f, -0.5f,  0.5f };
        v[5].position = { 0.5f, -0.5f,  0.5f };
        v[6].position = { 0.5f,  0.5f,  0.5f };
        v[7].position = { -0.5f,  0.5f,  0.5f };

        triangles.push_back({ v[0], v[2], v[1] });
        triangles.push_back({ v[0], v[3], v[2] });

        triangles.push_back({ v[4], v[5], v[6] });
        triangles.push_back({ v[4], v[6], v[7] });

        triangles.push_back({ v[0], v[5], v[4] });
        triangles.push_back({ v[0], v[1], v[5] });

        triangles.push_back({ v[3], v[6], v[2] });
        triangles.push_back({ v[3], v[7], v[6] });

        triangles.push_back({ v[1], v[6], v[5] });
        triangles.push_back({ v[1], v[2], v[6] });

        triangles.push_back({ v[0], v[7], v[3] });
        triangles.push_back({ v[0], v[4], v[7] });

        recalculateNormals();
    }

    std::pair<vec3<float>, vec3<float>> getBoundingBox(vec2<float> worldPos = { 0,0 }, vec2<float> windowSize = { 1,1 }) {
        float ndcX = (worldPos.x / 100.f) * 2.f - 1.f;
        float ndcY = 1.f - (worldPos.y / 100.f) * 2.f;

        vec3<float> mn(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        vec3<float> mx(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

        for (const auto& tri : triangles) {
            for (const auto& v : tri.vertices) {
                float x = v.position.x + ndcX;
                float y = v.position.y + ndcY;
                mn.x = std::min(mn.x, x); mx.x = std::max(mx.x, x);
                mn.y = std::min(mn.y, y); mx.y = std::max(mx.y, y);
                mn.z = std::min(mn.z, v.position.z); mx.z = std::max(mx.z, v.position.z);
            }
        }
        return { mn, mx };
    }

    std::vector<Triangle> triangles;
    std::shared_ptr<Texture> texture;
};