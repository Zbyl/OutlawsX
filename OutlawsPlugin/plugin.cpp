


#define OUTLAWSX_API __declspec(dllexport)

#include "LevelRuntime.h"

#include <algorithm>

outlaws::LvtLevel level;
outlaws::TexInfos texInfos;

extern "C" {

OUTLAWSX_API int func(int a)
{
    return a + 3;
}

OUTLAWSX_API int loadLevel()
{
    try {
        level = outlaws::loadLvt(R"(F:\VSProjects\LabFuse\trash\HIDEOUT.LVT)");
        texInfos = outlaws::loadTexInfos(R"(F:\VSProjects\LabFuse\UnityProj0\UnityProj0\Assets\Textures\pack.json)");
        return static_cast<int>(level.sectors.size());
    }
    catch (...) {
        return 0;
    }
}

OUTLAWSX_API int overlappingSector(float x, float z)
{
    try {
        for (int isector = 0; isector < static_cast<int>(level.sectors.size()); ++isector) {
            auto bounds = sectorBounds(level, isector);
            if (x < bounds.start.u)
                continue;
            if (x > bounds.end.u)
                continue;
            if (z < bounds.start.v)
                continue;
            if (z > bounds.end.v)
                continue;

            return isector;
        }
    }
    catch (...) {
    }

    return -1;
}

OUTLAWSX_API void fetchData(outlaws::Vector3* vertices, outlaws::Uv* texCoords, int* numVertices, int* triangles, int* numTriangles, int sectorNum)
{
    try {
        std::vector<outlaws::Vector3> verts;
        std::vector<outlaws::Uv> uvs;
        std::vector<int> tris;

    #if 0
        verts.push_back({ 0, 1, 0 });
        verts.push_back({ 1, 1, 0 });
        verts.push_back({ 0, 0, 0 });
        verts.push_back({ 1, 0, 0 });

        tris.push_back(0);
        tris.push_back(1);
        tris.push_back(2);

        tris.push_back(2);
        tris.push_back(1);
        tris.push_back(3);
    #endif

        if (sectorNum == -1)
            outlaws::computeLevel(level, texInfos, verts, uvs, tris);
        else
            outlaws::computeSector(level, texInfos, sectorNum, verts, uvs, tris);

        if (*numVertices < static_cast<int>(verts.size())) {
            *numVertices = 0;
            *numTriangles = 0;
            return;
        }

        if (*numTriangles < static_cast<int>(tris.size())) {
            *numVertices = 0;
            *numTriangles = 0;
            return;
        }

        *numVertices = static_cast<int>(verts.size());
        *numTriangles = static_cast<int>(tris.size());

        std::copy(verts.begin(), verts.end(), vertices);
        std::copy(uvs.begin(), uvs.end(), texCoords);
        std::copy(tris.begin(), tris.end(), triangles);
    }
    catch (...) {
        *numVertices = 0;
        *numTriangles = 0;
    }
}

} // extern "C"
