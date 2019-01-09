#include "inf.h"

#include "lvt.h"

#include "InfLexer.h"
#include "InfParser.h"
#include "InfParserBaseVisitor.h"

#include "AtxLexer.h"
#include "AtxParser.h"
#include "AtxParserBaseVisitor.h"

#include "zerrors.h"
#include "file_utilities.h"

#include <antlr4-runtime.h>

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <string>
#include <array>
#include <cmath>

#include <boost/lexical_cast.hpp>

namespace inf {

class ItemVisitor : public infgrammar::InfParserBaseVisitor {
public:

    virtual antlrcpp::Any visitInf_file(infgrammar::InfParser::Inf_fileContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitItem(infgrammar::InfParser::ItemContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitClass_(infgrammar::InfParser::Class_Context *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitClassElement(infgrammar::InfParser::ClassElementContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitFloat_(infgrammar::InfParser::Float_Context *ctx) override {
        return visitChildren(ctx);
    }
};

class  AtxVisitor : public atxgrammar::AtxParserBaseVisitor {
public:

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
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitTexture(atxgrammar::AtxParser::TextureContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitStart_sound(atxgrammar::AtxParser::Start_soundContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitStop(atxgrammar::AtxParser::StopContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitGoto_(atxgrammar::AtxParser::Goto_Context *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitGoto_stop(atxgrammar::AtxParser::Goto_stopContext *ctx) override {
        return visitChildren(ctx);
    }
};

void loadInf(const std::string& filePath) {
    std::ifstream stream;
    stream.open(filePath);

    antlr4::ANTLRInputStream input(stream);
    infgrammar::InfLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    infgrammar::InfParser parser(&tokens);    

    infgrammar::InfParser::Inf_fileContext* tree = parser.inf_file();

    ItemVisitor itemVisitor;
    itemVisitor.visitInf_file(tree);
}

void loadAtx(const std::string& filePath) {
    std::ifstream stream;
    stream.open(filePath);

    antlr4::ANTLRInputStream input(stream);
    atxgrammar::AtxLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    atxgrammar::AtxParser parser(&tokens);    

    atxgrammar::AtxParser::Atx_fileContext* tree = parser.atx_file();

    AtxVisitor atxVisitor;
    atxVisitor.visitAtx_file(tree);
}

} // namespace inf
