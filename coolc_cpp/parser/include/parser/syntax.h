#pragma once

#include <vector>
#include <memory>
#include <variant>

const std::size_t INVALID_LINE_OF_CODE = 1000000000;

struct Expression;

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

struct Type {
    std::string value;
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
    IdentifierExpr id;
    std::shared_ptr<Expression> expr;
};

struct NewExpr {
    Type type;
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
    IdentifierExpr id;
    Type type;
    std::shared_ptr<Expression> expr;
    std::shared_ptr<Expression> inExpr;
};

struct BranchExpr {
    IdentifierExpr id;
    Type type;
    std::shared_ptr<Expression> expr;

    std::size_t lineOfCode = INVALID_LINE_OF_CODE;
};

struct Case {
    std::shared_ptr<Expression> expr;
    std::vector<std::shared_ptr<BranchExpr>> branches;
};

struct BlockExpr {
    std::vector<std::shared_ptr<Expression>> exprs;
};

struct DispatchExpr {
    std::shared_ptr<Expression> obj;
    Type type;  // static dispatch
    IdentifierExpr id;
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

struct Formal {
    IdentifierExpr id;
    Type type;

    std::size_t lineOfCode = INVALID_LINE_OF_CODE;
};

struct Feature {
    IdentifierExpr id;
    std::vector<Formal> arguments; // for method
    Type type;
    std::shared_ptr<Expression> expr;

    std::size_t lineOfCode = INVALID_LINE_OF_CODE;
    bool isAttr;
};

struct Class {
    Type id;
    Type baseClass = Type{"Object"};
    std::vector<std::shared_ptr<Feature>> features;

    std::string filename;
    std::size_t lineOfCode = INVALID_LINE_OF_CODE;
};

struct Program {
    std::vector<std::shared_ptr<Class>> classes;
};

///////////////// writer
void PrintProgram(const Program& program); // only this public
// private:
void PrintClass(std::size_t offset, const Class& cls);
void PrintFeature(std::size_t offset, const Feature& feature);
void PrintFormal(std::size_t offset, const Formal& formal);
void PrintExpression(std::size_t offset, const Expression& expression);
void PrintBranchExpr(std::size_t offset, const BranchExpr& branch);

///////////////// reader
Program ReadProgram(); // only this public
// private:
// void PrintClass(std::size_t offset, const Class& cls);
// void PrintFeature(std::size_t offset, const Feature& feature);
// void PrintFormal(std::size_t offset, const Formal& formal);
// void PrintExpression(std::size_t offset, const Expression& expression);
// void PrintBranchExpr(std::size_t offset, const BranchExpr& branch);


// helper type for the visitor
template <class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;
