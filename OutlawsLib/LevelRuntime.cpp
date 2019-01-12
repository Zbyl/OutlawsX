#include "LevelRuntime.h"

#include "zerrors.h"

#include <array>

extern "C" {
#include "tessellate.h"
}

namespace outlaws {

std::vector<int> LevelRuntime::getIncidentSectors(Vector3 point) const {
    std::vector<int> incidentSectors;

    for (int i = 0; i < static_cast<int>(level.sectors.size()); ++i) {
        const auto& sector = sectorRuntimes[i];
        if (sector.isPointInSector(point)) {
            incidentSectors.push_back(i);
        }
    }

    return incidentSectors;
}

/// Returns true if given wall is crossed by vector v0 -> v1.
/// Crossed means that sign function (that returns -1, 0, 1) of distance from wall is different for v0 and v1 (and also that wall and vector touch).
bool isCrossed(Vertex2 wall0, Vertex2 wall1, Vertex2 v0, Vertex2 v1) {
    auto planew = Plane(wall0, wall1);
    auto d0 = planew.signedDistance(v0);
    auto d1 = planew.signedDistance(v1);

    if (sign(d0) == sign(d1))
        return false;

    auto tv = d0 / (d0 - d1);
    if ( (tv < 0.0f) || (tv > 1.0f) )
        return false;

    auto planev = Plane(v0, v1);
    auto w0 = planev.signedDistance(wall0);
    auto w1 = planev.signedDistance(wall1);

    auto tw = w0 / (w0 - w1);
    if ( (tw < 0.0f) || (tw > 1.0f) )
        return false;

    return true;
}

std::vector<int> SectorRuntime::getWallsCrossed(const Sector& sector, Vertex2 v0, Vertex2 v1) const {
    std::vector<int> wallsCrossed;

    for (int i = 0; i < static_cast<int>(sector.walls.size()); ++i) {
        const auto& wall = sector.walls[i];
        const auto& w1 = sector.vertices[wall.v1];
        const auto& w2 = sector.vertices[wall.v2];
        if (isCrossed(w1, w2, v0, v1)) {
            wallsCrossed.push_back(i);
        }
    }

    return wallsCrossed;
}

bool SectorRuntime::isPointInSector(Vertex2 point) const {
    Vector3 point3d { point.x, 0.0f, point.z };

    for (int i = 0; i < static_cast<int>(tesselatedTriangles.size()) / 3; ++i) {
        auto v0 = tesselatedVertices[ tesselatedTriangles[i * 3 + 0] ];
        auto v1 = tesselatedVertices[ tesselatedTriangles[i * 3 + 1] ];
        auto v2 = tesselatedVertices[ tesselatedTriangles[i * 3 + 2] ];

        auto p0 = Plane(v0, v1);
        auto p1 = Plane(v1, v2);
        auto p2 = Plane(v2, v0);

        auto d0 = p0.signedDistance(point3d);
        auto d1 = p1.signedDistance(point3d);
        auto d2 = p2.signedDistance(point3d);

        if ( (d0 >= 0.0f) && (d1 >= 0.0f) && (d2 >= 0.0f) )
            return true;
    }

    return false;
}

bool SectorRuntime::isPointInSector(Vector3 point) const {
    auto floorDist = floorPlane.signedDistance(point);
    if (floorDist < 0.0f)
        return false;

    auto ceilingDist = ceilingPlane.signedDistance(point);
    if (ceilingDist > 0.0f)
        return false;

    Vertex2 vertex { point.x, point.z };
    return isPointInSector(vertex);
}

/// Computes height of vertex on given slope.
/// Floor/ceiling height should be added also.
Plane computeSlopePlane(const LvtLevel& level, SlopeParams slopeParams) {
    if (slopeParams.angle == 0)
        return { Vector3::up, 0 };

    const Sector& referenceSector = level.sectors.at(slopeParams.sector);
    const Wall& wall = referenceSector.walls.at(slopeParams.wall);
    auto v1 = referenceSector.vertices[wall.v1];
    auto v2 = referenceSector.vertices[wall.v2];

    auto wallDir = (v2 - v1).normalized();
    auto perpDir = wallDir.perpendicularClockwise();

    auto angleInDeg = slopeParams.angle * 45.0f / 2048.0f;
    const float pi = 3.14159265358979323846f;
    auto angleInRad = angleInDeg * pi / 180.0f;
    auto tan = std::tan(angleInRad);

    Vector3 wallDir3 { wallDir.x, 0, wallDir.z };
    Vector3 perpDir3 { perpDir.x, tan, perpDir.z };
    perpDir3.normalize();

    auto normal = wallDir3.cross(perpDir3);
    auto dist = Vector3 { v1.x, 0, v1.z } .dot(normal);

    return { normal, dist };
}

/// Computes height of vertex on given slope (height of a point on given plane, that has x and z the same as vertex).
/// Floor/ceiling height should be added also.
/// @param plane        Plane of the slope. Must not be vertical.
float computeVertexHeight(const Plane& plane, Vertex2 vertex) {
    // ax + by + cz = d
    // y = (d - ax - cz) / b

    auto height = (plane.dist - plane.normal.x * vertex.x - plane.normal.z * vertex.z) / plane.normal.y;
    return height;
}

/// Returns rectangle in texture atlas that corresponds to given textureId.
TextureUvs getTextureUvs(const LvtLevel& level, const TexInfos& texInfos, int textureId) {
    if (textureId == -1)
        return TextureUvs { 0 };

    auto texName = level.textures.at(textureId);
    auto canonicalTexName = canonicalTextureName(texName);
    if (texInfos.count(canonicalTexName) > 0)   // @todo This is to skip animated textures for now (ATX format).
        return texInfos.at(canonicalTexName);

    return texInfos.at("default");
}

void computeWall(const LvtLevel& level, const TexInfos& texInfos, int isector, int iwall, std::vector<Vector3>& vertices, std::vector<Uv>& uvs, std::vector<int>& triangles) {
    const Sector& sector = level.sectors.at(isector);
    const Wall& wall = sector.walls.at(iwall);

    // Structure of a wall:
    // @note Dadjoin might be also above adjoin, I guess.
    //
    // ceiling         ----------------
    // TOP wall        |              |
    // adjoin ceiling  |--------------|   \
    // MID TOP wall    |              |    |
    // adjoin floor    |--------------|    |
    // MID MID wall    |              |    > MID and OVERLAY
    // dadjoin ceiling |--------------|    |
    // MID BOT wall    |              |    |
    // dadjoin floor   |--------------|   /
    // BOT wall        |              |
    // floor           ----------------
    //
    // TOP: min(max(dadjoin ceiling, adjoin ceiling), ceiling) -> ceiling
    // MID TOP: X -> min(max(dadjoin ceiling, adjoin ceiling), ceiling)
    // MID MID: Y -> X
    // MID BOT: max(min(dadjoin floor, adjoin floor), floor) -> max(min(dadjoin ceiling, adjoin ceiling), floor)
    // BOT: floor -> max(min(dadjoin floor, adjoin floor), floor)

    enum WallKind {
        TOP,
        MID_TOP,
        MID_MID,
        MID_BOT,
        BOT,
    };

    std::array<bool, 5> drawWall = { false, false, false, false, false };    ///< Flags that say if given wall should be drawn.

    const Sector* adjoinedSector;
    const Sector* dadjoinedSector;

    Plane floorPlane { Vector3::up, 0 };
    Plane ceilingPlane { Vector3::up, 0 };
    Plane adjoinedFloorPlane { Vector3::up, 0 };
    Plane adjoinedCeilingPlane { Vector3::up, 0 };
    Plane dadjoinedFloorPlane { Vector3::up, 0 };
    Plane dadjoinedCeilingPlane { Vector3::up, 0 };

    if (hasFlag(sector, SectorFlag1::SLOPED_FLOOR))
        floorPlane = computeSlopePlane(level, sector.floorSlope);
    if (hasFlag(sector, SectorFlag1::SLOPED_CEILING))
        ceilingPlane = computeSlopePlane(level, sector.ceilingSlope);

    if (wall.adjoin != -1) {
        adjoinedSector = &level.sectors.at(wall.adjoin);
        if (hasFlag(*adjoinedSector, SectorFlag1::SLOPED_FLOOR))
            adjoinedFloorPlane = computeSlopePlane(level, adjoinedSector->floorSlope);
        if (hasFlag(*adjoinedSector, SectorFlag1::SLOPED_CEILING))
            adjoinedCeilingPlane = computeSlopePlane(level, adjoinedSector->ceilingSlope);
    }

    if (wall.dadjoin != -1) {
        dadjoinedSector = &level.sectors.at(wall.dadjoin);
        if (hasFlag(*dadjoinedSector, SectorFlag1::SLOPED_FLOOR))
            dadjoinedFloorPlane = computeSlopePlane(level, dadjoinedSector->floorSlope);
        if (hasFlag(*dadjoinedSector, SectorFlag1::SLOPED_CEILING))
            dadjoinedCeilingPlane = computeSlopePlane(level, dadjoinedSector->ceilingSlope);
    }

    struct VertexHeights {
        float ceilingAltitude;
        float upJoinedCeilingAltitude;
        float upJoinedFloorAltitude;
        float downJoinedCeilingAltitude;
        float downJoinedFloorAltitude;
        float floorAltitude;
    };

    auto computeVertexHeights = [&](Vertex2 v) {
        VertexHeights vertexHeights;

        float ceilingAltitude;
        float floorAltitude;
        float adjoinedCeilingAltitude;
        float adjoinedFloorAltitude;
        float dadjoinedCeilingAltitude;
        float dadjoinedFloorAltitude;

        ceilingAltitude = computeVertexHeight(ceilingPlane, v) + sector.ceilingY;
        floorAltitude = computeVertexHeight(floorPlane, v) + sector.floorY;

        if (wall.adjoin != -1) {
            adjoinedCeilingAltitude = computeVertexHeight(adjoinedCeilingPlane, v) + adjoinedSector->ceilingY;
            adjoinedFloorAltitude = computeVertexHeight(adjoinedFloorPlane, v) + adjoinedSector->floorY;
        }

        if (wall.dadjoin != -1) {
            dadjoinedCeilingAltitude = computeVertexHeight(dadjoinedCeilingPlane, v) + dadjoinedSector->ceilingY;
            dadjoinedFloorAltitude = computeVertexHeight(dadjoinedFloorPlane, v) + dadjoinedSector->floorY;
        }

        if ((wall.adjoin == -1) && (wall.dadjoin == -1)) {
            // No adjoins. Only MID_MID wall will be visible.
            drawWall[MID_MID] = true;
            vertexHeights.ceilingAltitude = ceilingAltitude;
            vertexHeights.upJoinedCeilingAltitude = ceilingAltitude;
            vertexHeights.upJoinedFloorAltitude = ceilingAltitude;
            vertexHeights.downJoinedCeilingAltitude = floorAltitude;
            vertexHeights.downJoinedFloorAltitude = floorAltitude;
            vertexHeights.floorAltitude = floorAltitude;
        }
        else
        if ((wall.adjoin != -1) && (wall.dadjoin == -1)) {
            // Only adjoin. Only TOP, MID_MID and BOT walls might be visible.
            drawWall[TOP] = true;
            drawWall[MID_MID] = hasFlag(wall, WallFlag1::ADJOINING_MID_TX);
            drawWall[BOT] = true;
            vertexHeights.ceilingAltitude = ceilingAltitude;
            vertexHeights.upJoinedCeilingAltitude = adjoinedCeilingAltitude;
            vertexHeights.upJoinedFloorAltitude = adjoinedCeilingAltitude;
            vertexHeights.downJoinedCeilingAltitude = adjoinedFloorAltitude;
            vertexHeights.downJoinedFloorAltitude = adjoinedFloorAltitude;
            vertexHeights.floorAltitude = floorAltitude;
        }
        else
        if ((wall.adjoin == -1) && (wall.dadjoin != -1)) {
            // Only dadjoin. Only TOP, MID_MID and BOT walls might be visible.
            drawWall[TOP] = true;
            drawWall[MID_MID] = hasFlag(wall, WallFlag1::ADJOINING_MID_TX);
            drawWall[BOT] = true;
            vertexHeights.ceilingAltitude = ceilingAltitude;
            vertexHeights.upJoinedCeilingAltitude = dadjoinedCeilingAltitude;
            vertexHeights.upJoinedFloorAltitude = dadjoinedCeilingAltitude;
            vertexHeights.downJoinedCeilingAltitude = dadjoinedFloorAltitude;
            vertexHeights.downJoinedFloorAltitude = dadjoinedFloorAltitude;
            vertexHeights.floorAltitude = floorAltitude;
        }
        else
        {
            // Two adjoins. All walls might be visible.
            drawWall[TOP] = true;
            drawWall[MID_TOP] = hasFlag(wall, WallFlag1::ADJOINING_MID_TX);
            drawWall[MID_MID] = true;
            drawWall[MID_BOT] = hasFlag(wall, WallFlag1::ADJOINING_MID_TX);
            drawWall[BOT] = true;
            vertexHeights.ceilingAltitude = ceilingAltitude;
            vertexHeights.upJoinedCeilingAltitude = std::max(adjoinedCeilingAltitude, dadjoinedCeilingAltitude);
            vertexHeights.upJoinedFloorAltitude = std::max(adjoinedFloorAltitude, dadjoinedFloorAltitude);
            vertexHeights.downJoinedCeilingAltitude = std::min(adjoinedCeilingAltitude, dadjoinedCeilingAltitude);
            vertexHeights.downJoinedFloorAltitude = std::min(adjoinedFloorAltitude, dadjoinedFloorAltitude);
            vertexHeights.floorAltitude = floorAltitude;
        }

        // Clamp all values to floor/ceiling window.
        vertexHeights.upJoinedCeilingAltitude   = clamp(vertexHeights.upJoinedCeilingAltitude, floorAltitude, ceilingAltitude);
        vertexHeights.upJoinedFloorAltitude     = clamp(vertexHeights.upJoinedFloorAltitude, floorAltitude, ceilingAltitude);
        vertexHeights.downJoinedCeilingAltitude = clamp(vertexHeights.downJoinedCeilingAltitude, floorAltitude, ceilingAltitude);
        vertexHeights.downJoinedFloorAltitude   = clamp(vertexHeights.downJoinedFloorAltitude, floorAltitude, ceilingAltitude);

        return vertexHeights;
    };

    Vertex2 verts[2] = { sector.vertices[wall.v1], sector.vertices[wall.v2] };
    VertexHeights vertexHeights[2] = { computeVertexHeights(verts[0]), computeVertexHeights(verts[1]) };

    if (hasFlag(sector, SectorFlag1::NO_WALL_DRAW)) {
        // @note This is probably not ok.
        // @note Also EXTERIOR_ADJOIN and EXTERIOR_FLOOR_ADJOIN probably should play a part here.
        if (hasFlag(sector, SectorFlag1::EXTERIOR_NO_CEIL)) {
            drawWall[TOP] = false;
        }
        if (hasFlag(sector, SectorFlag1::EXTERIOR_NO_FLOOR)) {
            drawWall[BOT] = false;
        }
    }

    auto firstIdx = static_cast<int>(vertices.size());
    auto currentIdx = firstIdx;

    auto addWall = [&](WallKind wallKind, int textureId) {
        if (!drawWall[wallKind])
            return;
        if (textureId == -1)
            return;

        for (int i = 0; i < 2; ++i) {
            auto v = verts[i];
            auto& vertexHeight = vertexHeights[i];

            float low, high;   ///< Low/high altitudes for vertex
            switch (wallKind) {
                case TOP: {
                    high = vertexHeight.ceilingAltitude;
                    low = vertexHeight.upJoinedCeilingAltitude;
                }
                break;
                case MID_TOP: {
                    high = vertexHeight.upJoinedCeilingAltitude;
                    low = vertexHeight.upJoinedFloorAltitude;
                }
                break;
                case MID_MID: {
                    high = vertexHeight.upJoinedFloorAltitude;
                    low = vertexHeight.downJoinedCeilingAltitude;
                }
                break;
                case MID_BOT: {
                    high = vertexHeight.downJoinedCeilingAltitude;
                    low = vertexHeight.downJoinedFloorAltitude;
                }
                break;
                case BOT: {
                    high = vertexHeight.downJoinedFloorAltitude;
                    low = vertexHeight.floorAltitude;
                }
                break;
                default:
                    ZASSERT(false);
            };

            Vector3 vh { v.x, high, v.z };
            Vector3 vl { v.x, low, v.z };
            vertices.push_back(vh);
            vertices.push_back(vl);
        }

        TextureUvs texUvs = getTextureUvs(level, texInfos, textureId);

        uvs.push_back({ texUvs.start.u, texUvs.start.v });
        uvs.push_back({ texUvs.start.u, texUvs.end.v });

        uvs.push_back({ texUvs.end.u, texUvs.start.v });
        uvs.push_back({ texUvs.end.u, texUvs.end.v });

        triangles.push_back(currentIdx + 1);
        triangles.push_back(currentIdx + 0);
        triangles.push_back(currentIdx + 2);

        triangles.push_back(currentIdx + 2);
        triangles.push_back(currentIdx + 3);
        triangles.push_back(currentIdx + 1);

        currentIdx += 4;
    };

    addWall(TOP, wall.top.textureId);
    addWall(MID_TOP, wall.mid.textureId);
    addWall(MID_MID, wall.mid.textureId);
    addWall(MID_BOT, wall.mid.textureId);
    addWall(MID_TOP, wall.overlay.textureId);
    addWall(MID_MID, wall.overlay.textureId);
    addWall(MID_BOT, wall.overlay.textureId);
    addWall(BOT, wall.bot.textureId);
}

/// Tesselates given sector.
/// @note It is possible to tesselate with holes - we just need to identify walls that are creating holes (in current sector or some other sectors).
/// @note Tesselator might remove or reorder some vertices.
std::pair< std::vector<Vertex2>, std::vector<int> > tesselateSector(const std::vector<Sector>& sectors, int isector) {
    const Sector& sector = sectors.at(isector);

    // Separate walls into closed loops.

    std::vector<std::pair<int, int>> wallsToUse;
    for (int iwall = 0; iwall < static_cast<int>(sector.walls.size()); ++iwall) {
        const Wall& wall = sector.walls.at(iwall);
        wallsToUse.push_back({ wall.v1, wall.v2 });
    }

    std::vector<std::vector<std::pair<int, int>>> wallLoops;

    while (!wallsToUse.empty()) {
        auto wall = wallsToUse.back();
        wallsToUse.pop_back();

        std::vector<std::pair<int, int>> wallLoop;
        wallLoop.push_back(wall);

        while (true) {
            bool extendedLoop = false;
            for (int i = 0; i < static_cast<int>(wallsToUse.size()); ++i) {
                auto otherWall = wallsToUse[i];
                if (otherWall.first != wallLoop.back().second)
                    continue;

                wallsToUse.erase(wallsToUse.begin() + i);
                wallLoop.push_back(otherWall);
                extendedLoop = true;
                break;
            }
            if (!extendedLoop)
                break;
        }

        ZASSERT(wallLoop.front().first == wallLoop.back().second) << "Wall loop is not closed.";

        wallLoops.push_back(wallLoop);
    }

    // Triangulate floor

    std::vector<double> rawVertices;
    std::vector<const double*> countours;

    for (const auto& wallLoop : wallLoops) {
        for (const auto& wall : wallLoop) {
            auto vertex = sector.vertices[wall.first];
            rawVertices.push_back(vertex.x);
            rawVertices.push_back(vertex.z);
        }
    }

    int vertexCount = 0;
    for (const auto& wallLoop : wallLoops) {
        countours.push_back(rawVertices.data() + vertexCount * 2);
        vertexCount += static_cast<int>(wallLoop.size());
        countours.push_back(rawVertices.data() + vertexCount * 2);
    }

    double *coordinates_out;
    int *tris_out;
    int nverts, ntris;

    tessellate(&coordinates_out, &nverts, &tris_out, &ntris, countours.data(), countours.data() + countours.size());

    std::vector<Vertex2> vertices;
    for (int i = 0; i < nverts; ++i) {
        vertices.push_back({ static_cast<float>(*(coordinates_out + i * 2)), static_cast<float>(*(coordinates_out + i * 2 + 1)) });
    }
    std::vector<int> triangles(tris_out, tris_out + ntris * 3);

    free(coordinates_out);
    free(tris_out);

    return { vertices, triangles };
}

TextureUvs sectorBounds(const LvtLevel& level, int isector) {
    const Sector& sector = level.sectors.at(isector);

    float minX = 100000.0f;
    float minZ = 100000.0f;
    float maxX = -100000.0f;
    float maxZ = -100000.0f;
    for (auto wv : sector.vertices) {
        minX = std::min(minX, wv.x);
        maxX = std::max(maxX, wv.x);
        minZ = std::min(minZ, wv.z);
        maxZ = std::max(maxZ, wv.z);
    }
    return TextureUvs { minX, minZ, maxX, maxZ };
}

void computeSector(const LvtLevel& level, const TexInfos& texInfos, int isector, std::vector<Vector3>& vertices, std::vector<Uv>& uvs, std::vector<int>& triangles) {
    const Sector& sector = level.sectors.at(isector);

    auto tesselated = tesselateSector(level.sectors, isector);
    auto& tesselatedVertices = tesselated.first;
    auto& tesselatedTriangles = tesselated.second;

    // Floor / Ceilling
    auto firstIdx = static_cast<int>(vertices.size());

    TextureUvs floorUvs = getTextureUvs(level, texInfos, sector.floorTexture.textureId);
    TextureUvs ceillingUvs = getTextureUvs(level, texInfos, sector.ceilingTexture.textureId);

    TextureUvs bounds = sectorBounds(level, isector);
    float minX = bounds.start.u;
    float minZ = bounds.start.v;
    float maxX = bounds.end.u;
    float maxZ = bounds.end.u;

    auto rangeX = std::max(1.0f, maxX - minX);
    auto rangeZ = std::max(1.0f, maxZ - minZ);

    Plane floorPlane = hasFlag(sector, SectorFlag1::SLOPED_FLOOR) ? computeSlopePlane(level, sector.floorSlope) : Plane { Vector3::up, 0 };
    Plane ceilingPlane = hasFlag(sector, SectorFlag1::SLOPED_CEILING) ? computeSlopePlane(level, sector.ceilingSlope) : Plane { Vector3::up, 0 };

    for (auto wv : tesselatedVertices) {
        auto floorAltitude = computeVertexHeight(floorPlane, wv) + sector.floorY;
        auto ceilingAltitude = computeVertexHeight(ceilingPlane, wv) + sector.ceilingY;

        Vector3 vf { wv.x, floorAltitude, wv.z };
        Vector3 vc { wv.x, ceilingAltitude, wv.z };
        vertices.push_back(vf);
        vertices.push_back(vc);

        // 8 units on the floor is full texture width (maybe...)
        // To support it we need custom wrapping in shader.
        // For now we'll do a hack.
        Uv uv { (wv.x - minX) / rangeX, (wv.z - minZ) / rangeZ };

        auto floorUv = uv;
        floorUv.u = floorUv.u * (floorUvs.end.u - floorUvs.start.u) + floorUvs.start.u;
        floorUv.v = floorUv.v * (floorUvs.end.v - floorUvs.start.v) + floorUvs.start.v;

        auto ceillingUv = uv;
        ceillingUv.u = ceillingUv.u * (ceillingUvs.end.u - ceillingUvs.start.u) + ceillingUvs.start.u;
        ceillingUv.v = ceillingUv.v * (ceillingUvs.end.v - ceillingUvs.start.v) + ceillingUvs.start.v;

        uvs.push_back(floorUv);
        uvs.push_back(ceillingUv);
    }

    for (int i = 0; i < static_cast<int>(tesselatedTriangles.size()); i += 3) {
        int t0 = tesselatedTriangles[i + 0];
        int t1 = tesselatedTriangles[i + 1];
        int t2 = tesselatedTriangles[i + 2];

        // Floor
        if (!hasFlag(sector, SectorFlag1::EXTERIOR_NO_FLOOR)) {
            triangles.push_back(firstIdx + t0 * 2 + 0);
            triangles.push_back(firstIdx + t1 * 2 + 0);
            triangles.push_back(firstIdx + t2 * 2 + 0);
        }

        // Ceilling
        if (!hasFlag(sector, SectorFlag1::EXTERIOR_NO_CEIL)) {
            triangles.push_back(firstIdx + t0 * 2 + 1);
            triangles.push_back(firstIdx + t2 * 2 + 1);
            triangles.push_back(firstIdx + t1 * 2 + 1);
        }
    }

    // Walls
    for (int i = 0; i < static_cast<int>(sector.walls.size()); ++i) {
        computeWall(level, texInfos, isector, i, vertices, uvs, triangles);
    }
}

void computeLevel(const LvtLevel& level, const TexInfos& texInfos, std::vector<Vector3>& vertices, std::vector<Uv>& uvs, std::vector<int>& triangles) {
    for (int i = 0; i < static_cast<int>(level.sectors.size()); ++i) {
        computeSector(level, texInfos, i, vertices, uvs, triangles);
    }
}

} // namespace outlaws
