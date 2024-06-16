#include "Lvt.h"
#include "file_utilities.h"

#include "LvtLexer.h"
#include "LvtParser.h"
#include "LvtParserBaseVisitor.h"

#include <antlr4-runtime.h>

#include <sstream>

#include <boost/lexical_cast.hpp>

namespace outlaws {

std::string flagToStringImpl(const char* bitNames[32], uint32_t value) {
    std::stringstream ss;

    bool isFirst = true;
    for (int i = 0; i < 32; ++i) {
        uint32_t bitFlag = (1u << i);
        if ((value & bitFlag) == 0) {
            continue;
        }
        if (!isFirst) {
            ss << " ";
        }
        isFirst = false;
        ss << bitNames[i];
    }

    return ss.str();
}

std::string flagToString(SectorFlag1 value) {
    const char* bitNames[32] = {
        "EXTERIOR_NO_CEIL",
        "DOOR",
        "SHOT_REFLECTION",
        "EXTERIOR_ADJOIN",
        "ICE_FLOOR",
        "SNOW_FLOOR",
        "EXPLODING_WALL_OR_DOOR",
        "EXTERIOR_NO_FLOOR",
        "EXTERIOR_FLOOR_ADJOIN",
        "CRUSHING_SECTOR",
        "NO_WALL_DRAW",
        "LOW_DAMAGE",
        "HIGH_DAMAGE",
        "NO_SMART_OBJECT_REACTION",
        "SMART_OBJECT_REACTION",
        "SUBSECTOR",
        "SAFE_SECTOR",
        "RENDERED",
        "PLAYER",
        "SECRET_SECTOR",
        "BIT_20",
        "BIT_21",
        "BIT_22",
        "BIT_23",
        "BIT_24",
        "BIT_25",
        "BIT_26",
        "BIT_27",
        "BIT_28",
        "BIT_29",
        "SLOPED_FLOOR",
        "SLOPED_CEILING",
    };

    return flagToStringImpl(bitNames, static_cast<uint32_t>(value));
}

std::string flagToString(WallFlag1 value) {
    const char* bitNames[32] = {
        "ADJOINING_MID_TX",
        "ILLUMINATED_SIGN",
        "FLIP_TEXTURE_HORIZONTALLY",
        "ELEV_CAN_CHANGE_WALL_LIGHT",
        "WALL_TX_ANCHORED",
        "WALL_MORPHS_WITH_ELEV",
        "ELEV_CAN_SCROLL_TOP_TX",
        "ELEV_CAN_SCROLL_MID_TX",
        "ELEV_CAN_SCROLL_BOT_TX",
        "ELEV_CAN_SCROLL_SIGN_TX",
        "HIDE_ON_MAP",
        "SHOW_AS_NORMAL_ON_MAP",
        "SIGN_ANCHORED",
        "WALL_DAMAGES_PLAYER",
        "SHOW_AS_LEDGE_ON_MAP",
        "SHOW_AS_DOOR_ON_MAP",
        "BIT_16",
        "BIT_17",
        "BIT_18",
        "BIT_19",
        "BIT_20",
        "BIT_21",
        "BIT_22",
        "BIT_23",
        "BIT_24",
        "BIT_25",
        "BIT_26",
        "BIT_27",
        "BIT_28",
        "BIT_29",
        "BIT_30",
        "BIT_31",
    };

    return flagToStringImpl(bitNames, static_cast<uint32_t>(value));
}

std::string flagToString(WallFlag2 value) {
    const char* bitNames[32] = {
        "PLAYER_CAN_CLIMB_ANY_HEIGHT",
        "NO_ONE_CAN_WALK",
        "ENEMIES_CANT_WALK",
        "CANNOT_SHOT_THROUGH",
        "BIT_4",
        "BIT_5",
        "BIT_6",
        "BIT_7",
        "BIT_8",
        "BIT_9",
        "BIT_10",
        "BIT_11",
        "BIT_12",
        "BIT_12",
        "BIT_14",
        "BIT_15",
        "BIT_16",
        "BIT_17",
        "BIT_18",
        "BIT_19",
        "BIT_20",
        "BIT_21",
        "BIT_22",
        "BIT_23",
        "BIT_24",
        "BIT_25",
        "BIT_26",
        "BIT_27",
        "BIT_28",
        "BIT_29",
        "BIT_30",
        "BIT_31",
    };

    return flagToStringImpl(bitNames, static_cast<uint32_t>(value));
}

TextureParamsSmall parseTextureParamsSmall(lvtgrammar::LvtParser::TextureParamsSmallContext* ctx) {
    TextureParamsSmall textureParams;
    textureParams.textureId = boost::lexical_cast<int>(ctx->textureId->getText());
    textureParams.offsX = boost::lexical_cast<float>(ctx->offsX->getText());
    textureParams.offsY = boost::lexical_cast<float>(ctx->offsY->getText());
    return textureParams;
}

TextureParams parseTextureParams(lvtgrammar::LvtParser::TextureParamsContext* ctx) {
    TextureParams textureParams;
    static_cast<TextureParamsSmall&>(textureParams) = parseTextureParamsSmall(ctx->textureParamsSmall());
    textureParams.angle = boost::lexical_cast<float>(ctx->angle->getText());
    return textureParams;
}

/// Parses SlopeParams. If ctx is nullptr returns zero initialized SlopeParams.
SlopeParams parseSlopeParams(lvtgrammar::LvtParser::SlopeParamsContext* ctx) {
    SlopeParams slopeParams = { 0 };
    if (ctx == nullptr)
        return slopeParams;

    slopeParams.sector = (ctx->sectorId->getText() == "4294967295") ? -1 : boost::lexical_cast<int>(ctx->sectorId->getText());
    slopeParams.wall = (ctx->wallId->getText() == "4294967295") ? -1 : boost::lexical_cast<int>(ctx->wallId->getText());
    slopeParams.angle = boost::lexical_cast<int>(ctx->angle->getText());

    return slopeParams;
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
        return Vertex2 { x, z };
    }

