#pragma once

#include "LevelRuntime.h"
#include "Inf.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace outlaws {

enum class AtxInstructionKind {
    RATE,       ///< `RATE [num]` - sets animation playback rate (presumably in FPS).
    TEXTURE,    ///< `TEXTURE [texture name]` - sets texture to given.
    START_SOUND,///< `START_SOUND [num] [sound name]` - starts given sound.
    STOP,       ///< `STOP` - pauses the animation.
    GOTO,       ///< `GOTO [instruction index]` - goes to instruction with given index.
};

struct AtxInstruction {
    AtxInstructionKind instructionKind;
    int num;
    std::string name;
};

/// Specifies which part of wall/floor/ceiling ATX texture is bound to.
enum class AtxWallKind {
    TOP,
    MID,
    BOT,
    OVERLAY,
    FLOOR,
    CEILING,
    FLOOR_OVERLAY,
    CEILING_OVERLAY,
};

using AtxInstructions = std::vector<AtxInstruction>;

class AtxItem : public Item {
    int isector;                ///< Sector this texture is bound to.
    AtxWallKind wallKind;       ///< Specifies wchich part of wall this ATX is bound to.
    int iwall;                  ///< Wall this texture is bound to. Irrelevant for floors and ceilings.

    AtxInstructions instructions;
    int nextInstruction;        ///< Next instruction to execute.
    float timeLeft;             ///< Unused time left from last update.
    int currentRate;            ///< Current instruction rate, in frames per second.
    bool triggered;             ///< True if this object should be considered triggered in the next update.
public:
    AtxItem(int isector, AtxWallKind wallKind, int iwall, const AtxInstructions& instructions);
        
    virtual void trigger(const LvtLevel& level, uint32_t eventMask) override;

    /// Updates the item.
    /// @param deltaTime    Time in seconds since last update.
    virtual void update(const LvtLevel& level, float deltaTime) override;

private:
    /// Executes one fast instruction: RATE, GOTO or START_SOUND.
    /// @return True if instruction was executed. False otherwise.
    /// @note I assume that only STOP and TEXTURE instructions consume a clock cycle. All other instructions execute immediately. This needs investigation.
    bool executeFastInstruction();

    /// Executes one slow instruction: STOP or TEXTURE.
    /// @note I assume that only STOP and TEXTURE instructions consume a clock cycle. All other instructions execute immediately. This needs investigation.
    void executeSlowInstruction();
};

AtxInstructions loadAtx(const std::string& filePath);

} // namespace outlaws
