#ifndef WINDOW_H
#define WINDOW_H

// C++
#include <string>
#include <iostream>
#include <filesystem>
#include <ctime>

// GLFW
#include <GLFW/glfw3.h>

// Dear ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>

#include <model.h>
#include <shader_mlaa.h>
#include <shader_smaa.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

enum class LIGHTING_TYPE{FLAT, BLINN, PBR};

struct MenuBarOptions
{
    std::filesystem::path modelPath;
    Model model;
    ImVec2 menuBarSize;
    int screenWidth, screenHeight;
    LIGHTING_TYPE lightingType;
    glm::vec3 lightPos;
    glm::vec3 lightCol;
    Camera* camera;
    GLFWwindow* window;
    unsigned int framebuffer;

    // Allows control over parameters for shaders (Disabled for now and uses recommended max settings instead)
    ShaderMLAA* shaderMlaa;
    ShaderSMAA* shaderSmaa;

    bool* showDiffWithDefault;
    bool* showDiffWithMSAA;
    bool* enableWireframe;
    std::vector<std::filesystem::path>* modelPaths;
    std::vector<std::filesystem::path>* texturePaths;
    std::vector<long>* elapsedTime;
    std::string aaName;
};

struct SceneOptions
{
    int width, height;
    unsigned int textureID;
    Camera* camera;
    GLFWwindow* window;
};

struct StatsOptions
{
    ImVec2 mousePos;
    GLubyte pixelColor[4];
    GLint64 elapsedSceneDrawTime;
    GLint64 elapsedShaderDrawTime;
};

std::filesystem::path currentPath = ".";
std::filesystem::path basePath = ".";
bool enableCamHUD = false;
bool enableLightHUD = false;
bool enableLoadModel = false;
bool enableMLAASettings = false;
bool enableSMAASettings = false;
bool closeWindow = false;
int currentLoadModel = -1;

void getPixel(int x, int y, GLubyte *color)
{
    glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, color);
}

std::string timeToString(std::chrono::system_clock::time_point timePoint)
{
    time_t t = std::chrono::system_clock::to_time_t(timePoint);
    char str[26];
    ctime_s(str, sizeof str, &t);
    return str;
}

void closeImGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}

bool initializeDearImGui(GLFWwindow* glfWwindow)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    if (!ImGui_ImplGlfw_InitForOpenGL(glfWwindow, true)) { return false; }
    if (!ImGui_ImplOpenGL3_Init()) { return false; }
    return true;
}

void composeDearImGuiFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void saveDataToCSV(MenuBarOptions& options, std::string fileName)
{
    std::ofstream outCSVFile;
    outCSVFile.open (fileName);
    for(int i=0;i<options.elapsedTime->size();i++)
        outCSVFile << options.elapsedTime->at(i) << "\n";
    outCSVFile.close();
}

