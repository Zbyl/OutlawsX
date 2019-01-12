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
class SectorMesh {
public:
    std::vector<Vector3> vertices;  ///< Vertices.
    std::vector<int> triangles;     ///< Triangles.

    static const int floorTriangle = -1;
    static const int ceilingTriangle = -2;
    std::vector<int> triangleToWall;   ///< Maps triangle number to sector wall index. floorTriangle and ceilingTriangle constants are used for ceiling and floor triangles.
};

/// Sector mesh represents a mesh for a sector.
/// It can be for rendering or collision detection.
class SectorMeshUV : public SectorMesh {
public:
    std::vector<Uv> uvs;            ///< Uvs for each vertex. Empty if mesh doesn't have uvs.
};

/// Contains all data somputed from a sector, that is needed for other purposes, such as:
/// - generating 3d models,
/// - generating collision data,
/// - answering querries about player position,
/// - keeping mapping from texture ids to textures,
/// - etc.
class SectorRuntime {
public:
    bool geometryDirty;     ///< True if meshes should be recomputed (due to geometry or texture changes).

    Plane floorPlane;       ///< Plane equation of the floor. Normal points into the sector (up).
    Plane ceilingPlane;     ///< Plane equation of the ceiling. Normal points outside the sector (up).

    std::vector<Vertex2> tesselatedVertices; ///< Tesselated sector: vertices.
    std::vector<int> tesselatedTriangles;    ///< Tesselated sector: triangles.

    SectorMeshUV renderingMesh;         ///< Mesh used for rendering.
    SectorMesh playerCollisionMesh;     ///< Mesh used for player collisions.
    SectorMesh enemyCollisionMesh;      ///< Mesh used for enemy collisions.
    SectorMesh weaponCollisionMesh;     ///< Mesh used for weapon collisions.

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

class LevelRuntime {
public:
    LvtLevel level;
    TexInfos texInfos;

    std::vector<SectorRuntime> sectorRuntimes;

    /// Returns all sectors that contain given point.
    std::vector<int> getIncidentSectors(Vector3 point) const;
};

void computeWall(const LvtLevel& level, const TexInfos& texInfos, int isector, int iwall, std::vector<Vector3>& vertices, std::vector<Uv>& uvs, std::vector<int>& triangles);
void computeSector(const LvtLevel& level, const TexInfos& texInfos, int isector, std::vector<Vector3>& vertices, std::vector<Uv>& uvs, std::vector<int>& triangles);
void computeLevel(const LvtLevel& level, const TexInfos& texInfos, std::vector<Vector3>& vertices, std::vector<Uv>& uvs, std::vector<int>& triangles);

TextureUvs sectorBounds(const LvtLevel& level, int isector);

} // namespace outlaws
