#ifndef TEXTURE_H
#define TEXTURE_H
#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <stb_image.h>

#include <glm/glm.hpp>

#include <filesystem>
#include <iostream>

struct TextureSettings
{
    unsigned int textarget = GL_TEXTURE_2D;
    unsigned int texlevel = 0;
    unsigned int texinternalformat = GL_RGBA;
    unsigned int texborder = 0;
    unsigned int texformat = GL_RGBA;
    unsigned int textype = GL_UNSIGNED_BYTE;
    unsigned int texwrap = GL_REPEAT;
    unsigned int texfilter = GL_LINEAR;
    unsigned int texsamples = 1;
    bool isMultiSampled = false;
};

class Texture
{
public:
    int width, height, nrChannels;
    std::filesystem::path texPath;
    std::string type;
    unsigned char *data;
    unsigned int ID;

    Texture(){}

    Texture(int frameWidth, int frameHeight, TextureSettings* settings = nullptr)
    {
        if(settings != nullptr)
            textureSettings = settings;
        else
            textureSettings = &defaultSettings;
        width = frameWidth;
        height = frameHeight;
        nrChannels = 3;
        texPath = "NONE (BLANK)";
        glGenTextures(1, &ID);

        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frameWidth, frameHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        if(!textureSettings->isMultiSampled)
        {
            glBindTexture(GL_TEXTURE_2D, ID);
            glTexImage2D(textureSettings->textarget, 0, textureSettings->texinternalformat, width, height, 0, textureSettings->texformat, textureSettings->textype, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureSettings->texfilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureSettings->texfilter);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, ID);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, textureSettings->texsamples, textureSettings->texinternalformat, width, height, GL_TRUE);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
        }
    }
    Texture(float r, float g, float b)
    {
        width = 1;
        height = 1;
        nrChannels = 3;
        texPath = "NONE (COLOR)("+std::to_string(r)+", "+std::to_string(g)+", "+std::to_string(b)+")";
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        data = new unsigned char[3 * sizeof(unsigned char)];
        data[0] = (unsigned char)(r * 255.0f);
        data[1] = (unsigned char)(g * 255.0f);
        data[2] = (unsigned char)(b * 255.0f);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }

    void LoadDDS()
    {
        //ID = texture_loadDDS(texPath.string().c_str(), width, height);
    }

    void LoadSTBI(bool flipVertical)
    {
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureSettings->texwrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureSettings->texwrap);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureSettings->texfilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureSettings->texfilter);
        // load image, create texture and generate mipmaps
        stbi_set_flip_vertically_on_load(flipVertical); // tell stb_image.h to flip loaded texture's on the y-axis.
        data = stbi_load(texPath.string().c_str(), &width, &height, &nrChannels, 0);

        if (data)
        {
            GLenum format = GL_RGB;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 4)
                format = GL_RGBA;

            glTexImage2D(textureSettings->textarget, 0, textureSettings->texinternalformat, width, height, 0, format, textureSettings->textype, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture: "<< texPath << std::endl;
        }
        stbi_image_free(data);
    }

    Texture(std::filesystem::path texturePath, bool flipVertical = false, TextureSettings* settings = nullptr)
    {
        if(settings == nullptr)
            textureSettings = &defaultSettings;
        else
            textureSettings = settings;
        texPath = texturePath;
        if(texPath.extension().string().compare(".dds") == 0)
            LoadDDS();
        else
            LoadSTBI(flipVertical);
    }

    Texture(unsigned char* data)
    {
        texPath = "NONE (DATA)";
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sizeof(data), 1, 0, GL_RED, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    Texture(const unsigned char* data, int frameWidth, int frameHeight, TextureSettings* settings = nullptr)
    {
        texPath = "NONE (DATA)";
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, settings->texinternalformat, frameWidth, frameHeight, 0, settings->texformat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

private:
    TextureSettings defaultSettings;
    TextureSettings* textureSettings;
};

class MultisampleTexture: Texture
{
public:
    MultisampleTexture(){}

    MultisampleTexture(int frameWidth, int frameHeight, TextureSettings* settings = nullptr)
    {
        if(settings != nullptr)
            textureSettings = settings;
        else
            textureSettings = &defaultSettings;
        width = frameWidth;
        height = frameHeight;
        nrChannels = 3;
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, ID);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, textureSettings->texsamples, textureSettings->texformat, width, height, GL_TRUE);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    }

private:
    TextureSettings defaultSettings;
    TextureSettings* textureSettings;
};

#endif //TEXTURE_H
