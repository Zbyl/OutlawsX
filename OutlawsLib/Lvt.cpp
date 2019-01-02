#include "lvt.h"

#include "LvtLexer.h"
#include "LvtParser.h"
#include "LvtParserBaseVisitor.h"

#include <antlr4-runtime.h>

#include <iostream>
#include <vector>
#include <boost/lexical_cast.hpp>

namespace lab_fuse {

struct Vector3 {
    float x, y, z;
};

class LvtVisitor : public lvtgrammar::LvtParserBaseVisitor {
public:
    virtual antlrcpp::Any visitVertex(lvtgrammar::LvtParser::VertexContext *ctx) override {
        float x = boost::lexical_cast<float>(ctx->x->getText());
        float z = boost::lexical_cast<float>(ctx->z->getText());
        return Vector3 { x, 0, z };
    }

    virtual antlrcpp::Any visitSector(lvtgrammar::LvtParser::SectorContext *ctx) override {
        std::vector<Vector3> vertices;

        for (auto element : ctx->vertices()->vertex()) {                
            antlrcpp::Any el = visitVertex(element);

            vertices.push_back(el); 
        }    

        //antlrcpp::Any result = Scene(ctx->name()->NAME()->getText(), elements);

        return visitChildren(ctx);
    }
};

int loadLvt(const std::string& filePath) {
    std::ifstream stream;
    stream.open(filePath);

    antlr4::ANTLRInputStream input(stream);
    lvtgrammar::LvtLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    lvtgrammar::LvtParser parser(&tokens);    

    lvtgrammar::LvtParser::Lvt_fileContext* tree = parser.lvt_file();

    LvtVisitor visitor;
    visitor.visitLvt_file(tree);

    return 0;
}

} // namespace lab_fuse
