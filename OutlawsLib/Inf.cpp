#include "Inf.h"

#include "InfLexer.h"
#include "InfParser.h"
#include "InfParserBaseVisitor.h"

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

} // namespace outlaws
