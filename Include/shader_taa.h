// https://research.nvidia.com/sites/default/files/pubs/2018-08_Adaptive-Temporal-Antialiasing/adaptive-temporal-antialiasing-preprint.pdf
// https://www.nvidia.com/content/dam/en-zz/Solutions/events/siggraph2018/pdf/tuesday/sig1824-adam-marrs-rahul-sathe-adaptive-temporal-antialiasing.pdf
// http://behindthepixels.io/assets/files/TemporalAA.pdf

#ifndef SHADER_TAA_H
#define SHADER_TAA_H

#include <shaderLoader.h>
#include <texture.h>
#include <screenQuad.h>
#include <buffer.h>
#include <model.h>

#include <iostream>
#include <filesystem>

class ShaderTAA
{
public:
    struct defaultSettings_t
    {
        glm::mat4 projection;
        glm::mat4 view;
        glm::mat4 translation;
        glm::vec2 resolution;
        glm::vec2 mousePosition;
        float deltaTime;
        float totalTime;
        float framesPerSecond;
        int	totalFrames;

        defaultSettings_t()
        {
            projection = glm::mat4(1);
            view = glm::mat4(1);
            translation = glm::mat4(1);
            resolution = glm::vec2(1280,720);
        }
    };

    struct jitterSettings_t
    {
        glm::vec2			haltonSequence[128];
        float				haltonScale;
        int					haltonIndex;
        int					enableDithering;
        float				ditheringScale;

        jitterSettings_t()
        {
            haltonIndex = 16;
            enableDithering = 1;
            haltonScale = 100.0f;
            ditheringScale = 0.0f;
        }

        ~jitterSettings_t() {};
    };

    struct reprojectSettings_t
    {
        glm::mat4		previousProjection;
        glm::mat4		previousView;
        glm::mat4		prevTranslation;

        glm::mat4		currentView;

        reprojectSettings_t()
        {
            this->previousProjection = glm::mat4(1.0f);
            this->previousView = glm::mat4(1.0f);
            this->prevTranslation = glm::mat4(1.0f);

            this->currentView = glm::mat4(1.0f);
        }

        ~reprojectSettings_t() {};
    };

    struct TAASettings_t
    {
        //velocity
        float velocityScale;
        //Inside
        float feedbackFactor;
        //Custom
        float maxDepthFalloff;

        TAASettings_t()
        {
            this->feedbackFactor = 0.9f;
            this->maxDepthFalloff = 1.0f;
            this->velocityScale = 1.0f;
        }

        ~TAASettings_t() { };
    };

    struct sharpenSettings_t
    {
        GLfloat			kernel1;
        GLfloat			kernel2;

        sharpenSettings_t(
                GLfloat kernel1 = -0.125f, GLfloat kernel2 = 1.75f)
        {
            this->kernel1 = kernel1;
            this->kernel2 = kernel2;
        }

        ~sharpenSettings_t() { };
    };
private:
    std::filesystem::path taaShaderPath = std::filesystem::current_path().append("Shaders\\taa");
    std::filesystem::path taaPass1FragPath = absolute(taaShaderPath).append("taa1.frag");
    std::filesystem::path taaPass2FragPath = absolute(taaShaderPath).append("taa2.frag");
    std::filesystem::path taaPass3FragPath = absolute(taaShaderPath).append("taa3.frag");
    std::filesystem::path taaPass1VertPath = absolute(taaShaderPath).append("taa1.vert");
    std::filesystem::path taaPass2VertPath = absolute(taaShaderPath).append("taa2.vert");
    std::filesystem::path taaPass3VertPath = absolute(taaShaderPath).append("taa3.vert");
    glm::vec2 screenSize = glm::vec2(1280,720);
    glm::vec2 texelSize = glm::vec2(1)/screenSize;
    ScreenQuad quad;
    unsigned int framebufferImageID,depthImageID;
    Shader taa1Shader, taa2Shader, taa3Shader;

