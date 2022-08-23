#ifndef UTILITYMAP_H
#define UTILITYMAP_H

#include <texture.h>

// Takes care of SMAA and MLAA resources
// Not necessary but convenient
class UtilityMap
{
public:
    Texture map;

    enum class MapType{areaMap33, areaMapSMAA, searchMapSMAA};

    UtilityMap()
    {
        map = Texture(areaMap33Path, true);
    }

    UtilityMap(MapType mapType)
    {
        areaMapSMAASettings.texinternalformat = GL_RG8;
        areaMapSMAASettings.texfilter = GL_NEAREST;
        searchMapSMAASettings.texinternalformat = GL_R8;
        searchMapSMAASettings.texwrap = GL_CLAMP_TO_EDGE;
        searchMapSMAASettings.texfilter = GL_NEAREST;
        switch (mapType)
        {
            case MapType::areaMapSMAA:
                    map = Texture(areaMapSMAAPath, false, &areaMapSMAASettings);
                    break;
            case MapType::searchMapSMAA:
                    map = Texture(searchMapSMAAPath, false, &searchMapSMAASettings);
                    break;
            default:
                    map = Texture(areaMap33Path, true);
        }
    }

private:
    std::filesystem::path areaMap33Path = std::filesystem::current_path().append("Assets\\Textures\\mlaa\\AreaMap33.png");
    std::filesystem::path areaMapSMAAPath = std::filesystem::current_path().append("Assets\\Textures\\smaa\\AreaTex2.png");
    std::filesystem::path searchMapSMAAPath = std::filesystem::current_path().append("Assets\\Textures\\smaa\\SearchTex2.png");

    TextureSettings areaMapSMAASettings, searchMapSMAASettings;
};

#endif //UTILITYMAP_H
