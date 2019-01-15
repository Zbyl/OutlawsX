#pragma once

#include "lvt.h"
#include "TexInfos.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace outlaws {

/// Sector mesh represents a mesh for a sector.
/// It can be for rendering or collision detection.
class SectorMeshUV {
public:
    std::vector<Vector3> vertices;  ///< Vertices.
    std::vector<Vector4> uvs;       ///< Uvs for each vertex (texture coordinates in Outlaws space, plus texture id, plus WallFlag1 as float). Empty if mesh doesn't have uvs.
    std::vector<int> triangles;     ///< Triangles.

    static const int floorTriangle = -1;
    static const int ceilingTriangle = -2;
    std::vector<int> triangleToWall;   ///< Maps triangle number to sector wall index. floorTriangle and ceilingTriangle constants are used for ceiling and floor triangles.
};

class RuntimeLevel;

/// Contains all data somputed from a sector, that is needed for other purposes, such as:
/// - generating 3d models,
/// - generating collision data,
/// - answering querries about player position,
/// - keeping mapping from texture ids to textures,
/// - etc.
class RuntimeSector {
public:
    bool geometryDirty;     ///< True if geometry should be recomputed (due to geometry or texture changes).

    Plane floorPlane;       ///< Plane equation of the floor. Normal points into the sector (up).
    Plane ceilingPlane;     ///< Plane equation of the ceiling. Normal points outside the sector (up).

    std::vector<Vertex2> tesselatedVertices; ///< Tesselated sector: vertices.
    std::vector<int> tesselatedTriangles;    ///< Tesselated sector: triangles.

    /// Computes floorPlane, ceilingPlane and tesselation, based on sector data.
    /// Stores results in this object.
    void computeBasicGeometry(const LvtLevel& level, int isector);

    SectorMeshUV renderingMesh;           ///< Mesh used for rendering.
    SectorMeshUV playerCollisionMesh;     ///< Mesh used for player collisions.
    SectorMeshUV enemyCollisionMesh;      ///< Mesh used for enemy collisions.
    SectorMeshUV weaponCollisionMesh;     ///< Mesh used for weapon collisions.

    /// Computes meshes.
    /// Assumes computeBasicGeometry() was called before on all sectors.
    void computeMeshes(const RuntimeLevel& level, int isector);

    /// Returns true if given point is inside a sector.
    /// Ignores ceilings and floors.
    bool isPointInSector(Vertex2 point) const;

    /// Returns true if given point is inside a sector.
    /// Ceilings and floors are taken into account.
    bool isPointInSector(Vector3 point) const;

    /// Returns list of walls that are crossed by vector v0 -> v1.
    /// Crossed means that sign function (that returns -1, 0, 1) of distance from wall is different for v0 and v1 (and also that wall and vector touch).
    std::vector<int> getWallsCrossed(const Sector& sector, Vertex2 v0, Vertex2 v1) const;
};

class RuntimeLevel {
public:
    LvtLevel lvt;
    std::vector<TextureUvs> levelTexInfos;  ///< Maps texture index in the level to it's position in texture atlas and dimenensions.
    std::vector<RuntimeSector> runtimeSectors;

    RuntimeLevel() = default;
    RuntimeLevel(LvtLevel&& lvt_, const TexInfos& texInfos)
        : lvt(std::move(lvt_))
        , runtimeSectors(lvt.sectors.size())
    {
        for (int i = 0; i < static_cast<int>(lvt.textures.size()); ++i) {
            auto textureName = lvt.textures[i];
            TextureUvs uvs = getTextureUvs(texInfos, textureName, "default.pcx");
            levelTexInfos.push_back(uvs);
        }
    }

    void computeMeshes() {
        for (int isector = 0; isector < static_cast<int>(runtimeSectors.size()); ++isector) {
            runtimeSectors[isector].computeBasicGeometry(lvt, isector);
        }

        for (int isector = 0; isector < static_cast<int>(runtimeSectors.size()); ++isector) {
            runtimeSectors[isector].computeMeshes(*this, isector);
        }
    }

    /// Returns all sectors that contain given point.
    std::vector<int> getIncidentSectors(Vertex2 point) const;

    /// Returns all sectors that contain given point.
    std::vector<int> getIncidentSectors(Vector3 point) const;
};

void computeWall(const RuntimeLevel& level, int isector, int iwall, SectorMeshUV& mesh);
void computeSectorFloorCeiling(const RuntimeLevel& level, int isector, SectorMeshUV& mesh);
void computeSectorWalls(const RuntimeLevel& level, int isector, SectorMeshUV& mesh);

TextureUvs sectorBounds(int isector);

} // namespace outlaws
