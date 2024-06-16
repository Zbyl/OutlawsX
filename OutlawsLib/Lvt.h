#pragma once

#include "geometry.h"

#include <string>
#include <vector>
#include <cstdint>

namespace outlaws {

std::string flagToStringImpl(const char* bitNames[32], uint32_t value);

/// First flag of a Sector.
/// @note When modifying update implementation of flagToString().
#if 0
// This stuff is Dark Forces version, kind of.
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

    SLOPED_FLOOR = (1u << 30),
    SLOPED_CEILING = (1u << 31),
};
#else
// This stuff is Outlaws version, kind of.
// Mostly based on Flag Editor for Sector Flags in LawMaker.
// @note UNKNOWNs may be some flags used in DF, but not present below.
enum class SectorFlag1 : uint32_t {
    EXTERIOR_NO_CEIL = (1u << 0),   // SKY
    EXTERIOR_NO_FLOOR = (1u << 1),  // PIT
    EXTERIOR_CEIL_ADJOIN = (1u << 2),   /// PIT adjoin
    EXTERIOR_FLOOR_ADJOIN = (1u << 3),  /// PIT adjoin
    NO_WALL_DRAW = (1u << 4),
    NO_SLIDING_ON_SLOPE = (1u << 5),
    VELOCITY_FOR_FLOOR_ONLY = (1u << 6),
    UNDERWATER = (1u << 7),
    DOOR = (1u << 8),
    DOOR_REVERSE = (1u << 9),
    UNKNOWN_10 = (1u << 10), // Used on sector 78 and 178 in CANYON.
    UNKNOWN_11 = (1u << 11),
    SECRET_AREA = (1u << 12), // See SECRET_TAG below!
    UNKNOWN_13 = (1u << 13),
    UNKNOWN_14 = (1u << 14),
    UNKNOWN_15 = (1u << 15),
    UNKNOWN_16 = (1u << 16),
    UNKNOWN_17 = (1u << 17),
    LOW_DAMAGE = (1u << 18),
    HIGH_DAMAGE = (1u << 19),
    DEADLY_DAMAGE = (1u << 20),
    LOW_FLOOR_DAMAGE = (1u << 21),
    HIGH_FLOOR_DAMAGE = (1u << 22),
    DEADLY_FLOOR_DAMAGE = (1u << 23),
    DEAN_WERMER_FLAG = (1u << 24),
    SECRET_TAG = (1u << 25), // See SECRET_AREA above!
    DONT_SHADE_FLOOR = (1u << 26),
    RAIL_TRACK_PULL_CHAIN = (1u << 27),
    RAIL_LINE = (1u << 28),
    HIDE_ON_MAP = (1u << 29),
    SLOPED_FLOOR = (1u << 30),
    SLOPED_CEILING = (1u << 31),
};
#endif

#if 0
// This stuff is Dark Forces version, kind of.
enum class WallFlag1 : uint32_t {
    ADJOINING_MID_TX = (1u << 0),
    ILLUMINATED_SIGN = (1u << 1),
    FLIP_TEXTURE_HORIZONTALLY = (1u << 2),
    ELEV_CAN_CHANGE_WALL_LIGHT = (1u << 3),
    WALL_TX_ANCHORED = (1u << 4),
    WALL_MORPHS_WITH_ELEV = (1u << 5),
    ELEV_CAN_SCROLL_TOP_TX = (1u << 6),
    ELEV_CAN_SCROLL_MID_TX = (1u << 7),
    ELEV_CAN_SCROLL_BOT_TX = (1u << 8),
    ELEV_CAN_SCROLL_SIGN_TX = (1u << 9),
    HIDE_ON_MAP = (1u << 10),
    SHOW_AS_NORMAL_ON_MAP = (1u << 11),
    SIGN_ANCHORED = (1u << 12),
    WALL_DAMAGES_PLAYER = (1u << 13), ///< I think this is actually PLAYER_DAMAGES_WALL. After player hits wall ATX texture is advanced to next stop.
    SHOW_AS_LEDGE_ON_MAP = (1u << 14),
    SHOW_AS_DOOR_ON_MAP = (1u << 15),
};