    std::vector<FrameBuffer*> historyFrames;
    FrameBuffer* geometryBuffer;
    FrameBuffer* unJitteredBuffer;

    UniformBuffer defaultUBO, velocityUBO, jitterUBO;
    defaultSettings_t defaultSettings;
    jitterSettings_t jitterSettings;
    reprojectSettings_t reprojectSettings;
    TAASettings_t taaSettings;
    sharpenSettings_t sharpenSettings;

public:
    FrameBuffer geomfbo, resolvefbo, screenfbo;
    Texture geomImage, resolveImage, screenImage;

    ShaderTAA(unsigned int framebufferImageID, unsigned int depthImageID):
            framebufferImageID(framebufferImageID), depthImageID(depthImageID)
    {
        taa1Shader = Shader(taaPass1VertPath.string().c_str(), taaPass1FragPath.string().c_str());
        taa2Shader = Shader(taaPass2VertPath.string().c_str(), taaPass2FragPath.string().c_str());
        taa3Shader = Shader(taaPass3VertPath.string().c_str(), taaPass3FragPath.string().c_str());

        geomImage = Texture(screenSize.x, screenSize.y);
        geomfbo.BindTexture(geomImage.ID);

        resolveImage = Texture(screenSize.x, screenSize.y);
        resolvefbo.BindTexture(resolveImage.ID);

        screenImage = Texture(screenSize.x, screenSize.y);
        screenfbo.BindTexture(screenImage.ID);

        InitUBO();

        bool enableCompare = true;

        bool enableSharpen = false;
    }

    void use()
    {
        //Luma calculation into alpha channel
        glDisable(GL_DEPTH_TEST);
        geomfbo.BindFrameBuffer();
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        taa1Shader.use();
        taa1Shader.setInt("albedo_tex", 0);
        quad.BindTexture(GL_TEXTURE0, framebufferImageID);
        quad.Draw();

        resolvefbo.BindFrameBuffer();
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        taa2Shader.use();
        taa2Shader.setInt("albedo_tex", 0);
        quad.BindTexture(GL_TEXTURE0, framebufferImageID);
        quad.Draw();

        //FXAA Pass
        screenfbo.BindFrameBuffer();
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        taa3Shader.use();
        taa3Shader.setInt("albedo_tex", 0);
        taa3Shader.setVec2("rcpFrame", texelSize);
        quad.BindTexture(GL_TEXTURE0, resolveImage.ID);
        quad.Draw();
    }

    void RenderScene(Model* model)
    {
        unsigned int defaultSettings_index = glGetUniformBlockIndex(taa1Shader.ID, "defaultSettings");
        glUniformBlockBinding(taa1Shader.ID, defaultSettings_index, 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, defaultUBO.ID);
        defaultUBO.BindUniformBuffer();
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(defaultSettings_t), &defaultSettings);

        unsigned int velocitySettings_index = glGetUniformBlockIndex(taa1Shader.ID, "velocitySettings");
        glUniformBlockBinding(taa1Shader.ID, velocitySettings_index, 1);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, velocityUBO.ID);
        velocityUBO.BindUniformBuffer();
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(defaultSettings_t), &reprojectSettings);

        unsigned int jitterSettings_index = glGetUniformBlockIndex(taa1Shader.ID, "jitterSettings");
        glUniformBlockBinding(taa1Shader.ID, jitterSettings_index, 4);
        glBindBufferBase(GL_UNIFORM_BUFFER, 4, jitterUBO.ID);
        jitterUBO.BindUniformBuffer();
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(defaultSettings_t), &jitterSettings);

        //model->Draw(taa1Shader);
    }

    void InitUBO()
    {
        defaultUBO.BindData(&defaultSettings, sizeof(defaultSettings_t), GL_STATIC_READ);
        velocityUBO.BindData(&reprojectSettings, sizeof(reprojectSettings_t), GL_STATIC_READ);
        jitterUBO.BindData(&jitterSettings, sizeof(jitterSettings_t), GL_STATIC_READ);
    }

};
#endif //SHADER_TAA_H
