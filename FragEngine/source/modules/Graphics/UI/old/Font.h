#pragma once

#include "modules/Graphics/UI/UIObject.h"
#include "stb_image_write.h"
#include <fstream>
#include <vector>
#include <string>
#include <memory>

struct FontAtlas {
    GLuint _textureID = 0;
    std::vector<stbtt_bakedchar> _chars;
    int _width = 512;
    int _height = 512;

    std::vector<unsigned char> _bitmap;

    void saveToPNG(const char* path = "font_atlas.png") {
        if (!_bitmap.empty()) {
            stbi_write_png(path, _width, _height, 1, _bitmap.data(), _width);
        }
    }
};

struct Glyph {
    std::string _symbol = "";
    int _ASCII = -1;
    vec2<float> _size = { 0,0 };
};

class Font {
public:
    Font(std::string fontPath = "res/Roboto-Black.ttf", int fontSize = 16)
        : _fontPath(fontPath), _size(fontSize) {
        init();
    }

    std::shared_ptr<Texture> getTexture() const { return _mesh.texture; }

    const std::vector<stbtt_bakedchar>& getChars() const { return _atlas._chars; }

    int getAtlasWidth() const { return _atlas._width; }
    int getAtlasHeight() const { return _atlas._height; }

    int getSize() const { return _size; }
    void setSize(int size) {
        if (_size != size) {
            _size = size;
            init();
        }
    }

    const Color& getColor() const { return _color; }
    void setColor(const Color& color) { _color = color; }

private:
    void init() {
        std::ifstream file(_fontPath, std::ios::binary);
        if (!file.is_open()) {
            ERR("Couldn't open font file: " << _fontPath);
            return;
        }

        std::vector<unsigned char> ttf(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
        );

        int charCount = 96; // 32-127
        int requiredSize = calculateAtlasSize(_size, charCount);

        _atlas._width = requiredSize;
        _atlas._height = requiredSize;
        _atlas._chars.resize(charCount);

        _atlas._bitmap.resize(_atlas._width * _atlas._height, 0);

        int bakedChars = stbtt_BakeFontBitmap(
            ttf.data(),
            0,
            _size,
            _atlas._bitmap.data(),
            _atlas._width,
            _atlas._height,
            32,  // First character
            charCount,  // Number of characters
            _atlas._chars.data()
        );

        _mesh.texture = std::make_shared<Texture>();
        _mesh.texture->createFromRaw(
            _atlas._bitmap.data(),
            _atlas._width,
            _atlas._height
        );

    }

    int calculateAtlasSize(int fontSize, int charCount) {
        int padding = 2;
        int charArea = (fontSize + padding) * (fontSize + padding);
        int totalArea = charArea * charCount;

        int size = (int)ceil(sqrt(totalArea));

        size = (int)pow(2, ceil(log2(size)));

        size = std::clamp(size, 256, 4096);

        return size;
    }

private:
    std::string _fontPath;
    int _size = 16;
    Color _color = Color::White;
    FontAtlas _atlas;
    struct {
        std::shared_ptr<Texture> texture;
    } _mesh;
};