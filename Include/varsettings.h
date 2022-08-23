#ifndef VARSETTINGS_H
#define VARSETTINGS_H

#include <filesystem>
#include <shaderLoader.h>
#include <camera.h>
#include <window.h>

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
unsigned int WINDOW_WIDTH = 1280;
unsigned int WINDOW_HEIGHT = 720;
const float aspectRatio = float(SCR_WIDTH)/float(SCR_HEIGHT);

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
glm::vec3 lightCol(1.f,1.f,1.f);

// paths
std::filesystem::path texturePath = std::filesystem::current_path().append("Assets\\Textures");
std::filesystem::path modelsPath = std::filesystem::current_path().append("Assets\\Models");
std::filesystem::path shaderPath = std::filesystem::current_path().append("Shaders");

// models
std::filesystem::path shipModelPath = absolute(modelsPath).append("NewShip.obj");
std::filesystem::path backpackModelPath = absolute(modelsPath).append("backpack\\backpack.obj");
std::filesystem::path casaModelPath = absolute(modelsPath).append("casa.obj");
std::filesystem::path minewayModelPath = absolute(modelsPath).append("Mineways2Skfb.obj");
std::filesystem::path globeModelPath = absolute(modelsPath).append("Globe 3D Model.obj");
std::filesystem::path patchofheavenModelPath = absolute(modelsPath).append("patchofheaven\\Export.obj");
std::filesystem::path catModelPath = absolute(modelsPath).append("cat\\mesh.obj");
std::filesystem::path eggyptSouvenirBoxModelPath = absolute(modelsPath).append("egypt souvenir box\\pyramid2.obj");
std::filesystem::path interiorBathroomModelPath = absolute(modelsPath).append("interior bathroom\\model.obj");
std::filesystem::path minecraftSphereModelPath = absolute(modelsPath).append("minecraft sphere\\Testing2.obj");
std::filesystem::path streetModelPath = absolute(modelsPath).append("street\\#Ulica.obj");
std::filesystem::path veniceModelPath = absolute(modelsPath).append("venice\\venedig.obj");
std::filesystem::path soupModelPath = absolute(modelsPath).append("41_chicken pot pie soup\\chickenPotPieSoup.obj");
std::filesystem::path earthModelPath = absolute(modelsPath).append("106969_open3dmodel.com\\Earth 2K.obj");
std::filesystem::path cat2ModelPath = absolute(modelsPath).append("cat\\cat.obj");
std::filesystem::path n901ModelPath = absolute(modelsPath).append("Character_A1130034\\n901.obj");
std::filesystem::path horseModelPath = absolute(modelsPath).append("Models_E0113A080\\HORSE.obj");
std::filesystem::path pietaModelPath = absolute(modelsPath).append("pieta\\Pieta_C.obj");

std::filesystem::path texQuadModelPath = absolute(modelsPath).append("texQuad.obj");

std::vector<std::filesystem::path> modelPaths = {texQuadModelPath, shipModelPath, backpackModelPath, casaModelPath, minewayModelPath, globeModelPath, patchofheavenModelPath,
                                                 veniceModelPath, streetModelPath, catModelPath, eggyptSouvenirBoxModelPath, interiorBathroomModelPath, minecraftSphereModelPath,
                                                 soupModelPath, earthModelPath, cat2ModelPath, n901ModelPath, horseModelPath, pietaModelPath};

// textures for basic quad
std::filesystem::path wallTexPath = absolute(texturePath).append("wall.jpg");
std::filesystem::path stripe1TexPath = absolute(texturePath).append("stripe1.png");
std::filesystem::path stripe2TexPath = absolute(texturePath).append("stripe2.jpg");
std::filesystem::path stripe3TexPath = absolute(texturePath).append("stripe3.png");

std::vector<std::filesystem::path> texturePaths = {wallTexPath, stripe1TexPath, stripe2TexPath, stripe3TexPath};

// shaders
std::filesystem::path depthFragPath = absolute(shaderPath).append("depth.frag");
std::filesystem::path blinnVertPath = absolute(shaderPath).append("lighting\\blinn.vert");
std::filesystem::path blinnFragPath = absolute(shaderPath).append("lighting\\blinn.frag");
std::filesystem::path flatVertPath = absolute(shaderPath).append("lighting\\flat.vert");
std::filesystem::path flatFragPath = absolute(shaderPath).append("lighting\\flat.frag");
std::filesystem::path pbrFragPath = absolute(shaderPath).append("lighting\\pbr.frag");

std::filesystem::path defVertPath = absolute(shaderPath).append("screen.vert");
std::filesystem::path defFragPath = absolute(shaderPath).append("screen.frag");
std::filesystem::path diffFragPath = absolute(shaderPath).append("diff.frag");
std::filesystem::path ssaaVertPath = absolute(shaderPath).append("ssaa.vert");
std::filesystem::path ssaaFragPath = absolute(shaderPath).append("ssaa.frag");

MenuBarOptions menuBarOptions;
SceneOptions sceneOptions;
StatsOptions statsOptions;

std::vector<std::string> shaderNames = {"Default", "MLAA", "SMAA", "MSAA", "FXAA", "SSAA", "CSAA (Discrete GPU)", "CMAA (Integrated GPU)"};
enum class AAType{Default, MLAA, SMAA, MSAA, FXAA, SSAA, CSAA, CMAA};
int aaIndex = 0;
AAType aaType = AAType::Default;

std::vector<long> shaderTime, sceneTime;
bool enableSaveCSV = false;
std::vector<int> msaaStrength = {1,2,4,8,16};
bool showDiffWithDefault = false;
bool showDiffWithMSAA = false;
bool showDiffWithSSAA = false;
bool enableWireframe = false;
bool useDedicatedGPU = true;
bool recordElapsedTime = false;
int framesToRecord = 5000;
int currentElapsedFrameToRecord = 0;

#endif //VARSETTINGS_H
