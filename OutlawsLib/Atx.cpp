#include "Atx.h"

#include "AtxLexer.h"
#include "AtxParser.h"
#include "AtxParserBaseVisitor.h"

#include "zerrors.h"
#include "zlogging.h"
#include "file_utilities.h"

#include <antlr4-runtime.h>

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <string>
#include <array>
#include <cmath>
#include <cstdint>

#include <boost/lexical_cast.hpp>

namespace outlaws {

AtxItem::AtxItem(int isector, AtxWallKind wallKind, int iwall, const AtxInstructions& instructions)
    : isector(isector)
    , wallKind(wallKind)
    , iwall(iwall)
    , instructions(instructions)
    , nextInstruction(0)
    , timeLeft(0.0f)
    , currentRate(24)   // @note This is just a guess. This needs investigation.
{
}

void AtxItem::trigger(const LvtLevel& level, uint32_t eventMask) {
#if 0
    const lvt::Sector& sector = level.sectors.at(isector);
    const lvt::Wall& wall = sector.walls.at(iwall);

    if (!lvt::hasFlag(wall, lvt::WallFlag1::WALL_DAMAGES_PLAYER)) {
        // Won't trigger if wall doesn't have this flag.
        // @todo Not sure if this is correct.
        return;
    }
#endif

    // @todo Maybe we should check eventMask for EventMasks::DAMAGE?

    triggered = true;
}

void AtxItem::update(const LvtLevel& level, float deltaTime) {
    timeLeft += deltaTime;

    // Process trigger
    if (triggered) {
        triggered = false;

        ZASSERT(nextInstruction >= 0);
        ZASSERT(nextInstruction < static_cast<int>(instructions.size()));

        const AtxInstruction& instruction = instructions[nextInstruction];
        if (instruction.instructionKind == AtxInstructionKind::STOP) {
            // Triggering only affects stopped textures.
            // @note We can skip here STOP befor it was executed. This might not be correct.
            nextInstruction++;
            LOG() << "Skipped STOP.";
        }
    }

    while (true) {
        // Execute all instructions that don't take cycles: RATE, GOTO and START_SOUND.
        // We do this first to make sure RATE is set to a proper value.
        int loopCount = 0;
        while (true) {
            loopCount++;
            if (loopCount > 200) {
                ZTHROW() << "Too many instructions processed: " << loopCount << ". Possibly an infinite loop in the ATX.";
            }

            if (!executeFastInstruction())
                break;
        }

        bool haveNextFrame = timeLeft >= (1.0f / currentRate);
        if (!haveNextFrame)
            return;

        timeLeft -= (1.0f / currentRate);

        executeSlowInstruction();
    }
}

bool AtxItem::executeFastInstruction() {
    ZASSERT(nextInstruction >= 0);
    ZASSERT(nextInstruction < static_cast<int>(instructions.size()));

    const AtxInstruction& instruction = instructions[nextInstruction];
    switch (instruction.instructionKind) {
        case AtxInstructionKind::RATE: {
            currentRate = instruction.num;
            nextInstruction++;
            LOG() << "Set rate to: " << currentRate;
            return true;
        }
        break;

        case AtxInstructionKind::START_SOUND: {
            LOG() << "Playing sound: " << instruction.name;
            nextInstruction++;
            return true;
        }
        break;

        case AtxInstructionKind::GOTO: {
            nextInstruction = instruction.num;
            LOG() << "Jumping to instruction: " << nextInstruction;
            ZASSERT(nextInstruction >= 0);
            ZASSERT(nextInstruction < static_cast<int>(instructions.size()));
            return true;
        }
        break;

        default:
            return false;
    }
}

void AtxItem::executeSlowInstruction() {
    ZASSERT(nextInstruction >= 0);
    ZASSERT(nextInstruction < static_cast<int>(instructions.size()));

    const AtxInstruction& instruction = instructions[nextInstruction];
    switch (instruction.instructionKind) {
        case AtxInstructionKind::TEXTURE: {
            LOG() << "Setting texture: " << instruction.name;
            nextInstruction++;
        }
        break;

        case AtxInstructionKind::STOP: {
            // Just do noting.
            LOG() << "Texture is stopped.";
        }
        break;

        default:
            ZASSERT(false) << "Excpected a slow instruction, but got: " << static_cast<int>(instruction.instructionKind);
    }
}

class  AtxVisitor : public atxgrammar::AtxParserBaseVisitor {
public:
    AtxInstructions instructions;

    virtual antlrcpp::Any visitAtx_file(atxgrammar::AtxParser::Atx_fileContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitAtx_instruction(atxgrammar::AtxParser::Atx_instructionContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitFloat_(atxgrammar::AtxParser::Float_Context *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitRate(atxgrammar::AtxParser::RateContext *ctx) override {
        auto rate = boost::lexical_cast<int>(ctx->rateValue->getText());
        AtxInstruction instruction { AtxInstructionKind::RATE, rate, "" };
        instructions.push_back(instruction);
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitTexture(atxgrammar::AtxParser::TextureContext *ctx) override {
        auto textureName = ctx->textureName->getText();
        AtxInstruction instruction { AtxInstructionKind::TEXTURE, 0, textureName };
        instructions.push_back(instruction);
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitStart_sound(atxgrammar::AtxParser::Start_soundContext *ctx) override {
        auto unusedInt = boost::lexical_cast<int>(ctx->unusedInt->getText());
        auto soundName = ctx->soundName->getText();
        AtxInstruction instruction { AtxInstructionKind::START_SOUND, unusedInt, soundName };
        instructions.push_back(instruction);
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitStop(atxgrammar::AtxParser::StopContext *ctx) override {
        AtxInstruction instruction { AtxInstructionKind::STOP, 0, "" };
        instructions.push_back(instruction);
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitGoto_(atxgrammar::AtxParser::Goto_Context *ctx) override {
        auto instructionIdx = boost::lexical_cast<int>(ctx->instructionIdx->getText());
        AtxInstruction instruction { AtxInstructionKind::GOTO, instructionIdx, "" };
        instructions.push_back(instruction);
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitGoto_stop(atxgrammar::AtxParser::Goto_stopContext *ctx) override {
        LOG() << "Ignoring GOTO STOP instruction, as it is probably a bug.";
        return visitChildren(ctx);
    }
};

AtxInstructions loadAtx(const std::string& filePath) {
    std::ifstream stream;
    stream.open(filePath);

    antlr4::ANTLRInputStream input(stream);
    atxgrammar::AtxLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    atxgrammar::AtxParser parser(&tokens);    

    atxgrammar::AtxParser::Atx_fileContext* tree = parser.atx_file();

    AtxVisitor atxVisitor;
    atxVisitor.visitAtx_file(tree);

    return atxVisitor.instructions;
}

} // namespace outlaws
