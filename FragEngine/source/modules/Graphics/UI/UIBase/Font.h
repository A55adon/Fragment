#pragma once

#include "modules/Graphics/Texture.h"
#include "core/DataTypes.h"
#include "core/DefaultFunctions.h"
#include "stb_truetype.h"

#include <fstream>
#include <memory>
#include <string>
#include <vector>

class UIFont {
public:
    UIFont(const std::string& fontPath = "res/Roboto-Black.ttf", int fontSize = 18)
        : _fontPath(fontPath), _size(fontSize)
    {
        rebuild();
    }

    const std::vector<stbtt_bakedchar>& getChars() const { return _chars; }
    std::shared_ptr<Texture> getTexture() const { return _texture; }
    int getAtlasWidth() const { return _atlasWidth; }
    int getAtlasHeight() const { return _atlasHeight; }
    int getSize() const { return _size; }

    void setSize(int size) {
        if (_size == size) return;
        _size = size;
        rebuild();
    }

private:
    void rebuild() {
        std::ifstream file(_fontPath, std::ios::binary);
        if (!file.is_open()) {
            ERR("Could not open UI font: " + _fontPath);
            return;
        }

        std::vector<unsigned char> ttf(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
        );

        const int charCount = 96;
        _atlasWidth = calculateAtlasSize(_size, charCount);
        _atlasHeight = _atlasWidth;
        _chars.assign(charCount, {});
        _bitmap.assign(_atlasWidth * _atlasHeight, 0);

        stbtt_BakeFontBitmap(
            ttf.data(),
            0,
            static_cast<float>(_size),
            _bitmap.data(),
            _atlasWidth,
            _atlasHeight,
            32,
            charCount,
            _chars.data()
        );

        _texture = std::make_shared<Texture>();
        _texture->createFromRaw(_bitmap.data(), _atlasWidth, _atlasHeight);
    }

    static int calculateAtlasSize(int fontSize, int charCount) {
        int padding = 2;
        int charArea = (fontSize + padding) * (fontSize + padding);
        int totalArea = charArea * charCount;
        int size = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(totalArea))));
        size = static_cast<int>(std::pow(2.0f, std::ceil(std::log2(static_cast<float>(size)))));
        return std::clamp(size, 256, 4096);
    }

    std::string _fontPath;
    int _size = 18;
    int _atlasWidth = 0;
    int _atlasHeight = 0;
    std::vector<unsigned char> _bitmap;
    std::vector<stbtt_bakedchar> _chars;
    std::shared_ptr<Texture> _texture;
};
