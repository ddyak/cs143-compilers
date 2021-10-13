#pragma once

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "token.h"

/////////////  forward declataion
struct Program;  // ok
struct Class;    // ok
struct Feature;
struct Formal;  // ok
struct Expression;

struct AssignExpr;
// struct DispatchExpr;
// struct FuncExpr;
struct CondExpr;  // ok
struct WhileExpr;
struct BlockExpr;  // ok
// struct LetExpr;
struct BranchExpr;  // ok
struct Case;        // ok
struct NewExpr;     // ok
struct IsVoidExpr;  // ok

struct PlusExpr;  // ok
struct SubExpr;   // ok
struct MultExpr;  // ok
struct DivExpr;   // ok

struct LeExpr;    // ok
struct LessExpr;  // ok
struct EqExpr;    // ok

struct NegExpr;         // ok
struct NotExpr;         // ok
struct IdentifierExpr;  // ok
struct Type;            // ok
struct IntExpr;         // ok
struct StringExpr;      // ok
struct BoolExpr;        // ok

///////////// end of forward declataion

struct Program {
    std::vector<std::shared_ptr<Class>> classes;
};

struct Class {
    std::shared_ptr<Type> id;
    std::shared_ptr<Type> baseClass;
    std::vector<std::shared_ptr<Feature>> features;
};

struct Feature {
    std::shared_ptr<IdentifierExpr> id;
    std::vector<std::shared_ptr<Formal>> arguments;
    std::shared_ptr<Type> type;

    bool isVar = true;  // var or func
};

struct Formal {
    std::shared_ptr<IdentifierExpr> id;
    std::shared_ptr<Type> type;
};

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
struct AssignExpr : BinaryExpr {};

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

// struct LetExpr {
//     // let ID : TYPE [ <- expr ] [[,ID : TYPE [ <- expr ]]]* in expr
//     std::vector<std::shared_ptr<IdentifierExpr>> ids;
//     std::vector<std::shared_ptr<Type>> types;
//     std::vector<std::shared_ptr<Expression>> exprs;
//     std::shared_ptr<Expression> inExpr;
// };

struct Case {
    std::shared_ptr<Expression> expr;
    std::vector<std::shared_ptr<BranchExpr>> branches;  // at least 1
};

struct BranchExpr {
    std::shared_ptr<IdentifierExpr> id;
    std::shared_ptr<Type> type;
    std::shared_ptr<Expression> expr;
};

struct BlockExpr {
    std::vector<std::shared_ptr<Expression>> exprs;  // at least 1
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

struct Type {
    std::string value;
};

struct Expression {
    std::variant<NegExpr, NotExpr, IsVoidExpr,                                               // unary
                 PlusExpr, SubExpr, MulExpr, DivExpr, EqExpr, LeExpr, LessExpr, AssignExpr,  // binary
                 IntExpr, StringExpr, BoolExpr, IdentifierExpr,                              // literals
                 BlockExpr,
                 CondExpr,
                 WhileExpr,
                 Case,
                 NewExpr>
        data_;

    std::size_t lineOfCode = -5;

    bool IsPlus() const { return std::holds_alternative<PlusExpr>(data_); }
    bool IsMul() const { return std::holds_alternative<MulExpr>(data_); }
    bool IsInt() const { return std::holds_alternative<IntExpr>(data_); }

    const PlusExpr* AsPlus() {
        assert(IsPlus());
        return std::get_if<PlusExpr>(&data_);
    }
};

/* Compare -> Additive < Additive
 * Additive -> T + Additive | T
 * T -> A * T | A
 * A -> INT | '(' expr ')' | not expr
*/

class Parser {
   public:
    explicit Parser(const std::vector<Token>& tokens) { next_ = tokens.cbegin(); }

    Program parseProgram() {
        Program program;
        while (next_->tokenType != TokenType::EOFILE) {
            auto cls = parseClass();
            if (next_->rawValue != ";") assert(false);
            ++next_;
            program.classes.push_back(std::make_shared<Class>(cls));
        }
        return program;
    }

    Class parseClass() {
        //  class TYPE [inherits TYPE] { [[feature; ]]∗
        if (next_->tokenType != TokenType::CLASS) assert(false);
        ++next_;

        Class cls;
        cls.id = std::make_shared<Type>(parseType());
        if (next_->tokenType == TokenType::INHERITS) {
            ++next_;
            cls.baseClass = std::make_shared<Type>(parseType());
        }
        if (next_->rawValue != "{") assert(false);
        ++next_;

        while (next_->rawValue != "}") {
            cls.features.push_back(std::make_shared<Feature>(parseFeature()));
            if (next_->rawValue != ";") assert(false);
            ++next_;
        }
        ++next_;

        return cls;
    }

    Type parseType() {
        if (next_->tokenType != TokenType::TYPEID) assert(false);
        std::string value = next_->rawValue;
        ++next_;
        return Type{value};
    }

    IdentifierExpr parseIdentifier() {
        if (next_->tokenType != TokenType::OBJECTID) assert(false);
        std::string value = next_->rawValue;
        ++next_;
        return IdentifierExpr{value};
    }

