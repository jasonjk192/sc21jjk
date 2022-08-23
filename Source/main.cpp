#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <varsettings.h>
#include <shaderLoader.h>
#include <camera.h>
#include <texture.h>
#include <model.h>
#include <window.h>
#include <buffer.h>
#include <screenQuad.h>
#include <utilityMap.h>
#include <shader_mlaa.h>
#include <shader_smaa.h>
#include <shader_fxaa.h>

#include <iostream>
#include <filesystem>

//Enables NVIDIA's GPU
extern "C" { __declspec(dllexport) unsigned long NvOptimusEnablement = useDedicatedGPU?0x00000001:0; }

float modelScale = 1.f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void window_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 16);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Antialiasing", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    //Initialize some variables
    initializeDearImGui(window);
    menuBarOptions.window = window;
    menuBarOptions.lightingType = LIGHTING_TYPE::FLAT;
    menuBarOptions.lightPos = lightPos;
    menuBarOptions.lightCol = lightCol;
    menuBarOptions.camera = &camera;
    menuBarOptions.screenWidth = SCR_WIDTH;
    menuBarOptions.screenHeight = SCR_HEIGHT;
    menuBarOptions.showDiffWithMSAA = &showDiffWithMSAA;
    menuBarOptions.showDiffWithDefault = &showDiffWithDefault;
    menuBarOptions.enableWireframe = &enableWireframe;
    menuBarOptions.modelPaths = &modelPaths;
    menuBarOptions.texturePaths = &texturePaths;
    sceneOptions.width = SCR_WIDTH;
    sceneOptions.height = SCR_HEIGHT;
    sceneOptions.camera = &camera;
    sceneOptions.window = window;

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // build and compile our shader program
    Shader blinnShader(blinnVertPath.string().c_str(), blinnFragPath.string().c_str());
    Shader flatShader(flatVertPath.string().c_str(), flatFragPath.string().c_str());
    Shader pbrShader(blinnVertPath.string().c_str(), pbrFragPath.string().c_str());
    Shader defShader(defVertPath.string().c_str(), defFragPath.string().c_str());
    Shader depthShader(blinnVertPath.string().c_str(), depthFragPath.string().c_str());
    Shader ssaaShader(ssaaVertPath.string().c_str(), ssaaFragPath.string().c_str());
    Shader diffShader(defVertPath.string().c_str(), diffFragPath.string().c_str());
    Shader* currentShader = &flatShader;

    // load initial model
    menuBarOptions.modelPath = backpackModelPath;
    menuBarOptions.model = Model(backpackModelPath.string());

    ScreenQuad quad = ScreenQuad();

    // render-to-texture framebuffer
    FrameBuffer fbo;
    Texture framebufferImage = Texture(SCR_WIDTH, SCR_HEIGHT);
    fbo.BindTexture(framebufferImage.ID);
    // create a renderbuffer object for depth and stencil attachment
    RenderBuffer rbo = RenderBuffer(SCR_WIDTH, SCR_HEIGHT);
    fbo.BindRenderBufferObject(rbo.ID);
    // screen framebuffer
    FrameBuffer screenfbo;
    Texture screenImage = Texture(SCR_WIDTH, SCR_HEIGHT);
    screenfbo.BindTexture(screenImage.ID);
    // depth texture framebuffer
    FrameBuffer depthfbo;
    Texture depthImage = Texture(SCR_WIDTH, SCR_HEIGHT);
    depthfbo.BindTexture(depthImage.ID);
    // diff framebuffer
    FrameBuffer difffbo, diffmsfbo;
    Texture diffImage = Texture(SCR_WIDTH, SCR_HEIGHT);
    Texture diffmsImage = Texture(SCR_WIDTH, SCR_HEIGHT);
    difffbo.BindTexture(diffImage.ID);
    diffmsfbo.BindTexture(diffmsImage.ID);

    // multisampling framebuffer and renderbuffer (2x till 16x)
    TextureSettings msSettings;
    msSettings.isMultiSampled = true;
    std::vector<FrameBuffer> msfbo;
    std::vector<Texture> mstex;
    std::vector<RenderBufferMultisample> msrbo;
    for(int i=0;i<msaaStrength.size();i++)
    {
        msSettings.texsamples = msaaStrength[i];
        msfbo.push_back(FrameBuffer());
        mstex.push_back(Texture(SCR_WIDTH, SCR_HEIGHT, &msSettings));
        msrbo.push_back(RenderBufferMultisample(SCR_WIDTH, SCR_HEIGHT, msaaStrength[i]));
        msfbo[i].BindTextureMultisample(mstex[i].ID);
        msfbo[i].BindRenderBufferObject(msrbo[i].ID);
    }

    // specific ssaa framebuffer and renderbuffer
    int ssLevel = 4;
    FrameBuffer ssfbo;
    Texture ssImage = Texture(SCR_WIDTH * ssLevel, SCR_HEIGHT * ssLevel);
    ssfbo.BindTexture(ssImage.ID);
    RenderBuffer ssrbo = RenderBuffer(SCR_WIDTH * ssLevel, SCR_HEIGHT * ssLevel);
    ssfbo.BindRenderBufferObject(ssrbo.ID);

    // Enable only when using the NVIDIA Extension
    FrameBufferEXT csfbo;
    Texture csImage = Texture(SCR_WIDTH, SCR_HEIGHT);
    csfbo.BindTexture(csImage.ID);
    if(useDedicatedGPU)
    {
        RenderBufferMultisampleCoverageNV csrbo(SCR_WIDTH, SCR_HEIGHT, 3);
        csfbo.BindRenderBufferObject(csrbo.ID, GL_DEPTH_ATTACHMENT);
    }

    UtilityMap am_smaa(UtilityMap::MapType::areaMapSMAA), sm_smaa(UtilityMap::MapType::searchMapSMAA);

    ShaderMLAA shaderMlaa(framebufferImage.ID, depthImage.ID);
    ShaderSMAA shaderSmaa(framebufferImage.ID, depthImage.ID);
    ShaderFXAA shaderFxaa(framebufferImage.ID, depthImage.ID);
    menuBarOptions.shaderSmaa = &shaderSmaa;
    menuBarOptions.shaderMlaa = &shaderMlaa;

    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";

    sceneOptions.textureID = screenImage.ID;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    int msaaindex = 0;

    // Initialize queries for elapsed times
    GLint64 sceneElapsedTime, screenElapsedTime;
    GLuint timeElapsedQuery;
    glGenQueries(1, &timeElapsedQuery);

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // sampling based (spatial) shaders
        fbo.BindFrameBuffer();
        glEnable(GL_BLEND);
        if(aaType == AAType::MSAA)
        {
            msfbo[msaaindex].BindFrameBuffer();
            glEnable(GL_MULTISAMPLE);
        }
        else
            glDisable(GL_MULTISAMPLE);
        if(aaType == AAType::SSAA)
        {
            ssfbo.BindFrameBuffer();
            glViewport(0, 0, ssImage.width, ssImage.height);
        }
        else
            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        if(aaType == AAType::CSAA)
        {
            csfbo.BindFrameBuffer();
            glEnable(GL_MULTISAMPLE);
        }
        else
        {
            glDisable(GL_MULTISAMPLE);
        }

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // model/view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspectRatio, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(modelScale);

        // setting camera and lighting values
        currentShader->use();
        currentShader->setMat4("projection", projection);
        currentShader->setMat4("view", view);
        currentShader->setMat4("model", model);
        switch (menuBarOptions.lightingType)
        {
            case LIGHTING_TYPE::BLINN:
                currentShader = &blinnShader;
                blinnShader.setVec3("lightColor", menuBarOptions.lightCol);
                blinnShader.setVec3("lightPos", menuBarOptions.lightPos);
                blinnShader.setVec3("viewPos", camera.Position);
                break;
            case LIGHTING_TYPE::PBR:
                currentShader = &pbrShader;
                pbrShader.setVec3("lightColor", menuBarOptions.lightCol);
                pbrShader.setVec3("lightPos", menuBarOptions.lightPos);
                pbrShader.setVec3("viewPos", camera.Position);
                break;
            default:
                currentShader = &flatShader;
        }
        if(enableWireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        glBeginQuery(GL_TIME_ELAPSED, timeElapsedQuery);
        menuBarOptions.model.Draw(*currentShader);
        glEndQuery(GL_TIME_ELAPSED);
        unsigned int isQueryDone = 0;
        while (!isQueryDone)
            glGetQueryObjectuiv(timeElapsedQuery, GL_QUERY_RESULT_AVAILABLE, &isQueryDone);
        glGetQueryObjecti64v(timeElapsedQuery, GL_QUERY_RESULT, &sceneElapsedTime);
        statsOptions.elapsedSceneDrawTime = sceneElapsedTime;

        // Prefarably should use MRT to draw to color attachment 1
        depthShader.use();
        depthShader.setMat4("projection", projection);
        depthShader.setMat4("view", view);
        depthShader.setMat4("model", model);
        depthfbo.BindFrameBuffer();
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        menuBarOptions.model.Draw(depthShader);
        glDisable(GL_CULL_FACE);

        // Prepare and draw to MSAA's buffer for comparison if enabled
        if(showDiffWithMSAA)
        {
            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
            msfbo[msaaindex].BindFrameBuffer();
            glEnable(GL_MULTISAMPLE);
            glEnable(GL_DEPTH_TEST);
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            currentShader->use();
            menuBarOptions.model.Draw(*currentShader);

            diffmsfbo.BindFrameBuffer();
            glDisable(GL_DEPTH_TEST);
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, msfbo[msaaindex].ID);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, diffmsfbo.ID);
            glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
            glDisable(GL_MULTISAMPLE);
        }

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // bind screen framebuffer and draw a quad with intermediate framebuffer's texture
        glBeginQuery(GL_TIME_ELAPSED, timeElapsedQuery);
        screenfbo.BindFrameBuffer();
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        defShader.use();
        defShader.setInt("screenTexture", 0);
        defShader.setInt("depthTexture", 1);
        quad.BindTexture(GL_TEXTURE0, framebufferImage.ID);
        quad.BindTexture(GL_TEXTURE1, depthImage.ID);

        // Screen space based (post-processing) shaders
        if(aaType == AAType::MLAA)
        {
            shaderMlaa.use();
            screenfbo.BindFrameBuffer();
            defShader.use();
            defShader.setInt("screenTexture", 0);
            defShader.setInt("depthTexture", 1);
            quad.BindTexture(GL_TEXTURE0, shaderMlaa.screenImage.ID);
            quad.BindTexture(GL_TEXTURE1, depthImage.ID);
        }
        if(aaType == AAType::SMAA)
        {
            shaderSmaa.use2();
            screenfbo.BindFrameBuffer();
            defShader.use();
            defShader.setInt("screenTexture", 0);
            defShader.setInt("depthTexture", 1);
            quad.BindTexture(GL_TEXTURE0, shaderSmaa.screenImage.ID);
            quad.BindTexture(GL_TEXTURE1, depthImage.ID);
        }
        if(aaType == AAType::FXAA)
        {
            shaderFxaa.use();
            screenfbo.BindFrameBuffer();
            defShader.use();
            defShader.setInt("screenTexture", 0);
            defShader.setInt("depthTexture", 1);
            quad.BindTexture(GL_TEXTURE0, shaderFxaa.screenImage.ID);
            quad.BindTexture(GL_TEXTURE1, depthImage.ID);
        }
        if(aaType == AAType::SSAA)
        {
            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
            ssaaShader.use();
            screenfbo.BindFrameBuffer();
            ssaaShader.setInt("texture0", 0);
            ssaaShader.setInt("SSLevel", ssLevel);
            ssaaShader.setVec2("resolution", glm::vec2(ssImage.width,ssImage.height));
            quad.BindTexture(GL_TEXTURE0, ssImage.ID);
        }
        if(aaType == AAType::CMAA)
        {
            screenfbo.BindFrameBuffer();
            defShader.use();
            defShader.setInt("screenTexture", 0);
            defShader.setInt("depthTexture", 1);
            quad.BindTexture(GL_TEXTURE0, framebufferImage.ID);
            quad.BindTexture(GL_TEXTURE1, depthImage.ID);
        }
        quad.Draw();
        glEndQuery(GL_TIME_ELAPSED);
        isQueryDone = 0;
        while (!isQueryDone)
            glGetQueryObjectuiv(timeElapsedQuery, GL_QUERY_RESULT_AVAILABLE, &isQueryDone);
        glGetQueryObjecti64v(timeElapsedQuery, GL_QUERY_RESULT, &screenElapsedTime);
        statsOptions.elapsedShaderDrawTime = screenElapsedTime;
        if(aaType == AAType::MSAA)
        {
            glDisable(GL_DEPTH_TEST);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, msfbo[msaaindex].ID);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screenfbo.ID);
            glBeginQuery(GL_TIME_ELAPSED, timeElapsedQuery);
            glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
            glEndQuery(GL_TIME_ELAPSED);
            isQueryDone = 0;
            while (!isQueryDone)
                glGetQueryObjectuiv(timeElapsedQuery, GL_QUERY_RESULT_AVAILABLE, &isQueryDone);
            glGetQueryObjecti64v(timeElapsedQuery, GL_QUERY_RESULT, &screenElapsedTime);
            statsOptions.elapsedShaderDrawTime = screenElapsedTime;
            glDisable(GL_MULTISAMPLE);
        }
        if(aaType == AAType::CSAA)
        {
            glDisable(GL_DEPTH_TEST);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, csfbo.ID);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screenfbo.ID);
            glBeginQuery(GL_TIME_ELAPSED, timeElapsedQuery);
            glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
            glEndQuery(GL_TIME_ELAPSED);
            isQueryDone = 0;
            while (!isQueryDone)
                glGetQueryObjectuiv(timeElapsedQuery, GL_QUERY_RESULT_AVAILABLE, &isQueryDone);
            glGetQueryObjecti64v(timeElapsedQuery, GL_QUERY_RESULT, &screenElapsedTime);
            statsOptions.elapsedShaderDrawTime = screenElapsedTime;
            glDisable(GL_MULTISAMPLE);
        }
        if(aaType == AAType::CMAA)
        {
            screenfbo.BindFrameBuffer();
            if(!useDedicatedGPU)
            {
                glBeginQuery(GL_TIME_ELAPSED, timeElapsedQuery);
                glApplyFramebufferAttachmentCMAAINTEL();
                glEndQuery(GL_TIME_ELAPSED);
                isQueryDone = 0;
                while (!isQueryDone)
                    glGetQueryObjectuiv(timeElapsedQuery, GL_QUERY_RESULT_AVAILABLE, &isQueryDone);
                glGetQueryObjecti64v(timeElapsedQuery, GL_QUERY_RESULT, &screenElapsedTime);
                statsOptions.elapsedShaderDrawTime = screenElapsedTime;
            }

        }

        menuBarOptions.framebuffer = screenfbo.ID;

        if(showDiffWithDefault)
        {
            diffShader.use();
            difffbo.BindFrameBuffer();
            glDisable(GL_DEPTH_TEST);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            diffShader.setInt("defaultTexture", 0);
            diffShader.setInt("shadedTexture", 1);
            quad.BindTexture(GL_TEXTURE0, framebufferImage.ID);
            quad.BindTexture(GL_TEXTURE1, screenImage.ID);
            quad.Draw();
            glBindFramebuffer(GL_READ_FRAMEBUFFER, difffbo.ID);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screenfbo.ID);
            glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }

        if(showDiffWithMSAA)
        {
            diffShader.use();
            difffbo.BindFrameBuffer();
            glDisable(GL_DEPTH_TEST);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            diffShader.setInt("defaultTexture", 0);
            diffShader.setInt("shadedTexture", 1);
            quad.BindTexture(GL_TEXTURE0, diffmsImage.ID);
            quad.BindTexture(GL_TEXTURE1, screenImage.ID);
            quad.Draw();
            glBindFramebuffer(GL_READ_FRAMEBUFFER, difffbo.ID);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screenfbo.ID);
            glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }

        // switch back to default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        menuBarOptions.aaName = shaderNames[aaIndex];
        // ImGui frame
        composeDearImGuiFrame();
        showMenuBar(menuBarOptions);
        ImGui::SetNextWindowPos(ImVec2(0,menuBarOptions.menuBarSize.y));
        ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH,WINDOW_HEIGHT));
        showScene(sceneOptions, statsOptions);
        showStats(statsOptions);

        std::string msaaLabel = std::string("MSAA Samples: ") + std::to_string(msaaStrength[msaaindex]);

        ImGui::SliderInt(shaderNames[aaIndex].c_str(), &aaIndex, 0, shaderNames.size()-1);
        ImGui::SliderInt(msaaLabel.c_str(),&msaaindex,0,msaaStrength.size()-1);
        ImGui::DragFloat("Scale",&modelScale,0.001,0, 2);
        ImGui::Text("Recording: %s", recordElapsedTime?"TRUE":"FALSE");
        ImGui::Text("Frames To Record: %d", framesToRecord);
        ImGui::Text("Current Elapsed Frame To Record: %d", currentElapsedFrameToRecord);
        ImGui::Render();
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        switch (aaIndex) {
            case 1: aaType = AAType::MLAA; break;
            case 2: aaType = AAType::SMAA; break;
            case 3: aaType = AAType::MSAA; break;
            case 4: aaType = AAType::FXAA; break;
            case 5: aaType = AAType::SSAA; break;
            case 6: aaType = AAType::CSAA; break;
            case 7: aaType = AAType::CMAA; break;
            default: aaType = AAType::Default; break;
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();

        // record the elapsed times for current frame
        if(recordElapsedTime && (currentElapsedFrameToRecord<framesToRecord || framesToRecord == -1))
        {
            sceneTime.push_back(statsOptions.elapsedSceneDrawTime);
            shaderTime.push_back(statsOptions.elapsedShaderDrawTime);
            currentElapsedFrameToRecord++;
        }
    }

    // close everything
    glDeleteFramebuffers(1, &fbo.ID);
    glDeleteFramebuffers(1, &screenfbo.ID);
    closeImGui();
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        std::string gpuType = useDedicatedGPU?"D":"I";
        std::string fileNameCSV = "Assets\\CSV\\sceneTime (" + shaderNames[aaIndex] + gpuType + ").csv";
        recordElapsedTime = false;
        menuBarOptions.elapsedTime = &sceneTime;
        saveDataToCSV(menuBarOptions, fileNameCSV);
        fileNameCSV = "Assets\\CSV\\shaderTime (" + shaderNames[aaIndex] + gpuType + ").csv";
        menuBarOptions.elapsedTime = &shaderTime;
        saveDataToCSV(menuBarOptions, fileNameCSV);
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        recordElapsedTime = true;
        currentElapsedFrameToRecord = 0;
        sceneTime.clear();
        shaderTime.clear();
    }

    if(camera.enableCameraMovement)
    {
        if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS)
            camera.MovementSpeed = camera.MovementSpeed+0.05f;
        if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS)
            camera.MovementSpeed = camera.MovementSpeed>1.5f?camera.MovementSpeed-0.05f:camera.MovementSpeed;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            camera.ProcessKeyboard(UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            camera.ProcessKeyboard(DOWN, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            camera.enableCameraMovement = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

    }
    if(glfwGetKey(window,GLFW_KEY_LEFT_CONTROL)== GLFW_PRESS && glfwGetKey(window, GLFW_KEY_S)== GLFW_PRESS)
    {
        saveScreenshot(menuBarOptions);
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
    WINDOW_WIDTH = height * aspectRatio;
    WINDOW_HEIGHT = height;
    sceneOptions.width = WINDOW_WIDTH;
    sceneOptions.height = WINDOW_HEIGHT;
}

// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = lastX - xpos ;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if(camera.enableCameraMovement)
    {
        camera.ProcessMouseMovement(xoffset, yoffset);
    }

}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if(camera.enableCameraMovement)
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{

}