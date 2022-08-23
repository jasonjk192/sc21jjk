#ifndef SHADER_SMAA_H
#define SHADER_SMAA_H

#include <shaderLoader.h>
#include <texture.h>
#include <screenQuad.h>
#include <buffer.h>
#include <utilityMap.h>

#include <iostream>
#include <filesystem>

#include <smaa/smaa_glsl.h>

class ShaderSMAA
{
private:
    std::filesystem::path smaaShaderPath = std::filesystem::current_path().append("Shaders\\smaa");
    std::filesystem::path smaaVertPass1Path = absolute(smaaShaderPath).append("smaa1.vert");
    std::filesystem::path smaaVertPass2Path = absolute(smaaShaderPath).append("smaa2.vert");
    std::filesystem::path smaaVertPass3Path = absolute(smaaShaderPath).append("smaa3.vert");
    std::filesystem::path smaaFragPass1Path = absolute(smaaShaderPath).append("smaa1.frag");
    std::filesystem::path smaaFragPass2Path = absolute(smaaShaderPath).append("smaa2.frag");
    std::filesystem::path smaaFragPass3Path = absolute(smaaShaderPath).append("smaa3.frag");
    glm::vec2 screenSize = glm::vec2(1280,720);
    glm::vec2 texelSize = glm::vec2(1)/screenSize;
    ScreenQuad quad;
    UtilityMap areaMap = UtilityMap(UtilityMap::MapType::areaMapSMAA);
    UtilityMap searchMap = UtilityMap(UtilityMap::MapType::searchMapSMAA);
    unsigned int framebufferImageID,depthImageID;
    Shader smaa1Shader, smaa2Shader, smaa3Shader;
    Shader edgeShader, neighbourhoodShader, blendShader;
    TextureSettings intermediateTextureSettings;

public:
    float fDepthThreshold = 0.05f;
    float fLuminanceThreshold = 0.05f;
    float smaaThreshold = 0.05f;
    float weightStrength = 1.f;
    int maxDistance = 9;
    int maxSearchSteps = 16;
    int maxSearchStepsDiag = 8;
    int cornerRounding = 25;
    FrameBuffer screenfbo, edgefbo, blendfbo;
    Texture edgeImage, blendImage, screenImage;

    ShaderSMAA(unsigned int framebufferImageID, unsigned int depthImageID):
            framebufferImageID(framebufferImageID), depthImageID(depthImageID)
    {
        smaa1Shader = Shader(smaaVertPass1Path.string().c_str(), smaaFragPass1Path.string().c_str());
        smaa2Shader = Shader(smaaVertPass2Path.string().c_str(), smaaFragPass2Path.string().c_str());
        smaa3Shader = Shader(smaaVertPass3Path.string().c_str(), smaaFragPass3Path.string().c_str());

        intermediateTextureSettings.textype = GL_FLOAT;
        intermediateTextureSettings.texinternalformat = GL_RGBA32F;
        intermediateTextureSettings.texformat = GL_RGBA;

        edgeImage = Texture(screenSize.x, screenSize.y, &intermediateTextureSettings);
        edgefbo.BindTexture(edgeImage.ID);

        blendImage = Texture(screenSize.x, screenSize.y, &intermediateTextureSettings);
        blendfbo.BindTexture(blendImage.ID);

        screenImage = Texture(screenSize.x, screenSize.y, &intermediateTextureSettings);
        screenfbo.BindTexture(screenImage.ID);

        std::string smaaPath = "F:\\PREDATOR\\GitHub\\antialiasing\\Shaders\\smaa\\";
        //edgeShader = Shader(&test_vs,&test_ps);
        edgeShader = Shader(&smaa_edge_vs,&smaa_edge_ps,"SMAA", &smaaPath);
        blendShader = Shader(&smaa_blend_vs,&smaa_blend_ps, "SMAA", &smaaPath);
        neighbourhoodShader = Shader(&smaa_neighborhood_vs,&smaa_neighborhood_ps, "SMAA", &smaaPath);
    }