    Feature parseFeature() { return {}; }

    Formal parseFormal() {
        auto id = parseIdentifier();
        if (next_->rawValue != ":") assert(false);
        ++next_;
        auto type = parseType();
        return Formal{std::make_shared<IdentifierExpr>(id), std::make_shared<Type>(type)};
    }

    //////////////////////////////////////////

    Expression parseExpression() {
        auto addExpression = parseAdditiveExpression();

        std::size_t lineOfCode = next_->lineOfCode;

        if (next_->tokenType == TokenType::LE) {
            ++next_;
            return Expression{LeExpr{
                                  std::make_shared<Expression>(addExpression),
                                  std::make_shared<Expression>(parseAdditiveExpression())},
                              lineOfCode};
        }
        if (next_->rawValue == "<") {
            ++next_;
            return Expression{LessExpr{
                                  std::make_shared<Expression>(addExpression),
                                  std::make_shared<Expression>(parseAdditiveExpression())},
                              lineOfCode};
        }
        if (next_->rawValue == "=") {
            ++next_;
            return Expression{EqExpr{
                                  std::make_shared<Expression>(addExpression),
                                  std::make_shared<Expression>(parseAdditiveExpression())},
                              lineOfCode};
        }

        return addExpression;
    }

    Expression parseAdditiveExpression() {
        auto term = parseTerm();

        std::size_t lineOfCode = next_->lineOfCode;

        if (next_->rawValue == "+") {
            ++next_;
            return Expression{PlusExpr{
                                  std::make_shared<Expression>(term),
                                  std::make_shared<Expression>(parseAdditiveExpression())},
                              lineOfCode};
        }
        if (next_->rawValue == "-") {
            ++next_;
            return Expression{SubExpr{
                                  std::make_shared<Expression>(term),
                                  std::make_shared<Expression>(parseAdditiveExpression())},
                              lineOfCode};
        }

        return term;
    }

    Expression parseTerm() {
        auto atom = parseAtom();

        std::size_t lineOfCode = next_->lineOfCode;

        if (next_->rawValue == "*") {
            ++next_;
            return Expression{
                MulExpr{std::make_shared<Expression>(atom),
                        std::make_shared<Expression>(parseTerm())},
                lineOfCode};
        }

        if (next_->rawValue == "/") {
            ++next_;
            return Expression{
                DivExpr{std::make_shared<Expression>(atom),
                        std::make_shared<Expression>(parseTerm())},
                lineOfCode};
        }

        return atom;
    }

    Expression parseBlock() {
        ++next_;
        BlockExpr block;
        while (next_->rawValue != "}") {
            auto expr = parseExpression();
            block.exprs.push_back(std::make_shared<Expression>(expr));
            if (next_->rawValue != ";") {
                std::cerr << next_->rawValue << std::endl;
                assert(false);
            }
            ++next_;
        }
        ++next_;
        return Expression{block};
    }

    Expression parseAtom() {
        if (next_->tokenType == TokenType::WHILE) {
            // false;
        }

        if (next_->tokenType == TokenType::OBJECTID) {
            auto objectExpr = parseIdentifier();
            if (next_->tokenType == TokenType::ASSIGN) {
                ++next_;
                return Expression{AssignExpr{std::make_shared<Expression>(Expression{objectExpr}),
                                             std::make_shared<Expression>(parseExpression())}};
            }
            return {objectExpr};
        }

        ///////////////////////////////////////////
        if (next_->tokenType == TokenType::CASE) {
            ++next_;
            Case case_;
            case_.expr = std::make_shared<Expression>(parseExpression());
            if (next_->tokenType != TokenType::OF) assert(false);
            ++next_;
            while (next_->tokenType != TokenType::ESAC) {
                auto id = std::make_shared<IdentifierExpr>(parseIdentifier());
                if (next_->rawValue != ":") assert(false);
                ++next_;
                auto type = std::make_shared<Type>(parseType());
                if (next_->tokenType != TokenType::DARROW) assert(false);
                ++next_;
                auto expr = std::make_shared<Expression>(parseExpression());
                if (next_->rawValue != ";") assert(false);
                ++next_;
                case_.branches.push_back(std::make_shared<BranchExpr>(BranchExpr{id, type, expr}));
            }

            if (case_.branches.empty()) assert(false);
            ++next_;

            return Expression{case_};
        }

        if (next_->tokenType == TokenType::IF) {
            ++next_;
            auto predicat = std::make_shared<Expression>(parseExpression());
            if (next_->tokenType != TokenType::THEN) assert(false);
            auto trueExpr = std::make_shared<Expression>(parseExpression());
            if (next_->tokenType != TokenType::ELSE) assert(false);
            auto falseExpr = std::make_shared<Expression>(parseExpression());
            if (next_->tokenType != TokenType::FI) assert(false);
            ++next_;
            return Expression{CondExpr{predicat, trueExpr, falseExpr}};
        }

        if (next_->tokenType == TokenType::PUNCTUATION && next_->rawValue == "{") {
            return Expression{parseBlock()};
        }

        if (next_->tokenType == TokenType::NEW) {
            ++next_;
            return Expression{NewExpr{std::make_shared<Type>(parseType())}};
        }

        if (next_->tokenType == TokenType::PUNCTUATION && next_->rawValue == "~") {
            ++next_;
            return Expression{NegExpr{std::make_shared<Expression>(parseAtom())}};
        }

        if (next_->tokenType == TokenType::ISVOID) {
            ++next_;
            return Expression{IsVoidExpr{std::make_shared<Expression>(parseAtom())}};
        }

        if (next_->tokenType == TokenType::PUNCTUATION && next_->rawValue == "(") {
            ++next_;
            auto expr = parseExpression();
            if (next_->rawValue != ")") assert(false);
            ++next_;
            return expr;
        }

        if (next_->tokenType == TokenType::NOT) {
            ++next_;
            return Expression{NotExpr{std::make_shared<Expression>(parseExpression())}};
        }

        if (next_->tokenType == TokenType::INT_CONST) {
            int32_t value = std::stoi(next_->rawValue);
            std::size_t lineOfCode = next_->lineOfCode;
            ++next_;
            return Expression{IntExpr{value}, lineOfCode};
        }

        if (next_->tokenType == TokenType::STR_CONST) {
            std::string value = next_->rawValue;
            std::size_t lineOfCode = next_->lineOfCode;
            ++next_;
            return Expression{StringExpr{value}, lineOfCode};
        }

        if (next_->tokenType == TokenType::BOOL_CONST) {
            bool value = next_->rawValue == "true";
            std::size_t lineOfCode = next_->lineOfCode;
            ++next_;
            return Expression{BoolExpr{value}, lineOfCode};
        }

        assert(false);
    }

