


#define OUTLAWSX_API __declspec(dllexport)

#include "RuntimeLevel.h"
#include "Lvb.h"

#include <algorithm>
#include <cstring>

#include <Objbase.h>

#include <string_view>
#include <sstream>


using namespace outlaws;

RuntimeLevel level;
TexInfos texInfos;
std::string errorMessage;

static bool ends_with(std::string_view str, std::string_view suffix)
{
    return (str.size() >= suffix.size()) && (str.compare(str.size()-suffix.size(), suffix.size(), suffix) == 0);
}

static bool starts_with(std::string_view str, std::string_view prefix)
{
    return (str.size() >= prefix.size()) && (str.compare(0, prefix.size(), prefix) == 0);
}

extern "C" {

const char* returnString(const std::string& str){
    char* pszStringToReturn = (char*)::CoTaskMemAlloc(str.size() + 1);

    strcpy(pszStringToReturn, str.c_str());

    return pszStringToReturn;
}

/// @note Free result using FreeCStrMarshaler custom marshaller on C# side.
OUTLAWSX_API const char* getErrorMessage()
{
    return returnString(errorMessage);
}

/// @note Free result using FreeCStrMarshaler custom marshaller on C# side.
OUTLAWSX_API const char* sectorFlag1ToString(int isector)
{
    try {
        const Sector& sector = level.lvt.sectors.at(isector);

        std::string s = flagToString(static_cast<SectorFlag1>(sector.flag1));
        return returnString(s);
    }
    catch (const std::exception& exc) {
        return returnString(exc.what());
    }
    catch (...) {
        return returnString("Unknown exception.");
    }
}

/// @note Free result using FreeCStrMarshaler custom marshaller on C# side.
OUTLAWSX_API const char* wallFlag1ToString(int isector, int iwall)
{
    try {
        const Sector& sector = level.lvt.sectors.at(isector);
        const Wall& wall = sector.walls.at(iwall);

        std::string s = flagToString(static_cast<WallFlag1>(wall.flag1));
        return returnString(s);
    }
    catch (const std::exception& exc) {
        return returnString(exc.what());
    }
    catch (...) {
        return returnString("Unknown exception.");
    }
}

/// @note Free result using FreeCStrMarshaler custom marshaller on C# side.
OUTLAWSX_API const char* wallFlag2ToString(int isector, int iwall)
{
    try {
        const Sector& sector = level.lvt.sectors.at(isector);
        const Wall& wall = sector.walls.at(iwall);

        std::string s = flagToString(static_cast<WallFlag2>(wall.flag2));
        return returnString(s);
    }
    catch (const std::exception& exc) {
        return returnString(exc.what());
    }
    catch (...) {
        return returnString("Unknown exception.");
    }
}

OUTLAWSX_API int func(int a)
{
    return a + 3;
}

// @todo Unicode stuff? Currently everything is ANSI.
// @todo Add proper error handling everywhere.
OUTLAWSX_API int loadLevel(const char* levelFile, const char* textureFile)
{
    try {
        texInfos = loadTexInfos(textureFile);

        std::string levelPath = levelFile;
        if (ends_with(levelPath, ".LVB") || ends_with(levelPath, ".lvb")) {
            auto lvtString = lvb::loadLvbFile(levelFile);
            std::stringstream lvtStream(lvtString);
            auto lvt = loadLvt(lvtStream);
            level = RuntimeLevel(std::move(lvt), texInfos);
        } else {
            auto lvt = loadLvt(levelFile);
            level = RuntimeLevel(std::move(lvt), texInfos);
        }

        level.computeMeshes();
        return static_cast<int>(level.runtimeSectors.size());
    }
    catch (const std::exception& exc)
    {
        errorMessage = std::string("error=") + exc.what() + " levelFile=" + levelFile + " textureFile=" + textureFile;
        return -1;
    }
    catch (...)
    {
        errorMessage = "Unknown error.";
        return -1;
    }
}

OUTLAWSX_API int overlappingSector2d(Vertex2 point)
{
    try {
        auto sectors = level.getIncidentSectors(point);
        if (!sectors.empty())
            return sectors.front();
    }
    catch (...) {
    }

    return -1;
}

OUTLAWSX_API int overlappingSector3d(Vector3 point)
{
    try {
        auto sectors = level.getIncidentSectors(point);
        if (!sectors.empty())
            return sectors.front();
    }
    catch (...) {
    }

    return -1;
}

OUTLAWSX_API void fetchTexInfos(TextureUvs* texInfos, int* numTexInfos)
{
    try {
        if (*numTexInfos < static_cast<int>(level.levelTexInfos.size())) {
            *numTexInfos = 0;
            return;
        }

        *numTexInfos = static_cast<int>(level.levelTexInfos.size());

        std::copy(level.levelTexInfos.begin(), level.levelTexInfos.end(), texInfos);
    }
    catch (...) {
        *numTexInfos = 0;
    }
}

OUTLAWSX_API int getWallForTriangle(int isector, int triangleIndex)
{
    try {
        const RuntimeSector& sector = level.runtimeSectors.at(isector);
        int iwall = sector.renderingMesh.triangleToWall.at(triangleIndex);
        return iwall;
    }
    catch (...) {
        return -6;
    }
    return -5;
}

OUTLAWSX_API void fetchData(Vector3* vertices, Vector4* texCoords, int* numVertices, int* triangles, int* numTriangles, int sectorNum)
{
    try {
        const RuntimeSector& sector = level.runtimeSectors.at(sectorNum);
        const auto& mesh = sector.renderingMesh;

        if (*numVertices < static_cast<int>(mesh.vertices.size())) {
            *numVertices = 0;
            *numTriangles = 0;
            return;
        }

        if (*numTriangles < static_cast<int>(mesh.triangles.size())) {
            *numVertices = 0;
            *numTriangles = 0;
            return;
        }

        *numVertices = static_cast<int>(mesh.vertices.size());
        *numTriangles = static_cast<int>(mesh.triangles.size());

        std::copy(mesh.vertices.begin(), mesh.vertices.end(), vertices);
        std::copy(mesh.uvs.begin(), mesh.uvs.end(), texCoords);
        std::copy(mesh.triangles.begin(), mesh.triangles.end(), triangles);
    }
    catch (...) {
        *numVertices = 0;
        *numTriangles = 0;
    }
}

} // extern "C"
