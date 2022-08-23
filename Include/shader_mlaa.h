#ifndef SHADER_MLAA_H
#define SHADER_MLAA_H

#include <shaderLoader.h>
#include <texture.h>
#include <screenQuad.h>
#include <buffer.h>
#include <utilityMap.h>

#include <iostream>
#include <filesystem>

#include <mlaa/mlaa_glsl.h>

class ShaderMLAA
{
private:
    std::filesystem::path mlaaShaderPath = std::filesystem::current_path().append("Shaders\\mlaa");
    std::filesystem::path mlaaVertPath = absolute(mlaaShaderPath).append("mlaaScreen.vert");
    std::filesystem::path mlaaPass1Path = absolute(mlaaShaderPath).append("mlaa1.frag");
    std::filesystem::path mlaaPass2Path = absolute(mlaaShaderPath).append("mlaa2.frag");
    std::filesystem::path mlaaPass3Path = absolute(mlaaShaderPath).append("mlaa3.frag");
    glm::vec2 screenSize = glm::vec2(1280,720);
    glm::vec2 texelSize = glm::vec2(1)/screenSize;
    ScreenQuad quad;
    UtilityMap areaMap;

    FrameBuffer edgefbo, blendfbo;
    //Texture edgeImage, blendImage;
    unsigned int framebufferImageID,depthImageID;
    Shader mlaa1Shader, mlaa2Shader, mlaa3Shader;
    Shader edgeShader, neighbourhoodShader, blendShader;

    TextureSettings intermediateTextureSettings;

public:
    float fDepthThreshold = 0.1f;
    float fLuminanceThreshold = 0.1f;
    int maxDistance = 9;
    int maxSearchSteps = 8;
    FrameBuffer screenfbo;
    Texture edgeImage, blendImage, screenImage;
    Texture BIrg, BIba;

    ShaderMLAA(unsigned int framebufferImageID, unsigned int depthImageID):
        framebufferImageID(framebufferImageID), depthImageID(depthImageID)
    {
        mlaa1Shader = Shader(mlaaVertPath.string().c_str(), mlaaPass1Path.string().c_str());
        mlaa2Shader = Shader(mlaaVertPath.string().c_str(), mlaaPass2Path.string().c_str());
        mlaa3Shader = Shader(mlaaVertPath.string().c_str(), mlaaPass3Path.string().c_str());

        intermediateTextureSettings.textype = GL_FLOAT;
        intermediateTextureSettings.texinternalformat = GL_RGBA32F;
        intermediateTextureSettings.texformat = GL_RGBA;

        edgefbo = FrameBuffer();
        edgeImage = Texture(screenSize.x, screenSize.y, &intermediateTextureSettings);
        edgefbo.BindTexture(edgeImage.ID);

        blendfbo = FrameBuffer();
        blendImage = Texture(screenSize.x, screenSize.y, &intermediateTextureSettings);
        blendfbo.BindTexture(blendImage.ID);

        screenfbo = FrameBuffer();
        screenImage = Texture(screenSize.x, screenSize.y, &intermediateTextureSettings);
        screenfbo.BindTexture(screenImage.ID);

        std::string mlaaPath = "F:\\PREDATOR\\GitHub\\antialiasing\\Shaders\\mlaa\\";
        //edgeShader = Shader(&mlaa_test_vs,&mlaa_test_ps);
    }

    void use()
    {
        glDisable(GL_DEPTH_TEST);

        //First Pass
        edgefbo.BindFrameBuffer();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        mlaa1Shader.use();
        mlaa1Shader.setFloat("fDepthThreshold", fDepthThreshold);
        mlaa1Shader.setFloat("fLuminanceThreshold", fLuminanceThreshold);
        mlaa1Shader.setVec2("texelSize", texelSize);
        mlaa1Shader.setInt("screenTexture", 0);
        mlaa1Shader.setInt("depthTexture", 1);
        quad.BindTexture(GL_TEXTURE0, framebufferImageID);
        quad.BindTexture(GL_TEXTURE1, depthImageID);
        quad.Draw();

        //Second Pass
        blendfbo.BindFrameBuffer();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        mlaa2Shader.use();
        mlaa2Shader.setVec2("texelSize", texelSize);
        mlaa2Shader.setInt("maxDistance", maxDistance);
        mlaa2Shader.setInt("maxSearchSteps", maxSearchSteps);
        mlaa2Shader.setInt("edgeTexture", 0);
        mlaa2Shader.setInt("luminanceTexture", 1);
        mlaa2Shader.setInt("areaTexture", 2);
        quad.BindTexture(GL_TEXTURE0, edgeImage.ID);
        quad.BindTexture(GL_TEXTURE1, framebufferImageID);
        quad.BindTexture(GL_TEXTURE2, areaMap.map.ID);
        quad.Draw();

        //Third Pass
        screenfbo.BindFrameBuffer();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        mlaa3Shader.use();
        mlaa3Shader.setVec2("texelSize", texelSize);
        mlaa3Shader.setInt("colorTexture", 0);
        mlaa3Shader.setInt("blendTexture", 1);
        quad.BindTexture(GL_TEXTURE0, framebufferImageID);
        quad.BindTexture(GL_TEXTURE1, blendImage.ID);
        quad.Draw();
    }

};
#endif //SHADER_MLAA_H
