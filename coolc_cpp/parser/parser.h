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
struct Feature; // ok
struct Formal;  // ok
struct Expression; 
struct NoExpr {}; // ok

struct AssignExpr;
// struct DispatchExpr;
// struct FuncExpr;
struct CondExpr;  // ok
struct WhileExpr;
struct BlockExpr;  // ok
struct LetExpr;
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

// temp

struct Type {
    std::string value;
};

////////

struct Program {
    std::vector<std::shared_ptr<Class>> classes;
};

struct Class {
    std::shared_ptr<Type> id;
    std::shared_ptr<Type> baseClass = std::make_shared<Type>(Type{"Object"});
    std::vector<std::shared_ptr<Feature>> features;

    std::string filename;
    std::size_t lineOfCode = -5;
};

struct Feature {
    std::shared_ptr<IdentifierExpr> id;
    std::vector<std::shared_ptr<Formal>> arguments;
    std::shared_ptr<Type> type;
    std::shared_ptr<Expression> expr;

    std::size_t lineOfCode = -5;
    bool isVar;
    bool hasExpr = false;
};

struct Formal {
    std::shared_ptr<IdentifierExpr> id;
    std::shared_ptr<Type> type;

    std::size_t lineOfCode = -5;
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
    std::vector<std::shared_ptr<BranchExpr>> branches;  // at least 1
};

struct BranchExpr {
    std::shared_ptr<IdentifierExpr> id;
    std::shared_ptr<Type> type;
    std::shared_ptr<Expression> expr;

    std::size_t lineOfCode = -5;
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
                 NoExpr>
        data_;

