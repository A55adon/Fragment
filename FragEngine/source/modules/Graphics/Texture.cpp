#include "modules/Graphics/Texture.h"
#include <iostream>
#include <vector>
// Creates a Texture with normal RGB
Texture::Texture(const char *image_path)
{
    stbi_set_flip_vertically_on_load(true);
    // Load image with stb_image
    int width, height, nrChanels;
    unsigned char *data = stbi_load(image_path, &width, &height, &nrChanels, 0);
    
    if(!data) 
    {
        std::cerr << "Could not load data from file: " << image_path << std::endl;
    }
    
    // Generate texture objects
    glGenTextures(1, &ID); // Create texture
    glBindTexture(GL_TEXTURE_2D, ID);

    GLenum format = (nrChanels == 3) ? GL_RGB : GL_RGBA;

    // Generate Texture from image data
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(data);
}

Texture::~Texture()
{
    glDeleteTextures(1, &ID);
}


int Texture::getTexture()
{
    return ID;
}

void Texture::bind(unsigned int unit) const
{
    if (ID == 0) return;
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::createFromRaw(unsigned char* data, int width, int height)
{
    if (ID == 0)
        glGenTextures(1, &ID);

    glBindTexture(GL_TEXTURE_2D, ID);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
}
void Texture::saveBoundTextureToFile(const char* path, int width, int height)
{
    std::vector<unsigned char> pixels(width * height);

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, pixels.data());

    stbi_write_png(path, width, height, 1, pixels.data(), width);
}