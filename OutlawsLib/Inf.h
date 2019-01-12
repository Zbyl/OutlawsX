#pragma once

#include "Lvt.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace outlaws {

enum class EventMasks {
    CROSS_FRONT = 1,    ///<  Cross line from front side.
    CROSS_BACK = 2,     ///<  Cross line from back side.
    ENTER_SECTOR = 4,   ///<  Enter sector.
    LEAVE_SECTOR = 8,   ///<  Leave sector.
    NUDGE_FRONT = 16,   ///<  Nudge line from front side / Nudge sector from inside.
    NUDGE_BACK = 32,    ///<  Nudge line from back side / Nudge sector from outside.
    EXPLOSION = 64,     ///<  Explosion.
    DAMAGE = 256,       ///<  Shoot or punch line (see EntityMasks).
    LAND_ON_FLOOR = 512,///<  Land on floor of sector.
};

enum class EntityMasks : uint32_t {
    ENEMY = 1,          ///<  Enemy.
    WEAPON = 2,         ///<  Weapon.
    PLAYER = (1u << 31),///<  Player.
};

/// Determines whether or not the player moves with a morphing or a horizontally scrolling elevator.
enum class ElevatorFlags : uint32_t {
    MOVE_ON_FLOOR = 1,
    MOVE_ON_2ND_ALTITUDE = 2,
};

class Item {
public:
    /// Empty virtual destructor.
    virtual ~Item() {}

    /// Updates the item.
    /// @param deltaTime    Time in seconds since last update.
    virtual void update(const LvtLevel& level, float deltaTime) = 0;

    /// Triggers the item.
    /// @param eventMask    Object should be triggered only if it matches this event mask.
    virtual void trigger(const LvtLevel& level, uint32_t eventMask) = 0;
};

void loadInf(const std::string& filePath);

} // namespace outlaws
