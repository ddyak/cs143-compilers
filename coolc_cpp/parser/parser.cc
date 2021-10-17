#include "parser.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "token.h"

void syntax_error(const Token& token) {
    std::cerr << token << std::endl;
    assert(false);
}

Program Parser::parseProgram() {
    Program program;
    while (next_->tokenType != TokenType::EOFILE) {
        if (next_->tokenType == TokenType::PROGRAM) {
            filename_ = next_->rawValue;
            ++next_;
        }
        auto cls = parseClass();
        if (next_->rawValue != ";") syntax_error(*next_);
        ++next_;
        program.classes.push_back(std::make_shared<Class>(cls));
    }
    return program;
}

Class Parser::parseClass() {
    if (next_->tokenType != TokenType::CLASS) syntax_error(*next_);

    Class cls;
    cls.lineOfCode = next_->lineOfCode;
    cls.filename = filename_;

    ++next_;

    cls.id = std::make_shared<Type>(parseType());
    if (next_->tokenType == TokenType::INHERITS) {
        ++next_;
        cls.baseClass = std::make_shared<Type>(parseType());
    }
    if (next_->rawValue != "{") syntax_error(*next_);
    ++next_;

    while (next_->rawValue != "}") {
        cls.features.push_back(std::make_shared<Feature>(parseFeature()));
        if (next_->rawValue != ";") syntax_error(*next_);
        ++next_;
    }
    ++next_;

    return cls;
}

Type Parser::parseType() {
    if (next_->tokenType != TokenType::TYPEID) syntax_error(*next_);
    std::string value = next_->rawValue;
    ++next_;
    return Type{value};
}

IdentifierExpr Parser::parseIdentifier() {
    if (next_->tokenType != TokenType::OBJECTID) syntax_error(*next_);
    std::string value = next_->rawValue;
    ++next_;
    return IdentifierExpr{value};
}

