#ifndef SCREENQUAD_H
#define SCREENQUAD_H

#include <glad/glad.h>

class ScreenQuad
{
private:
    unsigned int quadVAO, quadVBO;
    float quadVertices[24] = {
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            1.0f,  1.0f,  1.0f, 1.0f
    };
public:
    ScreenQuad()
    {
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    }

    void BindTexture(unsigned int textureLoc, unsigned int textureID)
    {
        glBindVertexArray(quadVAO);
        glActiveTexture(textureLoc);
        glBindTexture(GL_TEXTURE_2D, textureID);
    }

    void Draw()
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    ~ScreenQuad()
    {
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteBuffers(1, &quadVBO);
    }
};

#endif //SCREENQUAD_H
