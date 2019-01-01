#include "lvt.h"

#include "lvtLexer.h"
#include "lvtParser.h"
#include "lvtVisitor.h"

#include <antlr4-runtime.h>

#include <iostream>
#include <vector>

namespace lab_fuse {

class LvtVisitor : public lvtgrammar::lvtVisitor {
public:
    antlrcpp::Any visitFile(lvtgrammar::lvtParser::FileContext *context) override;

    antlrcpp::Any visitName(lvtgrammar::lvtParser::NameContext *context) override { return 0; }

    antlrcpp::Any visitAction(lvtgrammar::lvtParser::ActionContext *context) override { return 0; }

    antlrcpp::Any visitSize(lvtgrammar::lvtParser::SizeContext *context) override { return 0; }

    antlrcpp::Any visitShape(lvtgrammar::lvtParser::ShapeContext *context) override { return 0; }

    antlrcpp::Any visitColor(lvtgrammar::lvtParser::ColorContext *context) override { return 0; }

    antlrcpp::Any visitPosition(lvtgrammar::lvtParser::PositionContext *context) override { return 0; }
};

antlrcpp::Any LvtVisitor::visitFile(lvtgrammar::lvtParser::FileContext *ctx) {
    std::vector<int> elements;

    for (auto element : ctx->elements) {                
        antlrcpp::Any el = visitAction(element);

        elements.push_back(el); 
    }    

    //antlrcpp::Any result = Scene(ctx->name()->NAME()->getText(), elements);

    return elements;
}

int loadLvt(const std::string& filePath) {
    std::ifstream stream;
    stream.open(filePath);

    antlr4::ANTLRInputStream input(stream);
    lvtgrammar::lvtLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    lvtgrammar::lvtParser parser(&tokens);    

    lvtgrammar::lvtParser::FileContext* tree = parser.file();

    LvtVisitor visitor;
    std::vector<int> scene = visitor.visitFile(tree);

    return 0;
}

} // namespace lab_fuse
