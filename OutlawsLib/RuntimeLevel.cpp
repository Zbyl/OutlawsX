#include "RuntimeLevel.h"

#include "zerrors.h"

#include <array>

extern "C" {
#include "tessellate.h"
}

namespace outlaws {

std::vector<int> RuntimeLevel::getIncidentSectors(Vertex2 point) const {
    std::vector<int> incidentSectors;

    for (int i = 0; i < static_cast<int>(runtimeSectors.size()); ++i) {
        const auto& sector = runtimeSectors[i];
        if (sector.isPointInSector(point)) {
            incidentSectors.push_back(i);
        }
    }

    return incidentSectors;
}

std::vector<int> RuntimeLevel::getIncidentSectors(Vector3 point) const {
    std::vector<int> incidentSectors;

    for (int i = 0; i < static_cast<int>(runtimeSectors.size()); ++i) {
        const auto& sector = runtimeSectors[i];
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

std::vector<int> RuntimeSector::getWallsCrossed(const Sector& sector, Vertex2 v0, Vertex2 v1) const {
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

bool RuntimeSector::isPointInSector(Vertex2 point) const {
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

bool RuntimeSector::isPointInSector(Vector3 point) const {
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
        return { Vector3::up, 0.0f };

    // @note Happens in Canyon for sector #109, and this seems to be fine.
    if (slopeParams.sector == -1)
        return { Vector3::up, 0.0f };

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

void computeWall(const RuntimeLevel& level, int isector, int iwall, SectorMeshUV& mesh) {
    const RuntimeSector& runtimeSector = level.runtimeSectors.at(isector);
    const Sector& sector = level.lvt.sectors.at(isector);
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

    Plane floorPlane = runtimeSector.floorPlane;
    Plane ceilingPlane  = runtimeSector.ceilingPlane;
    Plane adjoinedFloorPlane { Vector3::up, 0 };
    Plane adjoinedCeilingPlane { Vector3::up, 0 };
    Plane dadjoinedFloorPlane { Vector3::up, 0 };
    Plane dadjoinedCeilingPlane { Vector3::up, 0 };

    if (wall.adjoin != -1) {
        adjoinedSector = &level.lvt.sectors.at(wall.adjoin);
        const RuntimeSector& adjoinedRuntimeSector = level.runtimeSectors.at(wall.adjoin);
        adjoinedFloorPlane = adjoinedRuntimeSector.floorPlane;
        adjoinedCeilingPlane = adjoinedRuntimeSector.ceilingPlane;
    }

    if (wall.dadjoin != -1) {
        dadjoinedSector = &level.lvt.sectors.at(wall.dadjoin);
        const RuntimeSector& dadjoinedRuntimeSector = level.runtimeSectors.at(wall.dadjoin);
        dadjoinedFloorPlane = dadjoinedRuntimeSector.floorPlane;
        dadjoinedCeilingPlane = dadjoinedRuntimeSector.ceilingPlane;
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

    auto firstIdx = static_cast<int>(mesh.vertices.size());
    auto currentIdx = firstIdx;

    // @todo Check how WallFlag1::WALL_TX_ANCHORED and WallFlag1::ELEV_CAN_SCROLL_* flags come into play here.
    float wallBase = sector.floorY;
    float wallLength = (verts[1] - verts[0]).magnitude();

    auto addWall = [&](WallKind wallKind, TextureParamsSmall textureParams, bool isSign) {
        if (!drawWall[wallKind])
            return;
        if (textureParams.textureId == -1)
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

            mesh.vertices.push_back(vh);
            mesh.vertices.push_back(vl);

            //auto wb = hasFlag(wall, WallFlag1::WALL_TX_ANCHORED) ? : wallBase;
            auto wb = low;
            
            if ( (wallKind == TOP) && hasFlag(wall, WallFlag1::ELEV_CAN_SCROLL_TOP_TX) ) wb = wallBase;
            if (isSign) {
                if ( (wallKind == MID_TOP || wallKind == MID_MID || wallKind == MID_BOT) && hasFlag(wall, WallFlag1::ELEV_CAN_SCROLL_SIGN_TX) ) wb = wallBase;
            } else {
                if ( (wallKind == MID_TOP || wallKind == MID_MID || wallKind == MID_BOT) && hasFlag(wall, WallFlag1::ELEV_CAN_SCROLL_MID_TX) ) wb = wallBase;
            }
            if ( (wallKind == BOT) && hasFlag(wall, WallFlag1::ELEV_CAN_SCROLL_BOT_TX) ) wb = wallBase;

            int flag = wall.flag1;
            if (isSign)
                flag |= (1u << 31); // @todo This is a hacky flag for pixel shader.
            mesh.uvs.push_back({ i * wallLength + textureParams.offsX, (high - wb) + textureParams.offsY, static_cast<float>(textureParams.textureId), *reinterpret_cast<const float*>(&flag) });
            mesh.uvs.push_back({ i * wallLength + textureParams.offsX, (low - wb) + textureParams.offsY, static_cast<float>(textureParams.textureId), *reinterpret_cast<const float*>(&flag) });
        }

        mesh.triangles.push_back(currentIdx + 1);
        mesh.triangles.push_back(currentIdx + 0);
        mesh.triangles.push_back(currentIdx + 2);
        mesh.triangleToWall.push_back(iwall);

        mesh.triangles.push_back(currentIdx + 2);
        mesh.triangles.push_back(currentIdx + 3);
        mesh.triangles.push_back(currentIdx + 1);
        mesh.triangleToWall.push_back(iwall);

        currentIdx += 4;
    };

    addWall(TOP, wall.top, false);
    addWall(MID_TOP, wall.mid, false);
    addWall(MID_MID, wall.mid, false);
    addWall(MID_BOT, wall.mid, false);
    addWall(BOT, wall.bot, false);

    auto ovr = wall.overlay;
    ovr.offsX = wall.top.offsX - ovr.offsX;
    ovr.offsY = wall.top.offsY + ovr.offsY;
    addWall(TOP, ovr, true);

    ovr = wall.overlay;
    ovr.offsX = wall.mid.offsX - ovr.offsX;
    ovr.offsY = wall.mid.offsY + ovr.offsY;
    addWall(MID_TOP, ovr, true);
    addWall(MID_MID, ovr, true);
    addWall(MID_BOT, ovr, true);

    ovr = wall.overlay;
    ovr.offsX = wall.bot.offsX - ovr.offsX;
    ovr.offsY = wall.bot.offsY + ovr.offsY;
    addWall(BOT, ovr, true);
}

/// Tesselates given sector.
/// @note Tesselator might remove or reorder vertices.
std::pair< std::vector<Vertex2>, std::vector<int> > tesselateSector(const Sector& sector) {
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

void computeSectorWalls(const RuntimeLevel& level, int isector, SectorMeshUV& mesh) {
    const Sector& sector = level.lvt.sectors.at(isector);

    // Walls
    for (int i = 0; i < static_cast<int>(sector.walls.size()); ++i) {
        computeWall(level, isector, i, mesh);
    }
}

TextureUvs sectorBounds(const Sector& sector) {
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

void computeSectorFloorCeiling(const RuntimeLevel& level, int isector, SectorMeshUV& mesh) {
    const Sector& sector = level.lvt.sectors.at(isector);
    const RuntimeSector& runtimeSector = level.runtimeSectors.at(isector);

    // Floor / Ceilling
    auto firstIdx = static_cast<int>(mesh.vertices.size());

    Plane floorPlane = hasFlag(sector, SectorFlag1::SLOPED_FLOOR) ? computeSlopePlane(level.lvt, sector.floorSlope) : Plane { Vector3::up, 0 };
    Plane ceilingPlane = hasFlag(sector, SectorFlag1::SLOPED_CEILING) ? computeSlopePlane(level.lvt, sector.ceilingSlope) : Plane { Vector3::up, 0 };

    for (auto wv : runtimeSector.tesselatedVertices) {
        auto floorAltitude = computeVertexHeight(floorPlane, wv) + sector.floorY;
        auto ceilingAltitude = computeVertexHeight(ceilingPlane, wv) + sector.ceilingY;

        Vector3 vf { wv.x, floorAltitude, wv.z };
        Vector3 vc { wv.x, ceilingAltitude, wv.z };
        mesh.vertices.push_back(vf);
        mesh.vertices.push_back(vf);
        mesh.vertices.push_back(vc);
        mesh.vertices.push_back(vc);

        int zero = 0;
        auto refv = wv;
        // Rotating the floor/ceiling texture.
        const float pi = 3.14159265358979323846f;

        Vertex2 ceilt { refv.x - sector.ceilingTexture.offsX, refv.z - sector.ceilingTexture.offsY };
        auto ceilAngleInDeg = - sector.ceilingTexture.angle;
        auto ceilAngleInRad = ceilAngleInDeg * pi / 180.0f;
        Vertex2 ceilv { ceilt.x * cos(ceilAngleInRad) - ceilt.z * sin(ceilAngleInRad),
                        ceilt.x * sin(ceilAngleInRad) + ceilt.z * cos(ceilAngleInRad) };

        Vertex2 floort { refv.x - sector.floorTexture.offsX, refv.z - sector.floorTexture.offsY };
        auto floorAngleInDeg = - sector.floorTexture.angle;
        auto floorAngleInRad = floorAngleInDeg * pi / 180.0f;
        Vertex2 floorv { floort.x * cos(floorAngleInRad) - floort.z * sin(floorAngleInRad),
                         floort.x * sin(floorAngleInRad) + floort.z * cos(floorAngleInRad) };

        //auto refv = sector.vertices.front();
        Vector4 floorUv = { -floorv.x, floorv.z, static_cast<float>(sector.floorTexture.textureId), *reinterpret_cast<const float*>(&zero) };
        Vector4 floorOverlayUv = { -floorv.x, floorv.z, static_cast<float>(sector.floorTexture.textureId), *reinterpret_cast<const float*>(&zero) };
        Vector4 ceilingUv = { -ceilv.x, ceilv.z, static_cast<float>(sector.ceilingTexture.textureId), *reinterpret_cast<const float*>(&zero) };
        Vector4 ceilingOverlayUv = { -ceilv.x, ceilv.z, static_cast<float>(sector.ceilingTexture.textureId), *reinterpret_cast<const float*>(&zero) };

        mesh.uvs.push_back(floorUv);
        mesh.uvs.push_back(floorOverlayUv);
        mesh.uvs.push_back(ceilingUv);
        mesh.uvs.push_back(ceilingOverlayUv);
    }

    for (int i = 0; i < static_cast<int>(runtimeSector.tesselatedTriangles.size()); i += 3) {
        int t0 = runtimeSector.tesselatedTriangles[i + 0];
        int t1 = runtimeSector.tesselatedTriangles[i + 1];
        int t2 = runtimeSector.tesselatedTriangles[i + 2];

        // Floor
        if (!hasFlag(sector, SectorFlag1::EXTERIOR_NO_FLOOR)) {
            mesh.triangles.push_back(firstIdx + t0 * 4 + 0);
            mesh.triangles.push_back(firstIdx + t1 * 4 + 0);
            mesh.triangles.push_back(firstIdx + t2 * 4 + 0);
            mesh.triangleToWall.push_back(SectorMeshUV::floorTriangle);
        }

        // Floor overlay
        if (!hasFlag(sector, SectorFlag1::EXTERIOR_NO_FLOOR) && (sector.floorOverlayTexture.textureId != -1)) {    // @todo There is proabbly some flag for enabling floor overlay.
            mesh.triangles.push_back(firstIdx + t0 * 4 + 1);
            mesh.triangles.push_back(firstIdx + t1 * 4 + 1);
            mesh.triangles.push_back(firstIdx + t2 * 4 + 1);
            mesh.triangleToWall.push_back(SectorMeshUV::floorTriangle);
        }

        // Ceilling
        if (!hasFlag(sector, SectorFlag1::EXTERIOR_NO_CEIL)) {
            mesh.triangles.push_back(firstIdx + t0 * 4 + 2);
            mesh.triangles.push_back(firstIdx + t2 * 4 + 2);
            mesh.triangles.push_back(firstIdx + t1 * 4 + 2);
            mesh.triangleToWall.push_back(SectorMeshUV::ceilingTriangle);
        }

        // Ceilling overlay
        if (!hasFlag(sector, SectorFlag1::EXTERIOR_NO_CEIL) && (sector.ceilingOverlayTexture.textureId != -1)) {    // @todo There is proabbly some flag for enabling ceiling overlay.
            mesh.triangles.push_back(firstIdx + t0 * 4 + 3);
            mesh.triangles.push_back(firstIdx + t2 * 4 + 3);
            mesh.triangles.push_back(firstIdx + t1 * 4 + 3);
            mesh.triangleToWall.push_back(SectorMeshUV::ceilingTriangle);
        }
    }
}

void RuntimeSector::computeBasicGeometry(const LvtLevel& level, int isector) {
    const Sector& sector = level.sectors.at(isector);

    auto tesselated = tesselateSector(sector);
    tesselatedVertices = tesselated.first;
    tesselatedTriangles = tesselated.second;

    floorPlane = hasFlag(sector, SectorFlag1::SLOPED_FLOOR) ? computeSlopePlane(level, sector.floorSlope) : Plane { Vector3::up, 0 };
    ceilingPlane = hasFlag(sector, SectorFlag1::SLOPED_CEILING) ? computeSlopePlane(level, sector.ceilingSlope) : Plane { Vector3::up, 0 };
}

void RuntimeSector::computeMeshes(const RuntimeLevel& level, int isector) {
    computeSectorFloorCeiling(level, isector, renderingMesh);
    computeSectorWalls(level, isector, renderingMesh);
}

} // namespace outlaws
