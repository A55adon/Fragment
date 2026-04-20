#pragma once

#include "glad/glad.h"
#include "stb_image.h"
#include "stb_image_write.h"

class Texture {
    unsigned int ID;
public:
    Texture() : ID(0) {};
    Texture(const char* image_path);
    ~Texture();
    int getTexture();
    void bind(unsigned int unit = 0) const;

    void createFromRaw(unsigned char* data, int width, int height);

    static void saveBoundTextureToFile(const char* path, int width, int height);

};