Feature Parser::parseFeature() {
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
        if (next_->rawValue != ":") syntax_error(*next_);
        ++next_;
        feature.type = std::make_shared<Type>(parseType());
        if (next_->rawValue != "{") syntax_error(*next_);
        ++next_;
        feature.expr = std::make_shared<Expression>(parseExpression());
        if (next_->rawValue != "}") syntax_error(*next_);
        ++next_;
    } else {
        feature.isVar = true;
        if (next_->rawValue != ":") syntax_error(*next_);
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

Formal Parser::parseFormal() {
    std::size_t lineOfCode = next_->lineOfCode;
    auto id = parseIdentifier();
    if (next_->rawValue != ":") syntax_error(*next_);
    ++next_;
    auto type = parseType();
    return Formal{std::make_shared<IdentifierExpr>(id), std::make_shared<Type>(type), lineOfCode};
}

Expression Parser::parseExpression() {
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

Expression Parser::parseAdditiveExpression() {
    auto term = parseTerm();

    while (next_->rawValue == "+" || next_->rawValue == "-") {
        std::size_t lineOfCode = next_->lineOfCode;
        if (next_->rawValue == "+") {
            ++next_;
            auto next_term = parseTerm();
            auto expr = Expression{PlusExpr{
                                std::make_shared<Expression>(term),
                                std::make_shared<Expression>(next_term)},
                            lineOfCode};
            term = std::move(expr);
        } else {
            ++next_;
            auto next_term = parseTerm();
            auto expr = Expression{SubExpr{
                                std::make_shared<Expression>(term),
                                std::make_shared<Expression>(next_term)},
                            lineOfCode};
            term = std::move(expr);
        }
    }

    return term;
}

Expression Parser::parseTerm() {
    auto atom_ = parseAtom();

    while (next_->rawValue == "*" || next_->rawValue == "/") {
        std::size_t lineOfCode = next_->lineOfCode;
        if (next_->rawValue == "*") {
            ++next_;
            auto next_atom = parseAtom();
            auto expr = Expression{MulExpr{
                                std::make_shared<Expression>(atom_),
                                std::make_shared<Expression>(next_atom)},
                            lineOfCode};
            atom_ = std::move(expr);
        } else {
            ++next_;
            auto next_atom = parseAtom();
            auto expr = Expression{DivExpr{
                                std::make_shared<Expression>(atom_),
                                std::make_shared<Expression>(next_atom)},
                            lineOfCode};
            atom_ = std::move(expr);
        }
    }

    return atom_;
}

BlockExpr Parser::parseBlock() {
    ++next_;
    BlockExpr block;
    while (next_->rawValue != "}") {
        auto expr = parseExpression();
        block.exprs.push_back(std::make_shared<Expression>(expr));
        if (next_->rawValue != ";") syntax_error(*next_);
        ++next_;
    }
    if (block.exprs.empty()) syntax_error(*next_);
    ++next_;
    return block;
}

Expression Parser::parseLet() {
    std::size_t lineOfCode = next_->lineOfCode;

    LetExpr letExpr;

    letExpr.id = std::make_shared<IdentifierExpr>(parseIdentifier());
    if (next_->rawValue != ":") syntax_error(*next_);
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
        if (next_->tokenType != TokenType::IN) syntax_error(*next_);
        ++next_;
        letExpr.inExpr = std::make_shared<Expression>(parseExpression());
    }
    return Expression{letExpr, lineOfCode};
}

Expression Parser::parseDispatch(const std::shared_ptr<Expression>& obj) {
    DispatchExpr dispatch;
    dispatch.obj = obj;
    if (next_->rawValue == "@") {
        ++next_;
        dispatch.type = std::make_shared<Type>(parseType());
    }
    if (next_->rawValue != ".") syntax_error(*next_);
    std::size_t lineOfCode = next_->lineOfCode;
    ++next_;
    dispatch.id = std::make_shared<IdentifierExpr>(parseIdentifier());
    if (next_->rawValue != "(") syntax_error(*next_);
    ++next_;
    while (next_->rawValue != ")") {
        dispatch.arguments.push_back(std::make_shared<Expression>(parseExpression()));
        if (next_->rawValue == ")") break;
        if (next_->rawValue != ",") syntax_error(*next_);
        ++next_;
    }
    ++next_;

    auto expr = Expression{dispatch, lineOfCode};

    if (next_->rawValue == "@" || next_->rawValue == ".") {
        return parseDispatch(std::make_shared<Expression>(expr));
    }

    return expr;
}


Expression Parser::parseAtom() {
    if (next_->tokenType == TokenType::OBJECTID) {
        std::size_t lineOfCode = next_->lineOfCode;
        auto objectExpr = parseIdentifier();
        
        if (next_->rawValue == "@" || next_->rawValue == ".") {
            return parseDispatch(std::make_shared<Expression>(Expression{objectExpr, lineOfCode}));
        }

        if (next_->rawValue == "(") {
            ++next_;
            DispatchExpr dispatch;
            dispatch.obj = std::make_shared<Expression>(Expression{IdentifierExpr{"self"}, next_->lineOfCode}); 
            dispatch.id = std::make_shared<IdentifierExpr>(objectExpr);
            if (next_->rawValue != ")") {
                while (true) {
                    dispatch.arguments.push_back(std::make_shared<Expression>(parseExpression()));
                    if (next_->rawValue == ")") break;
                    if (next_->rawValue != ",") syntax_error(*next_);
                    ++next_;
                }
            }
            ++next_;

            auto expr = Expression{dispatch, lineOfCode};

            if (next_->rawValue == "@" || next_->rawValue == ".") {
                return parseDispatch(std::make_shared<Expression>(expr));
            }

            return expr;
        }

        if (next_->tokenType == TokenType::ASSIGN) {
            ++next_;
            return Expression{AssignExpr{std::make_shared<IdentifierExpr>(objectExpr),
                                            std::make_shared<Expression>(parseExpression())},
                                lineOfCode};
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

        if (next_->tokenType != TokenType::LOOP) syntax_error(*next_);
        ++next_;
        auto expr = parseExpression();
        if (next_->tokenType != TokenType::POOL) syntax_error(*next_);
        ++next_;

        return Expression{WhileExpr{std::make_shared<Expression>(predicatExpr),
                                    std::make_shared<Expression>(expr)},
                            lineOfCode};
    }

    if (next_->tokenType == TokenType::CASE) {
        std::size_t lineOfCode = next_->lineOfCode;
        ++next_;
        Case case_;
        case_.expr = std::make_shared<Expression>(parseExpression());
        if (next_->tokenType != TokenType::OF) syntax_error(*next_);
        ++next_;
        while (next_->tokenType != TokenType::ESAC) {
            std::size_t lineOfCode = next_->lineOfCode;
            auto id = std::make_shared<IdentifierExpr>(parseIdentifier());
            if (next_->rawValue != ":") syntax_error(*next_);
            ++next_;
            auto type = std::make_shared<Type>(parseType());
            if (next_->tokenType != TokenType::DARROW) syntax_error(*next_);
            ++next_;
            auto expr = std::make_shared<Expression>(parseExpression());
            if (next_->rawValue != ";") syntax_error(*next_);
            ++next_;
            case_.branches.push_back(std::make_shared<BranchExpr>(BranchExpr{id, type, expr, lineOfCode}));
        }

        if (case_.branches.empty()) syntax_error(*next_);
        ++next_;

        return Expression{case_, lineOfCode};
    }

    if (next_->tokenType == TokenType::IF) {
        std::size_t lineOfCode = next_->lineOfCode;
        ++next_;
        auto predicat = std::make_shared<Expression>(parseExpression());
        if (next_->tokenType != TokenType::THEN) syntax_error(*next_);
        ++next_;
        auto trueExpr = std::make_shared<Expression>(parseExpression());
        if (next_->tokenType != TokenType::ELSE) syntax_error(*next_);
        ++next_;
        auto falseExpr = std::make_shared<Expression>(parseExpression());
        if (next_->tokenType != TokenType::FI) syntax_error(*next_);
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
        if (next_->rawValue != ")") syntax_error(*next_);
        ++next_;

        if (next_->rawValue == "@" || next_->rawValue == ".") {
            return parseDispatch(std::make_shared<Expression>(Expression{expr}));
        }

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

        auto expr = Expression{IntExpr{value}, lineOfCode};

        if (next_->rawValue == "@" || next_->rawValue == ".") {
            return parseDispatch(std::make_shared<Expression>(expr));
        }

        return Expression{IntExpr{value}, lineOfCode};
    }

    if (next_->tokenType == TokenType::STR_CONST) {
        std::string value = next_->rawValue;
        std::size_t lineOfCode = next_->lineOfCode;
        ++next_;

        auto expr = Expression{StringExpr{value}, lineOfCode};

        if (next_->rawValue == "@" || next_->rawValue == ".") {
            return parseDispatch(std::make_shared<Expression>(expr));
        }

        return expr;
    }

    if (next_->tokenType == TokenType::BOOL_CONST) {
        bool value = next_->rawValue == "true";
        std::size_t lineOfCode = next_->lineOfCode;
        ++next_;

        auto expr = Expression{BoolExpr{value}, lineOfCode};

        if (next_->rawValue == "@" || next_->rawValue == ".") {
            return parseDispatch(std::make_shared<Expression>(expr));
        }

        return Expression{BoolExpr{value}, lineOfCode};
    }

    syntax_error(*next_);
    return {};
}

///////////////// printer zone

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
        for (const auto& formal : feature->arguments) {
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
    for (const auto& cls : program.classes) {
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

void PrintExpression(std::size_t offset, const Expression& expression) {
    std::cout << std::string(offset, ' ');
    std::cout << "#" << expression.lineOfCode << std::endl;
    std::cout << std::string(offset, ' ');
    offset += 2;

    std::visit(overloaded{
                   [offset](const DispatchExpr& expr) {
                       if (!expr.type) {
                           std::cout << "_dispatch" << std::endl;
                       } else {
                           std::cout << "_static_dispatch" << std::endl;
                       }
                       PrintExpression(offset, *expr.obj);
                       if (expr.type) {
                           std::cout << std::string(offset, ' ') << expr.type->value << std::endl;
                       }
                       std::cout << std::string(offset, ' ') << expr.id->value << std::endl;
                       std::cout << std::string(offset, ' ') << "(" << std::endl;
                       for (const auto& arg : expr.arguments) {
                           PrintExpression(offset, *arg);
                       }
                       std::cout << std::string(offset, ' ') << ")" << std::endl;
                   },
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
                       for (const auto& branch : expr.branches) {
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