    std::size_t lineOfCode = -5;
    std::string type = "_no_type";

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
            if (next_->tokenType == TokenType::PROGRAM) {
                filename_ = next_->rawValue;
                ++next_;
            }
            auto cls = parseClass();
            if (next_->rawValue != ";") assert(false);
            ++next_;
            program.classes.push_back(std::make_shared<Class>(cls));
        }
        return program;
    }

    Class parseClass() {
        //  class TYPE [inherits TYPE] { [[feature; ]]*
        if (next_->tokenType != TokenType::CLASS) assert(false);

        Class cls;
        cls.lineOfCode = next_->lineOfCode;
        cls.filename = filename_;

        ++next_;

        cls.id = std::make_shared<Type>(parseType());
        if (next_->tokenType == TokenType::INHERITS) {
            ++next_;
            cls.baseClass = std::make_shared<Type>(parseType());
        }
        if (next_->rawValue != "{") assert(false);
        ++next_;

        while (next_->rawValue != "}") {
            cls.features.push_back(std::make_shared<Feature>(parseFeature()));
            if (next_->rawValue != ";") {
                std::cout << *next_ << std::endl; assert(false);
            }
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

    Feature parseFeature() {
        Feature feature;
        feature.lineOfCode = next_->lineOfCode;
        feature.id = std::make_shared<IdentifierExpr>(parseIdentifier());
        if (next_->rawValue == "(") {
            feature.isVar = false;
            ++next_;
            while (next_->rawValue != ")") {
                feature.arguments.push_back(std::make_shared<Formal>(parseFormal()));
                if (next_->rawValue == ",") {
                    ++next_;
                }
            }
            ++next_;
            if (next_->rawValue != ":") assert(false);
            ++next_;
            feature.type = std::make_shared<Type>(parseType());
            if (next_->rawValue != "{") assert(false);
            ++next_;
            feature.expr = std::make_shared<Expression>(parseExpression());
            if (next_->rawValue != "}") {
                std::cout << *next_ << std::endl;
                assert(false);
            }
            ++next_;
        } else {
            feature.isVar = true;
            if (next_->rawValue != ":") assert(false);
            ++next_;
            feature.type = std::make_shared<Type>(parseType());
            if (next_->tokenType != TokenType::ASSIGN) {
                feature.hasExpr = false;
                feature.expr = std::make_shared<Expression>(Expression{NoExpr{}, 0});
            } else {
                feature.hasExpr = true;
                ++next_;
                feature.expr = std::make_shared<Expression>(parseExpression());
            }
        }
        return feature; 
    }

    Formal parseFormal() {
        std::size_t lineOfCode = next_->lineOfCode;
        auto id = parseIdentifier();
        if (next_->rawValue != ":") assert(false);
        ++next_;
        auto type = parseType();
        return Formal{std::make_shared<IdentifierExpr>(id), std::make_shared<Type>(type), lineOfCode};
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
                                  std::make_shared<Expression>(parseAdditiveExpression())
                                  },
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

    BlockExpr parseBlock() {
        ++next_;
        BlockExpr block;
        while (next_->rawValue != "}") {
            auto expr = parseExpression();
            block.exprs.push_back(std::make_shared<Expression>(expr));
            if (next_->rawValue != ";") {
                std::cerr << *next_ << std::endl;
                assert(false);
            }
            ++next_;
        }
        if (block.exprs.empty()) assert(false);
        ++next_;
        return block;
    }

    Expression parseLet() {
        std::size_t lineOfCode = next_->lineOfCode;

        LetExpr letExpr;

        letExpr.id = std::make_shared<IdentifierExpr>(parseIdentifier());
        if (next_->rawValue != ":") assert(false);
        ++next_;
        letExpr.type = std::make_shared<Type>(parseType());
        if (next_->tokenType == TokenType::ASSIGN) {
            ++next_;
            letExpr.expr = std::make_shared<Expression>(parseExpression());
        } else {
            letExpr.expr = std::make_shared<Expression>(Expression{NoExpr(), 0});
        }
        if (next_->rawValue == ",") {
            ++next_;
            letExpr.inExpr = std::make_shared<Expression>(parseLet());
        } else {
            if (next_->tokenType != TokenType::IN) assert(false);
            ++next_;
            letExpr.inExpr = std::make_shared<Expression>(parseExpression());
        }
        return Expression{letExpr, lineOfCode};
    }

    Expression parseAtom() {
        if (next_->tokenType == TokenType::OBJECTID) {
            std::size_t lineOfCode = next_->lineOfCode;
            auto objectExpr = parseIdentifier();
            if (next_->tokenType == TokenType::ASSIGN) {
                ++next_;
                return Expression{AssignExpr{std::make_shared<IdentifierExpr>(objectExpr),
                                             std::make_shared<Expression>(parseExpression())}, lineOfCode};
            }
            return {objectExpr, lineOfCode};
        }

        if (next_->tokenType == TokenType::LET) {
            ++next_;
            return parseLet();
        }

        if (next_->tokenType == TokenType::WHILE) {
            std::size_t lineOfCode = next_->lineOfCode;
            ++next_;
            auto predicatExpr = parseExpression();

            if (next_->tokenType != TokenType::LOOP) assert(false);
            ++next_;
            auto expr = parseExpression();
            if (next_->tokenType != TokenType::POOL) assert(false);
            ++next_;

            return Expression{WhileExpr{std::make_shared<Expression>(predicatExpr),
                                        std::make_shared<Expression>(expr)}, lineOfCode};
        }

        if (next_->tokenType == TokenType::CASE) {
            std::size_t lineOfCode = next_->lineOfCode;            
            ++next_;
            Case case_;
            case_.expr = std::make_shared<Expression>(parseExpression());
            if (next_->tokenType != TokenType::OF) assert(false);
            ++next_;
            while (next_->tokenType != TokenType::ESAC) {
                std::size_t lineOfCode = next_->lineOfCode;
                auto id = std::make_shared<IdentifierExpr>(parseIdentifier());
                if (next_->rawValue != ":") assert(false);
                ++next_;
                auto type = std::make_shared<Type>(parseType());
                if (next_->tokenType != TokenType::DARROW) assert(false);
                ++next_;
                auto expr = std::make_shared<Expression>(parseExpression());
                if (next_->rawValue != ";") assert(false);
                ++next_;
                case_.branches.push_back(std::make_shared<BranchExpr>(BranchExpr{id, type, expr, lineOfCode}));
            }

            if (case_.branches.empty()) assert(false);
            ++next_;

            return Expression{case_, lineOfCode};
        }

        if (next_->tokenType == TokenType::IF) {
            std::size_t lineOfCode = next_->lineOfCode;
            ++next_;
            auto predicat = std::make_shared<Expression>(parseExpression());
            if (next_->tokenType != TokenType::THEN) assert(false);
            ++next_;
            auto trueExpr = std::make_shared<Expression>(parseExpression());
            if (next_->tokenType != TokenType::ELSE) assert(false);
            ++next_;
            auto falseExpr = std::make_shared<Expression>(parseExpression());
            if (next_->tokenType != TokenType::FI) assert(false);
            ++next_;
            return Expression{CondExpr{predicat, trueExpr, falseExpr}, lineOfCode};
        }

        if (next_->tokenType == TokenType::PUNCTUATION && next_->rawValue == "{") {
            std::size_t lineOfCode = next_->lineOfCode;
            return Expression{parseBlock(), lineOfCode};
        }

        if (next_->tokenType == TokenType::NEW) {
            std::size_t lineOfCode = next_->lineOfCode;
            ++next_;
            return Expression{NewExpr{std::make_shared<Type>(parseType())}, lineOfCode};
        }

        if (next_->tokenType == TokenType::PUNCTUATION && next_->rawValue == "~") {
            std::size_t lineOfCode = next_->lineOfCode;
            ++next_;
            return Expression{NegExpr{std::make_shared<Expression>(parseAtom())}, lineOfCode};
        }

        if (next_->tokenType == TokenType::ISVOID) {
            std::size_t lineOfCode = next_->lineOfCode;
            ++next_;
            return Expression{IsVoidExpr{std::make_shared<Expression>(parseAtom())}, lineOfCode};
        }

        if (next_->tokenType == TokenType::PUNCTUATION && next_->rawValue == "(") {
            ++next_;
            auto expr = parseExpression();
            if (next_->rawValue != ")") assert(false);
            ++next_;
            return expr;
        }

        if (next_->tokenType == TokenType::NOT) {
            std::size_t lineOfCode = next_->lineOfCode;
            ++next_;
            return Expression{NotExpr{std::make_shared<Expression>(parseExpression())}, lineOfCode};
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

        std::cout << *next_ << std::endl;

        assert(false);
    }

   private:
    std::vector<Token>::const_iterator next_;
    std::string filename_;
};

///////////////// printer zone!//

void PrintExpression(std::size_t offset, const Expression& expression);

void PrintFormal(std::size_t offset, const std::shared_ptr<Formal>& formal) {
    std::cout << std::string(offset, ' ') << "#" << formal->lineOfCode << std::endl;
    std::cout << std::string(offset, ' ') << "_formal" << std::endl;
    offset += 2;
    std::cout << std::string(offset, ' ') << formal->id->value << std::endl;
    std::cout << std::string(offset, ' ') << formal->type->value << std::endl;    
}

void PrintFeature(std::size_t offset, const std::shared_ptr<Feature>& feature) {
    if (!feature->isVar) {
        std::cout << std::string(offset, ' ') << "#" << feature->lineOfCode << std::endl;
        std::cout << std::string(offset, ' ') << "_method" << std::endl;
        offset += 2;
        std::cout << std::string(offset, ' ') << feature->id->value << std::endl;
        for (const auto& formal: feature->arguments) {
            PrintFormal(offset, formal);
        }
        std::cout << std::string(offset, ' ') << feature->type->value << std::endl;
        PrintExpression(offset, *(feature->expr));
    } else {
        std::cout << std::string(offset, ' ') << "#" << feature->lineOfCode << std::endl;
        std::cout << std::string(offset, ' ') << "_attr" << std::endl;
        offset += 2;
        std::cout << std::string(offset, ' ') << feature->id->value << std::endl;
        std::cout << std::string(offset, ' ') << feature->type->value << std::endl;
        if (feature->hasExpr) {
            PrintExpression(offset, *(feature->expr));
        } else {
            std::cout << std::string(offset, ' ') << "#" << 0 << std::endl;
            std::cout << std::string(offset, ' ') << "_no_expr" << std::endl;
            std::cout << std::string(offset, ' ') << ": _no_type" << std::endl;
        }
    }
}

void PrintClass(std::size_t offset, const std::shared_ptr<Class>& cls) {
    std::cout << std::string(offset, ' ') << "#" << cls->lineOfCode << std::endl;
    std::cout << std::string(offset, ' ') << "_class" << std::endl;
    offset += 2;
    std::cout << std::string(offset, ' ') << cls->id->value << std::endl;
    std::cout << std::string(offset, ' ') << cls->baseClass->value << std::endl;
    std::cout << std::string(offset, ' ') << '"' << cls->filename << '"' << std::endl;
    std::cout << std::string(offset, ' ') << "(" << std::endl;
    for (const auto& feature : cls->features) {
        PrintFeature(offset, feature);
    }
    std::cout << std::string(offset, ' ') << ")" << std::endl;
}

void PrintProgram(const Program& program) {
    if (program.classes.empty()) {
        return;
    }
    std::cout << "#" << program.classes.front()->lineOfCode << std::endl;
    std::cout << "_program" << std::endl;
    const std::size_t offset = 2;
    for (const auto& cls: program.classes) {
        PrintClass(offset, cls);
    }
}

void PrintBranchExpr(std::size_t offset, const BranchExpr& branch) {
    std::cout << std::string(offset, ' ') << "#" << branch.lineOfCode << std::endl;
    std::cout << std::string(offset, ' ') << "_branch" << std::endl;
    offset += 2;
    std::cout << std::string(offset, ' ') << branch.id->value << std::endl;
    std::cout << std::string(offset, ' ') << branch.type->value << std::endl;
    PrintExpression(offset, *branch.expr);
}

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
                   [offset](const LetExpr& expr) {
                        std::cout << "_let" << std::endl;
                        std::cout << std::string(offset, ' ') << expr.id->value << std::endl;
                        std::cout << std::string(offset, ' ') << expr.type->value << std::endl;
                        PrintExpression(offset, *expr.expr);
                        PrintExpression(offset, *expr.inExpr);
                   },
                   [offset](const AssignExpr& expr) {
                        std::cout << "_assign" << std::endl;
                        std::cout << std::string(offset, ' ') << expr.id->value << std::endl;
                        PrintExpression(offset, *expr.expr);
                   },
                   [offset](const WhileExpr& expr) {
                       std::cout << "_loop" << std::endl;
                        PrintExpression(offset, *expr.predicat);
                        PrintExpression(offset, *expr.trueExpr);
                   },
                   [offset](const NewExpr& expr) {
                        std::cout << "_new" << std::endl;
                        std::cout << std::string(offset, ' ') << expr.type->value << std::endl;
                   },
                   [offset](const CondExpr& expr) {
                        std::cout << "_cond" << std::endl;
                        PrintExpression(offset, *expr.predicat);
                        PrintExpression(offset, *expr.trueExpr);
                        PrintExpression(offset, *expr.falseExpr);                       
                   },
                   [offset](const Case& expr) {
                        std::cout << "_typcase" << std::endl;
                        PrintExpression(offset, *expr.expr);
                        for (const auto& branch: expr.branches) {
                            PrintBranchExpr(offset, *branch);
                        }
                   },
                   [offset](const NoExpr& expr) {
                        std::cout << "_no_expr" << std::endl;
                   },
                   [offset](const BlockExpr& expr) {
                       std::cout << "_block" << std::endl;
                       for (const auto& exp : expr.exprs) {
                           PrintExpression(offset, *exp);
                       }
                   },
                   [offset](const NegExpr& expr) {
                       std::cout << "_neg" << std::endl;
                       PrintExpression(offset, *expr.rhs);
                   },
                   [offset](const NotExpr& expr) {
                       std::cout << "_comp" << std::endl;
                       PrintExpression(offset, *expr.rhs);
                   },
                   [offset](const IsVoidExpr& expr) {
                       std::cout << "_isvoid" << std::endl;
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
                       std::cout << "_eq" << std::endl;
                       PrintExpression(offset, *expr.lhs);
                       PrintExpression(offset, *expr.rhs);
                   },
                   [offset](const LeExpr& expr) {
                       std::cout << "_leq" << std::endl;
                       PrintExpression(offset, *expr.lhs);
                       PrintExpression(offset, *expr.rhs);
                   },
                   [offset](const LessExpr& expr) {
                       std::cout << "_lt" << std::endl;
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
                       std::cout << std::string(offset, ' ') << expr.value << std::endl;
                   },
                   [offset](const IdentifierExpr& expr) {
                       std::cout << "_object" << std::endl;
                       std::cout << std::string(offset, ' ') << expr.value << std::endl;
                   },
               },
               expression.data_);
    std::cout << std::string(offset - 2, ' ') << ": " << expression.type << std::endl;
}