enum class WallFlag2 : uint32_t {
    PLAYER_CAN_CLIMB_ANY_HEIGHT = (1u << 0),
    NO_ONE_CAN_WALK = (1u << 1),
    ENEMIES_CANT_WALK = (1u << 2),
    CANNOT_SHOT_THROUGH = (1u << 3),
};
#else
// This stuff is Outlaws version, kind of.
// Mostly based on Flag Editor for Wall Flags in LawMaker.
enum class WallFlag1 : uint32_t {
    ADJOINING_MID_TX = (1u << 0),
    ILLUMINATED_SIGN = (1u << 1),
    FLIP_TEXTURE_HORIZONTALLY = (1u << 2),
    WALL_TX_ANCHORED = (1u << 3),
    SIGN_TX_ANCHORED = (1u << 4),
    TINTING = (1u << 5),
    WALL_MORPHS_WITH_ELEV = (1u << 6), // Morph with sector in Outlaws. Is it the same?
    ELEV_CAN_SCROLL_TOP_TX = (1u << 7),
    ELEV_CAN_SCROLL_MID_TX = (1u << 8),
    ELEV_CAN_SCROLL_BOT_TX = (1u << 9),
    ELEV_CAN_SCROLL_SIGN_TX = (1u << 10),
    NO_ONE_CAN_WALK = (1u << 11),   // In DF this is in Flag2.
    PLAYER_CAN_CLIMB_ANY_HEIGHT = (1u << 12), // In DF this is in Flag2.
    ENEMIES_CANT_WALK = (1u << 13), // In DF this is in Flag2.
    PLAYER_DAMAGES_WALL = (1u << 14), ///< Shatter. After player hits wall ATX texture is advanced to next stop. I think...
    CANNOT_SHOT_THROUGH = (1u << 15), // In DF this is in Flag2.
    NOT_A_RAIL = (1u << 16), // Don't know what's that.
    HIDE_ON_MAP = (1u << 17),
    ALWAYS_HIDE_ON_MAP = (1u << 18),
    //ELEV_CAN_CHANGE_WALL_LIGHT = (1u << 3),
    //SHOW_AS_NORMAL_ON_MAP = (1u << 11),
    //SHOW_AS_LEDGE_ON_MAP = (1u << 14),
    //SHOW_AS_DOOR_ON_MAP = (1u << 15),
};

// Don't know if this is correct or not...
enum class WallFlag2 : uint32_t {
};
#endif

std::string flagToString(SectorFlag1 value);
std::string flagToString(WallFlag1 value);
std::string flagToString(WallFlag2 value);

struct TextureParamsSmall {
    int textureId;
    float offsX;
    float offsY;
};

struct TextureParams : TextureParamsSmall {
    float angle; // From 0 to 360.
};

/// Parameter of a sloped floor or ceiling.
/// @note Sometimes sector and wall will be 4294967295 (so 0xFFFFFFFF).
///       Don't know what that means yet...
///       But we'll parse it as -1.
struct SlopeParams {
    int sector;     ///< Reference sector.
    int wall;       ///< Reference wall in reference sector. This will be start of the slope.
    int angle;      ///< Angle of the slope (2048 == 45 degrees, positive is up, negative is down).
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
    TextureParams floorOverlayTexture;
    float ceilingY;
    TextureParams ceilingTexture;
    TextureParams ceilingOverlayTexture;

    std::vector<Vertex2> vertices;
    std::vector<Wall> walls;

    int flag1;
    int flag2;
    int flag3;

    SlopeParams floorSlope;
    SlopeParams ceilingSlope;
        
    int layer;
};

inline bool hasFlag(const Wall& wall, WallFlag1 flag) {
    return wall.flag1 & static_cast<uint32_t>(flag);
}

inline bool hasFlag(const Wall& wall, WallFlag2 flag) {
    return wall.flag2 & static_cast<uint32_t>(flag);
}

inline bool hasFlag(const Sector& sector, SectorFlag1 flag) {
    return sector.flag1 & static_cast<uint32_t>(flag);
}

struct LvtLevel {
    std::vector<Sector> sectors;
    std::vector<std::string> textures;
};

LvtLevel loadLvt(const std::string& filePath);
LvtLevel loadLvt(std::istream& stream);

} // namespace outlaws
