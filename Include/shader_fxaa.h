#ifndef SHADER_FXAA_H
#define SHADER_FXAA_H

#include <shaderLoader.h>
#include <texture.h>
#include <screenQuad.h>
#include <buffer.h>

#include <iostream>
#include <filesystem>
#include <fxaa/fxaa_glsl.h>

class ShaderFXAA
{
private:
    glm::vec2 screenSize = glm::vec2(1280,720);
    glm::vec2 texelSize = glm::vec2(1)/screenSize;
    ScreenQuad quad;
    unsigned int framebufferImageID,depthImageID;
    Shader fxaaLumaShader, fxaaScreenShader;

public:
    FrameBuffer screenfbo, lumafbo;
    Texture lumaImage, screenImage;

    ShaderFXAA(unsigned int framebufferImageID, unsigned int depthImageID):
            framebufferImageID(framebufferImageID), depthImageID(depthImageID)
    {
        lumaImage = Texture(screenSize.x, screenSize.y);
        lumafbo.BindTexture(lumaImage.ID);

        screenImage = Texture(screenSize.x, screenSize.y);
        screenfbo.BindTexture(screenImage.ID);

        std::string fxaaPath = "F:\\PREDATOR\\GitHub\\antialiasing\\Shaders\\fxaa\\";
        fxaaScreenShader = Shader(&fxaa_vs, &fxaa_ps,"FXAA", &fxaaPath);
        fxaaLumaShader = Shader(&fxaa_luma_vs, &fxaa_luma_ps, "FXAA");
    }

    void use()
    {
        //Luma calculation into alpha channel (Blending should be disabled before this)
        /*glDisable(GL_DEPTH_TEST);
        lumafbo.BindFrameBuffer();
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        fxaaLumaShader.use();
        fxaaLumaShader.setInt("albedo_tex", 0);
        quad.BindTexture(GL_TEXTURE0, framebufferImageID);
        quad.Draw();*/

        //FXAA Pass
        screenfbo.BindFrameBuffer();
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        fxaaScreenShader.use();
        fxaaScreenShader.setInt("albedo_tex", 0);
        fxaaScreenShader.setVec2("rcpFrame", texelSize);
        quad.BindTexture(GL_TEXTURE0, framebufferImageID);
        quad.Draw();
    }
};
#endif //SHADER_FXAA_H
