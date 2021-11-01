#pragma once

#include <memory>
#include <vector>

#include "lexer/token.h"
#include "syntax.h"

class Parser {
   public:
    explicit Parser(const std::vector<Token>& tokens) { next_ = tokens.cbegin(); }
    Program parseProgram();

   private:
    Class parseClass();
    Feature parseFeature();
    Formal parseFormal();
    Expression parseExpression();

    Expression parseAdditiveExpression();
    Expression parseTerm();
    Expression parseAtom();

    BlockExpr parseBlock();
    Expression parseLet();
    Type parseType();
    IdentifierExpr parseIdentifier();

    Expression parseDispatch(const std::shared_ptr<Expression>& obj);

   private:
    std::vector<Token>::const_iterator next_;
    std::string filename_;
};