void saveBufferToImage(MenuBarOptions& options, std::string fileName)
{
    GLubyte *data = (GLubyte*)malloc(3 * options.screenWidth * options.screenHeight);
    if(data)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, options.framebuffer);
        glReadBuffer(GL_FRONT);
        glReadPixels(0, 0, options.screenWidth, options.screenHeight, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_flip_vertically_on_write(true);
        stbi_write_png(fileName.c_str(), options.screenWidth, options.screenHeight, 3, data, options.screenWidth*3);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void saveScreenshot(MenuBarOptions& options)
{
    GLubyte *data = (GLubyte*)malloc(3 * options.screenWidth * options.screenHeight);
    if(data)
    {
        std::string screenshotFileName = "Assets\\Screenshots\\screenshot ("+options.aaName+").png";
        glBindFramebuffer(GL_FRAMEBUFFER, options.framebuffer);
        glReadBuffer(GL_FRONT);
        glReadPixels(0, 0, options.screenWidth, options.screenHeight, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_flip_vertically_on_write(true);
        stbi_write_png(screenshotFileName.c_str(), options.screenWidth, options.screenHeight, 3, data, options.screenWidth*3);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void showMLAASettings(ShaderMLAA* shaderMlaa)
{
    ImGui::Begin("MLAA Settings", NULL, ImGuiWindowFlags_NoDocking);
    ImGui::DragFloat("fDepthThreshold",&shaderMlaa->fDepthThreshold,0,2);
    ImGui::DragFloat("fLuminanceThreshold",&shaderMlaa->fLuminanceThreshold,0,2);
    ImGui::DragInt("maxDistance",&shaderMlaa->maxDistance,0,100);
    ImGui::DragInt("maxSearchSteps",&shaderMlaa->maxSearchSteps,0,100);
    ImGui::End();
}

void showSMAASettings(ShaderSMAA* shaderSmaa)
{
    ImGui::Begin("SMAA Settings", NULL, ImGuiWindowFlags_NoDocking);
    ImGui::DragFloat("fDepthThreshold",&shaderSmaa->fDepthThreshold,0.001f,0,2);
    ImGui::DragFloat("fLuminanceThreshold",&shaderSmaa->fLuminanceThreshold,0.001f,0,2);
    ImGui::DragFloat("smaaThreshold",&shaderSmaa->smaaThreshold,0.001f,0,2);
    ImGui::DragFloat("weightStrength",&shaderSmaa->weightStrength,0.001f,1,20);
    ImGui::DragInt("maxDistance",&shaderSmaa->maxDistance,1,0,100);
    ImGui::DragInt("maxSearchSteps",&shaderSmaa->maxSearchSteps,1,0,100);
    ImGui::DragInt("maxSearchStepsDiag",&shaderSmaa->maxSearchStepsDiag,1,0,100);
    ImGui::DragInt("cornerRounding",&shaderSmaa->cornerRounding,1,0,100);
    ImGui::End();
}

void showCameraHUD(Camera* camera)
{
    ImGui::Begin("Camera HUD", NULL, ImGuiWindowFlags_NoBackground|ImGuiWindowFlags_NoDocking|ImGuiWindowFlags_NoTitleBar);
    ImGui::Text("Enable movement: %s", camera->enableCameraMovement?"True":"False");
    ImGui::Text("Position: %.3f, %.3f, %.3f", camera->Position.x, camera->Position.y, camera->Position.z);
    ImGui::Text("Speed: %.3f", camera->MovementSpeed);
    ImGui::Text("Sensitivity: %.3f", camera->MouseSensitivity);
    ImGui::Text("Zoom: %.3f", camera->Zoom);
    ImGui::Text("Pitch: %.3f", camera->Pitch);
    ImGui::Text("Yaw: %.3f", camera->Yaw);
    ImGui::End();
}

void showLightHUD(MenuBarOptions* options)
{
    static float col1[3] = {options->lightCol.r,options->lightCol.g,options->lightCol.b};
    ImGui::Begin("Light HUD", NULL, ImGuiWindowFlags_NoBackground|ImGuiWindowFlags_NoDocking|ImGuiWindowFlags_NoTitleBar);
    ImGui::ColorEdit3("Light Color", col1);
    ImGui::DragFloat("x",&options->lightPos.x);
    ImGui::DragFloat("y",&options->lightPos.y);
    ImGui::DragFloat("z",&options->lightPos.z);
    ImGui::End();
    options->lightCol = glm::vec3(col1[0],col1[1],col1[2]);
}

void showMenuBar(MenuBarOptions& options)
{
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("Options"))
    {
        ImGui::MenuItem("Load Model",NULL, &enableLoadModel);
        ImGui::MenuItem("Wireframe Mode",NULL, options.enableWireframe);

        if(ImGui::MenuItem("Save Screenshot","CTRL+S"))
        {
            saveScreenshot(options);
        }
        ImGui::MenuItem("Exit", "Esc", &closeWindow);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Lighting"))
    {
        if(ImGui::MenuItem("Flat")) options.lightingType = LIGHTING_TYPE::FLAT;
        if(ImGui::MenuItem("Blinn")) options.lightingType = LIGHTING_TYPE::BLINN;
        if(ImGui::MenuItem("PBR")) options.lightingType = LIGHTING_TYPE::PBR;
        ImGui::Separator();
        ImGui::MenuItem("HUD",NULL,&enableLightHUD);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Camera"))
    {
        if(ImGui::MenuItem("Save"))
        {
            options.camera->SaveCameraSettings();
        }
        if(ImGui::MenuItem("Load"))
        {
            options.camera->LoadCameraSettings();
        }
        if(ImGui::MenuItem("Reset"))
        {
            options.camera->Reset();
        }
        ImGui::MenuItem("HUD",NULL,&enableCamHUD);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("AA Settings"))
    {
        ImGui::MenuItem("MLAA",NULL,&enableMLAASettings);
        ImGui::MenuItem("SMAA",NULL,&enableSMAASettings);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Diff Settings"))
    {
        ImGui::MenuItem("Default",NULL,options.showDiffWithDefault);
        ImGui::MenuItem("MSAA",NULL,options.showDiffWithMSAA);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Models"))
    {
        for(int i=0;i<options.modelPaths->size();i++)
        {
            if(ImGui::MenuItem(options.modelPaths->at(i).filename().string().c_str()))
            {
                currentLoadModel = i;
                enableLoadModel = true;
            }
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Textures"))
    {
        for(int i=0;i<options.texturePaths->size();i++)
        {
            if(ImGui::MenuItem(options.texturePaths->at(i).filename().string().c_str()))
            {
                TextureSettings nts;
                nts.texwrap = GL_REPEAT;
                nts.texfilter = GL_NEAREST;
                options.model.meshes.at(0).textures.at(0) = Texture(options.texturePaths->at(i),false,&nts);
                options.model.meshes.at(0).textures.at(0).type = "texture_diffuse";
            }
        }
        ImGui::EndMenu();
    }

    options.menuBarSize = ImGui::GetWindowSize();
    ImGui::EndMainMenuBar();

    if(enableLoadModel)
    {
        if(currentLoadModel>=0)
        {
            options.modelPath = options.modelPaths->at(currentLoadModel);
            if(exists(options.modelPath))
                options.model = Model(options.modelPath.string());
            currentLoadModel = -1;
            enableLoadModel = false;
        }
        else
        {
            static char str0[512] = "backpack\\backpack.obj";
            ImGui::Begin("Load Model");
            ImGui::InputText("Model", str0, IM_ARRAYSIZE(str0));
            if(ImGui::Button("Load"))
            {
                options.modelPath = std::filesystem::path(str0);
                if(exists(options.modelPath))
                    options.model = Model(options.modelPath.string());
                else
                    std::cerr<<"Path doesnt exist\n";
                enableLoadModel = false;
            }
            ImGui::SameLine();
            if(ImGui::Button("Close"))
            {
                enableLoadModel = false;
            }
            ImGui::End();
        }

    }
    if(closeWindow)
    {
        glfwSetWindowShouldClose(options.window, true);
    }

    if(enableCamHUD)
        showCameraHUD(options.camera);
    if(enableLightHUD)
        showLightHUD(&options);
    if(enableMLAASettings)
        showMLAASettings(options.shaderMlaa);
    if(enableSMAASettings)
        showSMAASettings(options.shaderSmaa);
}

void showScene(SceneOptions& options, StatsOptions& statsOptions)
{
    ImGui::Begin("Scene",NULL,ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse|ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoTitleBar);
    ImVec2 wsize = ImVec2(options.width, options.height);
    ImGui::Image((void*)(intptr_t) options.textureID, wsize, ImVec2(0, 1), ImVec2(1, 0));
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsItemHovered())
    {
        if(!options.camera->enableCameraMovement)
        {
            options.camera->enableCameraMovement = true;
            glfwSetInputMode(options.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else
        {
            options.camera->enableCameraMovement = false;
            glfwSetInputMode(options.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    if(ImGui::IsItemHovered())
    {
        statsOptions.mousePos = ImGui::GetMousePos();
        getPixel(statsOptions.mousePos.x, options.height - 1 - statsOptions.mousePos.y, statsOptions.pixelColor);
    }
    ImGui::End();
}

void showStats(StatsOptions& statsOptions)
{
    ImGui::Begin("Stats", NULL, ImGuiWindowFlags_NoBackground|ImGuiWindowFlags_NoDocking|ImGuiWindowFlags_NoTitleBar);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Text("Time: %s", timeToString(std::chrono::system_clock::now()).c_str());

    // Opengl's GPU times are in nanoseconds
    ImGui::Text("Scene Draw Time: %f us", (statsOptions.elapsedSceneDrawTime/1000.f));
    ImGui::Text("Shader Draw Time: %f us", (statsOptions.elapsedShaderDrawTime/1000.f));
    float pixelCol[3] = {(float)statsOptions.pixelColor[0]/255.f,(float)statsOptions.pixelColor[1]/255.f,(float)statsOptions.pixelColor[2]/255.f};
    ImGui::ColorEdit3("Pixel Color", pixelCol, ImGuiColorEditFlags_NoInputs);
    ImGui::Text("Pixel Value: %d, %d, %d", statsOptions.pixelColor[0], statsOptions.pixelColor[1], statsOptions.pixelColor[2]);
    ImGui::Text("Mouse Pos: %.0f, %.0f", statsOptions.mousePos.x, statsOptions.mousePos.y);
    ImGui::End();
}

#endif
