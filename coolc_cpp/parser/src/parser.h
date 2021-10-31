#pragma once

#include <memory>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "lexer/token.h"

const std::size_t INVALID_LINE_OF_CODE = 1000000000;

/////////////  forward declataion
struct Program;
struct Class;
struct Feature;
struct Formal;
struct Expression;
struct NoExpr;

struct AssignExpr;
struct DispatchExpr;
struct CondExpr;
struct WhileExpr;
struct BlockExpr;
struct LetExpr;
struct BranchExpr;
struct Case;
struct NewExpr;
struct IsVoidExpr;

struct PlusExpr;
struct SubExpr;
struct MulExpr;
struct DivExpr;

struct LeExpr;
struct LessExpr;
struct EqExpr;

struct NegExpr;
struct NotExpr;
struct IdentifierExpr;
struct Type;
struct IntExpr;
struct StringExpr;
struct BoolExpr;
///////////// end of forward declataion

struct Type {
    std::string value;
};

struct Program {
    std::vector<std::shared_ptr<Class>> classes;
};

struct Class {
    std::shared_ptr<Type> id;
    std::shared_ptr<Type> baseClass = std::make_shared<Type>(Type{"Object"});
    std::vector<std::shared_ptr<Feature>> features;

    std::string filename;
    std::size_t lineOfCode = INVALID_LINE_OF_CODE;
};

struct Feature {
    std::shared_ptr<IdentifierExpr> id;
    std::vector<std::shared_ptr<Formal>> arguments;
    std::shared_ptr<Type> type;
    std::shared_ptr<Expression> expr;

    std::size_t lineOfCode = INVALID_LINE_OF_CODE;
    bool isVar;
    bool hasExpr = false;
};

struct Formal {
    std::shared_ptr<IdentifierExpr> id;
    std::shared_ptr<Type> type;

    std::size_t lineOfCode = INVALID_LINE_OF_CODE;
};

struct NoExpr {};

struct UnaryExpr {
    std::shared_ptr<Expression> rhs;
};

struct NotExpr : UnaryExpr {};
struct NegExpr : UnaryExpr {};
struct IsVoidExpr : UnaryExpr {};

struct BinaryExpr {
    std::shared_ptr<Expression> lhs;
    std::shared_ptr<Expression> rhs;
};

struct PlusExpr : BinaryExpr {};
struct SubExpr : BinaryExpr {};
struct MulExpr : BinaryExpr {};
struct DivExpr : BinaryExpr {};
struct EqExpr : BinaryExpr {};
struct LeExpr : BinaryExpr {};
struct LessExpr : BinaryExpr {};

struct AssignExpr {
    std::shared_ptr<IdentifierExpr> id;
    std::shared_ptr<Expression> expr;
};

struct NewExpr {
    std::shared_ptr<Type> type;
};

struct CondExpr {
    std::shared_ptr<Expression> predicat;
    std::shared_ptr<Expression> trueExpr;
    std::shared_ptr<Expression> falseExpr;
};

struct WhileExpr {
    std::shared_ptr<Expression> predicat;
    std::shared_ptr<Expression> trueExpr;
};

struct LetExpr {
    std::shared_ptr<IdentifierExpr> id;
    std::shared_ptr<Type> type;
    std::shared_ptr<Expression> expr;
    std::shared_ptr<Expression> inExpr;
};

struct Case {
    std::shared_ptr<Expression> expr;
    std::vector<std::shared_ptr<BranchExpr>> branches;
};

struct BranchExpr {
    std::shared_ptr<IdentifierExpr> id;
    std::shared_ptr<Type> type;
    std::shared_ptr<Expression> expr;

    std::size_t lineOfCode = INVALID_LINE_OF_CODE;
};

struct BlockExpr {
    std::vector<std::shared_ptr<Expression>> exprs;
};

struct IntExpr {
    int32_t value;
};

struct StringExpr {
    std::string value;
};

struct BoolExpr {
    bool value;
};

struct IdentifierExpr {
    std::string value;
};

struct DispatchExpr {
    std::shared_ptr<Expression> obj;
    std::shared_ptr<Type> type;  // static dispatch
    std::shared_ptr<IdentifierExpr> id;
    std::vector<std::shared_ptr<Expression>> arguments;
};

struct Expression {
    std::variant<NegExpr, NotExpr, IsVoidExpr,                                               // unary
                 PlusExpr, SubExpr, MulExpr, DivExpr, EqExpr, LeExpr, LessExpr, AssignExpr,  // binary
                 IntExpr, StringExpr, BoolExpr, IdentifierExpr,                              // literals
                 BlockExpr,
                 CondExpr,
                 WhileExpr,
                 Case,
                 LetExpr,
                 NewExpr,
                 NoExpr,
                 DispatchExpr>
        data_;

    std::size_t lineOfCode = INVALID_LINE_OF_CODE;
    std::string type = "_no_type";
};

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

///////////////// printer zone

void PrintExpression(std::size_t offset, const Expression& expression);
void PrintFormal(std::size_t offset, const std::shared_ptr<Formal>& formal);
void PrintFeature(std::size_t offset, const std::shared_ptr<Feature>& feature);
void PrintClass(std::size_t offset, const std::shared_ptr<Class>& cls);
void PrintProgram(const Program& program);
void PrintBranchExpr(std::size_t offset, const BranchExpr& branch);

// helper type for the visitor #4
template <class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;