    virtual antlrcpp::Any visitWall(lvtgrammar::LvtParser::WallContext *ctx) override {
        Wall wall;

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
        Sector sector;

        sector.id = ctx->id->getText();
        sector.name = ctx->name ? ctx->name->getText() : "";

        sector.floorY = boost::lexical_cast<float>(ctx->floorY->getText());
        sector.ceilingY = boost::lexical_cast<float>(ctx->ceilingY->getText());

        sector.floorTexture = parseTextureParams(ctx->floorTexture);
        sector.floorOverlayTexture = parseTextureParams(ctx->floorOverlayTexture);
        sector.ceilingTexture = parseTextureParams(ctx->ceilingTexture);
        sector.ceilingOverlayTexture = parseTextureParams(ctx->ceilingOverlayTexture);

        sector.flag1 = boost::lexical_cast<int>(ctx->flag1->getText());
        sector.flag2 = boost::lexical_cast<int>(ctx->flag2->getText());
        sector.flag3 = ctx->flag3 ? boost::lexical_cast<int>(ctx->flag3->getText()) : 0;

        sector.floorSlope = parseSlopeParams(ctx->floorSlope);
        sector.ceilingSlope = parseSlopeParams(ctx->ceilingSlope);

        sector.layer = boost::lexical_cast<int>(ctx->layer->getText());

        for (auto element : ctx->vertices()->vertex()) {
            antlrcpp::Any el = visitVertex(element);
            sector.vertices.push_back(std::any_cast<Vertex2>(el)); 
        }    

        for (auto element : ctx->walls()->wall()) {
            antlrcpp::Any el = visitWall(element);
            sector.walls.push_back(std::any_cast<Wall>(el)); 
        }    

        level.sectors.push_back(sector);

        return 0;
    }
};

LvtLevel loadLvt(const std::string& filePath) {
    std::ifstream stream;
    stream.open(filePath);

    return loadLvt(stream);
}

LvtLevel loadLvt(std::istream& stream) {
    antlr4::ANTLRInputStream input(stream);
    lvtgrammar::LvtLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    lvtgrammar::LvtParser parser(&tokens);    

    lvtgrammar::LvtParser::Lvt_fileContext* tree = parser.lvt_file();

    SectorVisitor sectorVisitor;
    sectorVisitor.visitLvt_file(tree);

    return sectorVisitor.level;
}

} // namespace outlaws
