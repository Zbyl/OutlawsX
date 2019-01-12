#include "Lvt.h"
#include "file_utilities.h"

#include "LvtLexer.h"
#include "LvtParser.h"
#include "LvtParserBaseVisitor.h"

#include <antlr4-runtime.h>

#include <boost/lexical_cast.hpp>

namespace outlaws {

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
    textureParams.unused = boost::lexical_cast<float>(ctx->unused->getText());
    return textureParams;
}

/// Parses SlopeParams. If ctx is nullptr returns zero initialized SlopeParams.
SlopeParams parseSlopeParams(lvtgrammar::LvtParser::SlopeParamsContext* ctx) {
    SlopeParams slopeParams = { 0 };
    if (ctx == nullptr)
        return slopeParams;

    slopeParams.sector = boost::lexical_cast<int>(ctx->sectorId->getText());
    slopeParams.wall = boost::lexical_cast<int>(ctx->wallId->getText());
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
        sector.ceilingTexture = parseTextureParams(ctx->ceilingTexture);

        sector.flag1 = boost::lexical_cast<int>(ctx->flag1->getText());
        sector.flag2 = boost::lexical_cast<int>(ctx->flag2->getText());
        sector.flag3 = ctx->flag3 ? boost::lexical_cast<int>(ctx->flag3->getText()) : 0;

        sector.floorSlope = parseSlopeParams(ctx->floorSlope);
        sector.ceilingSlope = parseSlopeParams(ctx->ceilingSlope);

        sector.layer = boost::lexical_cast<int>(ctx->layer->getText());

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

} // namespace outlaws