    void use3(unsigned int msFramebufferImageID)
    {
        // Using Multi-sampled textures (Not working atm)
        glDisable(GL_DEPTH_TEST);

        //First Pass
        edgefbo.BindFrameBuffer();
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        edgeShader.use();
        edgeShader.setInt("albedo_tex", 0);
        quad.BindTexture(GL_TEXTURE0, msFramebufferImageID);
        quad.Draw();

        //Second Pass
        blendfbo.BindFrameBuffer();
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        blendShader.use();
        blendShader.setInt("edge_tex", 0);
        blendShader.setInt("area_tex", 1);
        blendShader.setInt("search_tex", 2);
        quad.BindTexture(GL_TEXTURE0, edgeImage.ID);
        quad.BindTexture(GL_TEXTURE1, areaMap.map.ID);
        quad.BindTexture(GL_TEXTURE2, searchMap.map.ID);
        quad.Draw();

        //Third Pass
        screenfbo.BindFrameBuffer();
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        neighbourhoodShader.use();
        neighbourhoodShader.setInt("albedo_tex", 0);
        neighbourhoodShader.setInt("blend_tex", 1);
        quad.BindTexture(GL_TEXTURE0, framebufferImageID);
        quad.BindTexture(GL_TEXTURE1, blendImage.ID);
        quad.Draw();
    }

    void use2()
    {
        glDisable(GL_DEPTH_TEST);

        //First Pass
        edgefbo.BindFrameBuffer();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glClearColor(0.f, 0.f, 0.f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        edgeShader.use();
        edgeShader.setInt("albedo_tex", 0);
        quad.BindTexture(GL_TEXTURE0, framebufferImageID);
        quad.Draw();

        //Second Pass
        blendfbo.BindFrameBuffer();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        blendShader.use();
        blendShader.setInt("edge_tex", 0);
        blendShader.setInt("area_tex", 1);
        blendShader.setInt("search_tex", 2);
        quad.BindTexture(GL_TEXTURE0, edgeImage.ID);
        quad.BindTexture(GL_TEXTURE1, areaMap.map.ID);
        quad.BindTexture(GL_TEXTURE2, searchMap.map.ID);
        quad.Draw();

        //Third Pass
        screenfbo.BindFrameBuffer();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        neighbourhoodShader.use();
        neighbourhoodShader.setInt("albedo_tex", 0);
        neighbourhoodShader.setInt("blend_tex", 1);
        quad.BindTexture(GL_TEXTURE0, framebufferImageID);
        quad.BindTexture(GL_TEXTURE1, blendImage.ID);
        quad.Draw();
    }

    void use()
    {
        glDisable(GL_DEPTH_TEST);

        //First Pass
        edgefbo.BindFrameBuffer();
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        smaa1Shader.use();
        smaa1Shader.setVec2("resolution", screenSize);
        smaa1Shader.setInt("colorTex", 0);
        quad.BindTexture(GL_TEXTURE0, framebufferImageID);
        quad.Draw();

        //Second Pass
        blendfbo.BindFrameBuffer();
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        smaa2Shader.use();
        smaa2Shader.setVec2("resolution", screenSize);
        smaa2Shader.setInt("edgesTex", 0);
        smaa2Shader.setInt("areaTex", 1);
        smaa2Shader.setInt("searchTex", 2);
        quad.BindTexture(GL_TEXTURE0, edgeImage.ID);
        quad.BindTexture(GL_TEXTURE1, areaMap.map.ID);
        quad.BindTexture(GL_TEXTURE2, searchMap.map.ID);
        quad.Draw();

        //Third Pass
        screenfbo.BindFrameBuffer();
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        smaa3Shader.use();
        smaa3Shader.setVec2("resolution", screenSize);
        smaa3Shader.setInt("colorTex", 0);
        smaa3Shader.setInt("blendTex", 1);
        quad.BindTexture(GL_TEXTURE0, framebufferImageID);
        quad.BindTexture(GL_TEXTURE1, blendImage.ID);
        quad.Draw();
    }
};
#endif //SHADER_SMAA_H
