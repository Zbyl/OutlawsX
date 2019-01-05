#include "lvt.h"

#include "LvtLexer.h"
#include "LvtParser.h"
#include "LvtParserBaseVisitor.h"
#include "zerrors.h"
#include "file_utilities.h"

#include <antlr4-runtime.h>

#include <iostream>
#include <vector>
#include <algorithm>
#include <string> 

#include <boost/lexical_cast.hpp>

#include <nlohmann/json.hpp>

extern "C" {
#include "tessellate.h"
}

namespace lvt {

void computeWall(const LvtLevel& level, const TexInfos& texInfos, int isector, int iwall, std::vector<Vector3>& vertices, std::vector<Uv>& uvs, std::vector<int>& triangles) {
    const Sector& sector = level.sectors.at(isector);
    const Wall& wall = sector.walls.at(iwall);

    // There are 8 vertices for each vertex: floor, bottom wall end, top wall end, ceiling, and each wall has it's own vertices.
    // @todo What about DADJOIN? Do we need more vertices?

    auto firstIdx = static_cast<int>(vertices.size());

    auto v1 = sector.vertices[wall.v1];
    auto v2 = sector.vertices[wall.v2];

    enum WallKind {
        MID = 0,
        TOP = 1,
        BOT = 2,
        OVERLAY = 3,
    };

    bool drawWall[4] = { true, false, false, true };

    float upperFloorY = sector.floorY;
    float lowerCeilingY = sector.ceilingY;
    if (wall.adjoin != -1) {
        if (!(wall.flag1 & static_cast<uint32_t>(WallFlag1::ADJOINING_MID_TX))) {
            drawWall[MID] = false;
            drawWall[OVERLAY] = false;
        }

        // Reposition vertices.
        const Sector& otherSector = level.sectors.at(wall.adjoin);
        const Wall& otherWall = otherSector.walls.at(wall.mirror);

        upperFloorY = std::max(sector.floorY, otherSector.floorY);
        lowerCeilingY = std::min(sector.ceilingY, otherSector.ceilingY);
    }

    float lowAndHighAltitudesForWalls[4 * 2] = {
        upperFloorY, lowerCeilingY,     // MID
        lowerCeilingY, sector.ceilingY, // TOP
        sector.floorY, upperFloorY,     // BOT
        upperFloorY, lowerCeilingY,     // OVERLAY
    };

    // For each possible texture on the wall prepare 4 vertices with uvs.
    auto addWallVertices = [&](WallKind wallKind, int textureId) {
        auto highAltitude = lowAndHighAltitudesForWalls[wallKind * 2 + 1];
        auto lowAltitude = lowAndHighAltitudesForWalls[wallKind * 2 + 0];

        if (highAltitude <= lowAltitude) {
            drawWall[wallKind] = false;
        }

        Vector3 v1h { v1.x, highAltitude, v1.z };
        Vector3 v2h { v2.x, highAltitude, v2.z };
        Vector3 v1l { v1.x, lowAltitude, v1.z };
        Vector3 v2l { v2.x, lowAltitude, v2.z };
        vertices.push_back(v1h);
        vertices.push_back(v2h);
        vertices.push_back(v1l);
        vertices.push_back(v2l);

        TextureUvs texUvs = { 0 };
        if (textureId != -1) {
            auto texName = level.textures.at(textureId);
            auto canonicalTexName = canonicalTextureName(texName);
            if (texInfos.count(canonicalTexName) > 0)   // @todo This is to skip animated textures for now (ATX format).
                texUvs = texInfos.at(canonicalTexName);
            else
                texUvs = texInfos.at("default");
        }

        uvs.push_back({ texUvs.start.u, texUvs.start.v });
        uvs.push_back({ texUvs.end.u, texUvs.start.v });
        uvs.push_back({ texUvs.start.u, texUvs.end.v });
        uvs.push_back({ texUvs.end.u, texUvs.end.v });
    };

    addWallVertices(MID, wall.mid.textureId);
    addWallVertices(TOP, wall.top.textureId);
    addWallVertices(BOT, wall.bot.textureId);
    addWallVertices(OVERLAY, wall.overlay.textureId);

    if (sector.flag1 & static_cast<uint32_t>(SectorFlag1::NO_WALL_DRAW)) {
        // @note This might be ok, but might be not...
        if (sector.flag1 & static_cast<uint32_t>(SectorFlag1::EXTERIOR_NO_CEIL)) {
            drawWall[TOP] = false;
        }
        if (sector.flag1 & static_cast<uint32_t>(SectorFlag1::EXTERIOR_NO_FLOOR)) {
            drawWall[BOT] = false;
        }
    }

    drawWall[MID] &= (wall.mid.textureId != -1);
    drawWall[TOP] &= (wall.mid.textureId != -1);
    drawWall[BOT] &= (wall.mid.textureId != -1);
    drawWall[OVERLAY] &= (wall.mid.textureId != -1);

    for (int wallKind = MID; wallKind <= OVERLAY; ++wallKind) {
        if (!drawWall[wallKind])
            continue;

        triangles.push_back(firstIdx + wallKind * 4 + 2);
        triangles.push_back(firstIdx + wallKind * 4 + 0);
        triangles.push_back(firstIdx + wallKind * 4 + 1);

        triangles.push_back(firstIdx + wallKind * 4 + 1);
        triangles.push_back(firstIdx + wallKind * 4 + 3);
        triangles.push_back(firstIdx + wallKind * 4 + 2);
    }
}

/// Tesselates given sector.
/// @note It is possible to tesselate with holes - we just need to identify walls that are creating holes (in current sector or some other sectors).
/// @note Tesselator might remove or reorder some vertices.
std::pair< std::vector<lvt::Vertex2>, std::vector<int> > tesselateSector(const std::vector<Sector>& sectors, int isector) {
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

    std::vector<lvt::Vertex2> vertices;
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

    TextureUvs floorUvs = { 0 };
    TextureUvs ceillingUvs = { 0 };
    if (sector.floorTexture.textureId != -1) {
        auto texName = level.textures.at(sector.floorTexture.textureId);
        auto canonicalTexName = canonicalTextureName(texName);
        if (texInfos.count(canonicalTexName) > 0)   // @todo This is to skip animated textures for now (ATX format).
            floorUvs = texInfos.at(canonicalTexName);
        else
            floorUvs = texInfos.at("default");
    }
    if (sector.ceilingTexture.textureId != -1) {
        auto texName = level.textures.at(sector.ceilingTexture.textureId);
        auto canonicalTexName = canonicalTextureName(texName);
        if (texInfos.count(canonicalTexName) > 0)   // @todo This is to skip animated textures for now (ATX format).
            ceillingUvs = texInfos.at(canonicalTexName);
        else
            ceillingUvs = texInfos.at("default");
    }

    TextureUvs bounds = sectorBounds(level, isector);
    float minX = bounds.start.u;
    float minZ = bounds.start.v;
    float maxX = bounds.end.u;
    float maxZ = bounds.end.u;

    auto rangeX = std::max(1.0f, maxX - minX);
    auto rangeZ = std::max(1.0f, maxZ - minZ);

    for (auto wv : tesselatedVertices) {
        Vector3 vf { wv.x, sector.floorY, wv.z };
        Vector3 vc { wv.x, sector.ceilingY, wv.z };
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
        if (!(sector.flag1 & static_cast<uint32_t>(SectorFlag1::EXTERIOR_NO_FLOOR))) {
            triangles.push_back(firstIdx + t0 * 2 + 0);
            triangles.push_back(firstIdx + t1 * 2 + 0);
            triangles.push_back(firstIdx + t2 * 2 + 0);
        }

        // Ceilling
        if (!(sector.flag1 & static_cast<uint32_t>(SectorFlag1::EXTERIOR_NO_CEIL))) {
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

lvt::TextureParamsSmall parseTextureParamsSmall(lvtgrammar::LvtParser::TextureParamsSmallContext* ctx) {
    lvt::TextureParamsSmall textureParams;
    textureParams.textureId = boost::lexical_cast<int>(ctx->textureId->getText());
    textureParams.offsX = boost::lexical_cast<float>(ctx->offsX->getText());
    textureParams.offsY = boost::lexical_cast<float>(ctx->offsY->getText());
    return textureParams;
}

lvt::TextureParams parseTextureParams(lvtgrammar::LvtParser::TextureParamsContext* ctx) {
    lvt::TextureParams textureParams;
    static_cast<lvt::TextureParamsSmall&>(textureParams) = parseTextureParamsSmall(ctx->textureParamsSmall());
    textureParams.unused = boost::lexical_cast<float>(ctx->unused->getText());
    return textureParams;
}

class SectorVisitor : public lvtgrammar::LvtParserBaseVisitor {
public:
    LvtLevel level;

    virtual antlrcpp::Any visitTexture(lvtgrammar::LvtParser::TextureContext *ctx) override {
        auto texName = ctx->textureName->getText();
        level.textures.push_back(texName);
        return 0;
    }

    virtual antlrcpp::Any visitVertex(lvtgrammar::LvtParser::VertexContext *ctx) override {
        float x = boost::lexical_cast<float>(ctx->x->getText());
        float z = boost::lexical_cast<float>(ctx->z->getText());
        return lvt::Vertex2 { x, z };
    }

    virtual antlrcpp::Any visitWall(lvtgrammar::LvtParser::WallContext *ctx) override {
        lvt::Wall wall;

        wall.id = ctx->wallId->getText();

        wall.v1 = boost::lexical_cast<int>(ctx->v1->getText());
        wall.v2 = boost::lexical_cast<int>(ctx->v2->getText());

        wall.mid = parseTextureParamsSmall(ctx->mid);
        wall.top = parseTextureParamsSmall(ctx->top);
        wall.bot = parseTextureParamsSmall(ctx->bot);
        wall.overlay = parseTextureParamsSmall(ctx->overlay);

        wall.adjoin = boost::lexical_cast<int>(ctx->adjoin->getText());
        wall.mirror = boost::lexical_cast<int>(ctx->mirror->getText());
        wall.dadjoin = boost::lexical_cast<int>(ctx->dadjoin->getText());
        wall.dmirror = boost::lexical_cast<int>(ctx->dmirror->getText());

        wall.flag1 = boost::lexical_cast<int>(ctx->flag1->getText());
        wall.flag2 = boost::lexical_cast<int>(ctx->flag2->getText());

        wall.light = boost::lexical_cast<int>(ctx->light->getText());

        return wall;
    }

    virtual antlrcpp::Any visitSector(lvtgrammar::LvtParser::SectorContext *ctx) override {
        lvt::Sector sector;

        sector.id = ctx->id->getText();
        sector.name = ctx->name ? ctx->name->getText() : "";

        sector.floorY = boost::lexical_cast<float>(ctx->floorY->getText());
        sector.ceilingY = boost::lexical_cast<float>(ctx->ceilingY->getText());

        sector.floorTexture = parseTextureParams(ctx->floorTexture);
        sector.ceilingTexture = parseTextureParams(ctx->ceilingTexture);

        sector.flag1 = boost::lexical_cast<int>(ctx->flag1->getText());
        sector.flag2 = boost::lexical_cast<int>(ctx->flag2->getText());
        sector.flag3 = ctx->flag3 ? boost::lexical_cast<int>(ctx->flag3->getText()) : 0;

        for (auto element : ctx->vertices()->vertex()) {
            antlrcpp::Any el = visitVertex(element);
            sector.vertices.push_back(el); 
        }    

        for (auto element : ctx->walls()->wall()) {
            antlrcpp::Any el = visitWall(element);
            sector.walls.push_back(el); 
        }    

        level.sectors.push_back(sector);

        return 0;
    }
};

LvtLevel loadLvt(const std::string& filePath) {
    std::ifstream stream;
    stream.open(filePath);

    antlr4::ANTLRInputStream input(stream);
    lvtgrammar::LvtLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    lvtgrammar::LvtParser parser(&tokens);    

    lvtgrammar::LvtParser::Lvt_fileContext* tree = parser.lvt_file();

    SectorVisitor sectorVisitor;
    sectorVisitor.visitLvt_file(tree);

    return sectorVisitor.level;
}

struct TexRect {
    int x, y, width, height;
};

struct TexInfo {
    std::unordered_map<std::string, TexRect> textureRects;
    int width;
    int height;
};

void from_json(const nlohmann::json& json, TexInfo& texInfo) {
    texInfo.width = json["meta"]["size"]["w"].get<int>();
    texInfo.height = json["meta"]["size"]["h"].get<int>();

    auto frames = json["frames"];
    for (auto& frame : frames) {
        std::string texName = frame["filename"].get<std::string>();
        TexRect rect = {
            frame["frame"]["x"].get<int>(),
            frame["frame"]["y"].get<int>(),
            frame["frame"]["w"].get<int>(),
            frame["frame"]["h"].get<int>(),
        };

        texInfo.textureRects[texName] = rect;
    }
}

TexInfos loadTexInfos(const std::string& filePath) {
    auto texInfoJson = terminal_editor::readFileAsString(filePath);
    TexInfo texInfo = nlohmann::json::parse(texInfoJson);

    TexInfos texInfos;
    for (const auto& kv : texInfo.textureRects) {
        auto& texName = kv.first;
        auto& rect = kv.second;
        auto tu0 = static_cast<float>(rect.x) / texInfo.width;
        auto tu1 = static_cast<float>(rect.x + rect.width) / texInfo.width;
        auto tv0 = static_cast<float>(rect.y) / texInfo.height;
        auto tv1 = static_cast<float>(rect.y + rect.height) / texInfo.height;
        tv0 = 1.0f - tv0;
        tv1 = 1.0f - tv1;

        TextureUvs uvs { tu0, tv0, tu1, tv1 };
        auto canonicalTexName = canonicalTextureName(texName);
        texInfos[canonicalTexName] = uvs;
    }

    return texInfos;
}

std::string canonicalTextureName(const std::string& textureName) {
    auto texName = textureName.substr(0, textureName.size() - 4); // Cut off the extension.
    std::transform(texName.begin(), texName.end(), texName.begin(), ::tolower);
    return texName;
}

} // namespace lvt