   private:
    std::vector<Token>::const_iterator next_;
};

///////////////// printer zone!//

// helper type for the visitor #4
template <class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

void PrintExpression(std::size_t offset, const Expression& expression) {
    std::cout << std::string(offset, ' ');
    std::cout << "#" << expression.lineOfCode << std::endl;
    std::cout << std::string(offset, ' ');
    offset += 2;

    std::visit(overloaded{
                   [offset](const AssignExpr& expr) {},
                   [offset](const CondExpr& expr) {},
                   [offset](const WhileExpr& expr) {},
                   [offset](const Case& expr) {},
                   [offset](const NewExpr& expr) {},
                   [offset](const BlockExpr& expr) {
                       std::cout << "block" << std::endl;
                       for (const auto& exp : expr.exprs) {
                           PrintExpression(offset, *exp);
                       }
                   },
                   [offset](const NegExpr& expr) {
                       std::cout << "~" << std::endl;
                       PrintExpression(offset, *expr.rhs);
                   },
                   [offset](const NotExpr& expr) {
                       std::cout << "not" << std::endl;
                       PrintExpression(offset, *expr.rhs);
                   },
                   [offset](const IsVoidExpr& expr) {
                       std::cout << "is void" << std::endl;
                       PrintExpression(offset, *expr.rhs);
                   },
                   [offset](const PlusExpr& expr) {
                       std::cout << "_plus" << std::endl;
                       PrintExpression(offset, *expr.lhs);
                       PrintExpression(offset, *expr.rhs);
                   },
                   [offset](const SubExpr& expr) {
                       std::cout << "_sub" << std::endl;
                       PrintExpression(offset, *expr.lhs);
                       PrintExpression(offset, *expr.rhs);
                   },
                   [offset](const MulExpr& expr) {
                       std::cout << "_mul" << std::endl;
                       PrintExpression(offset, *expr.lhs);
                       PrintExpression(offset, *expr.rhs);
                   },
                   [offset](const DivExpr& expr) {
                       std::cout << "_divide" << std::endl;
                       PrintExpression(offset, *expr.lhs);
                       PrintExpression(offset, *expr.rhs);
                   },
                   [offset](const EqExpr& expr) {
                       std::cout << "=" << std::endl;
                       PrintExpression(offset, *expr.lhs);
                       PrintExpression(offset, *expr.rhs);
                   },
                   [offset](const LeExpr& expr) {
                       std::cout << "<=" << std::endl;
                       PrintExpression(offset, *expr.lhs);
                       PrintExpression(offset, *expr.rhs);
                   },
                   [offset](const LessExpr& expr) {
                       std::cout << "<" << std::endl;
                       PrintExpression(offset, *expr.lhs);
                       PrintExpression(offset, *expr.rhs);
                   },
                   [offset](const IntExpr& expr) {
                       std::cout << "_int" << std::endl;
                       std::cout << std::string(offset, ' ') << expr.value << std::endl;
                   },
                   [offset](const BoolExpr& expr) {
                       std::cout << "_bool" << std::endl;
                       std::cout << std::string(offset, ' ') << expr.value << std::endl;
                   },
                   [offset](const StringExpr& expr) {
                       std::cout << "_string" << std::endl;
                       std::cout << std::string(offset, ' ') << '"' << expr.value << '"' << std::endl;
                   },
                   [offset](const IdentifierExpr& expr) {
                       std::cout << expr.value << std::endl;
                   },
               },
               expression.data_);
    std::cout << std::string(offset - 2, ' ') << ": _no_type" << std::endl;
}
