#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace lvt {

enum class SectorFlag1 : uint32_t {
    EXTERIOR_NO_CEIL = (1u << 0),
    DOOR = (1u << 1),
    SHOT_REFLECTION = (1u << 2),
    EXTERIOR_ADJOIN = (1u << 3),
    ICE_FLOOR = (1u << 4),
    SNOW_FLOOR = (1u << 5),
    EXPLODING_WALL_OR_DOOR = (1u << 6),
    EXTERIOR_NO_FLOOR = (1u << 7),
    EXTERIOR_FLOOR_ADJOIN = (1u << 8),
    CRUSHING_SECTOR = (1u << 9),
    NO_WALL_DRAW = (1u << 10),
    LOW_DAMAGE = (1u << 11),
    HIGH_DAMAGE = (1u << 12),
    NO_SMART_OBJECT_REACTION = (1u << 13),
    SMART_OBJECT_REACTION = (1u << 14),
    SUBSECTOR = (1u << 15),
    SAFE_SECTOR = (1u << 16),
    RENDERED = (1u << 17),
    PLAYER = (1u << 18),
    SECRET_SECTOR = (1u << 19),
};

enum class WallFlag1 : uint32_t {
    ADJOINING_MID_TX = (1u << 0),
    ILLUMINATED_SIGN = (1u << 1),
    FLIP_TEXTURE_HORIZONTALLY = (1u << 2),
    ELEV_CAN_CHANGE_WALL_LIGHT = (1u << 3),
    WALL_TX_ANCHORED = (1u << 4),
    WALL_MARPHS_WITH_ELEV = (1u << 5),
    ELEV_CAN_SCROLL_TOP_TX = (1u << 6),
    ELEV_CAN_SCROLL_MID_TX = (1u << 7),
    ELEV_CAN_SCROLL_BOT_TX = (1u << 8),
    ELEV_CAN_SCROLL_SIGN_TX = (1u << 9),
    HIDE_ON_MAP = (1u << 10),
    SHOW_AS_NORMAL_ON_MAP = (1u << 11),
    SIGN_ANCHORED = (1u << 12),
    WALL_DAMAGES_PLAYER = (1u << 13),
    SHOW_AS_LEDGE_ON_MAP = (1u << 14),
    SHOW_AS_DOOR_ON_MAP = (1u << 15),
};

enum class WallFlag2 : uint32_t {
    PLAYER_CAN_CLIMB_ANY_HEIGHT = (1u << 0),
    NO_ONE_CAN_WALK = (1u << 1),
    ENEMIES_CANT_WALK = (1u << 2),
    CANNOT_SHOT_THROUGH = (1u << 3),
};

struct TextureParamsSmall {
    int textureId;
    float offsX;
    float offsY;
};

struct TextureParams : TextureParamsSmall {
    float unused;
};

struct Vertex2 {
    float x, z;
};

struct Wall {
    std::string id;

    int v1;
    int v2;

    TextureParamsSmall mid;
    TextureParamsSmall top;
    TextureParamsSmall bot;
    TextureParamsSmall overlay;

    int adjoin;
    int mirror;
    int dadjoin;
    int dmirror;

    int flag1;
    int flag2;

    int light;
};

struct Sector {
    std::string id;
    std::string name;

    float floorY;
    TextureParams floorTexture;
    float ceilingY;
    TextureParams ceilingTexture;

    std::vector<Vertex2> vertices;
    std::vector<Wall> walls;

    int flag1;
    int flag2;
    int flag3;
};

struct Uv {
    float u, v;
};

struct Vector3 {
    float x, y, z;
};

struct LvtLevel {
    std::vector<Sector> sectors;
    std::vector<std::string> textures;
};

struct TextureUvs {
    Uv start;
    Uv end;
};

using TexInfos = std::unordered_map<std::string, TextureUvs>;

std::string canonicalTextureName(const std::string& textureName);

void computeWall(const LvtLevel& level, const TexInfos& texInfos, int isector, int iwall, std::vector<Vector3>& vertices, std::vector<Uv>& uvs, std::vector<int>& triangles);
void computeSector(const LvtLevel& level, const TexInfos& texInfos, int isector, std::vector<Vector3>& vertices, std::vector<Uv>& uvs, std::vector<int>& triangles);
void computeLevel(const LvtLevel& level, const TexInfos& texInfos, std::vector<Vector3>& vertices, std::vector<Uv>& uvs, std::vector<int>& triangles);

LvtLevel loadLvt(const std::string& filePath);

TexInfos loadTexInfos(const std::string& filePath);

TextureUvs sectorBounds(const LvtLevel& level, int isector);

} // namespace lvt
