#-----------------------------
# Initialising stuff
#-----------------------------
cmake_minimum_required(VERSION 3.21)
project(Antialiasing)
if (APPLE)
	enable_language(OBJC)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
# static libraries please
set(BUILD_SHARED_LIBS FALSE)
# make executable run in project's root directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
# FetchContent to fetch git repos remotely
include(FetchContent)
# enable CTest
enable_testing()
#-----------------------------

#-----------------------------
# External libraries from git repos
#-----------------------------
# glfw
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.3.6)
FetchContent_MakeAvailable(glfw)
#-----------------------------
# Dear ImGui
FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG origin/docking)
FetchContent_MakeAvailable(imgui)
add_subdirectory("CMake Subfolders/ImGui CMake")
#-----------------------------
# ImGuiFileDialog
add_compile_definitions(USE_BOOKMARK)
FetchContent_Declare(imguifiledialog GIT_REPOSITORY https://github.com/aiekick/ImGuiFileDialog.git GIT_TAG v0.6.4)
FetchContent_MakeAvailable(imguifiledialog)
include_directories(${FETCHCONTENT_BASE_DIR}/imguifiledialog-src)
#-----------------------------
# Immediate Mode Plotting (implot)
FetchContent_Declare(implot GIT_REPOSITORY https://github.com/epezent/implot.git GIT_TAG v0.13)
FetchContent_MakeAvailable(implot)
add_subdirectory("CMake Subfolders/ImPlot CMake")
#-----------------------------
# GLM
FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 0.9.9.8)
FetchContent_MakeAvailable(glm)
add_compile_definitions(GLM_FORCE_LEFT_HANDED)
#-----------------------------
# assimp
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT FALSE)
set(ASSIMP_BUILD_OBJ_IMPORTER TRUE)
set(ASSIMP_BUILD_FBX_IMPORTER FALSE)
set(ASSIMP_BUILD_BLEND_IMPORTER FALSE)
set(ASSIMP_NO_EXPORT TRUE)
set(ASSIMP_BUILD_TESTS FALSE)
FetchContent_Declare(assimp GIT_REPOSITORY https://github.com/assimp/assimp.git GIT_TAG 5.2.0)
FetchContent_MakeAvailable(assimp)
#-----------------------------
# glew
set(glew-cmake_BUILD_SHARED OFF)
set(USE_GLU "Use GLU" ON)
FetchContent_Declare(glew GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git GIT_TAG glew-cmake-2.2.0)
FetchContent_MakeAvailable(glew)
#-----------------------------
# imguizmo
FetchContent_Declare(imguizmo GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git GIT_TAG origin/master)
FetchContent_MakeAvailable(imguizmo)
#-----------------------------
# stb_image
FetchContent_Declare(stb GIT_REPOSITORY https://github.com/nothings/stb.git GIT_TAG origin/master)
FetchContent_MakeAvailable(stb)
#-----------------------------
# GLI
FetchContent_Declare(gli GIT_REPOSITORY https://github.com/g-truc/gli.git GIT_TAG 0.8.2.0)
FetchContent_MakeAvailable(gli)
include_directories(${FETCHCONTENT_BASE_DIR}/gli-src)
#-----------------------------
# GLAD
add_subdirectory("CMake Subfolders/GLAD CMake")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/glad/include)
#-----------------------------


#-----------------------------
# IMPORTANT SECTION! Source code
#-----------------------------
# New place to put all your source code files into:
set(PROJECT_SOURCES_WITHOUT_MAIN
	# order by folder structure

	"${FETCHCONTENT_BASE_DIR}/imguifiledialog-src/ImGuiFileDialog.cpp;"
	"${FETCHCONTENT_BASE_DIR}/imguizmo-src/ImGuizmo.cpp;"
	)

add_executable(${PROJECT_NAME} Source/main.cpp ${PROJECT_SOURCES_WITHOUT_MAIN})
#-----------------------------

#-----------------------------
# IMPORTANT SECTION! Include files
#-----------------------------
# list of folders to include
set(PROJECT_INCLUDES
	"Include;"
	"${CMAKE_CURRENT_SOURCE_DIR}/Shaders;"
	"${FETCHCONTENT_BASE_DIR}/imguizmo-src;"
	"${FETCHCONTENT_BASE_DIR}/stb-src;"
	)
	
# different targets
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDES})
#-----------------------------

#-----------------------------
# Linking libraries
#-----------------------------
# list of libraries we're using in the main engine
set(PROJECT_LIBRARIES
	"glfw;"
	"imgui;"
	"implot;"
	"glm;"
	"assimp;"
	"libglew_static;"
	"glad;"
  )

# different targets
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_LIBRARIES})
#-----------------------------
