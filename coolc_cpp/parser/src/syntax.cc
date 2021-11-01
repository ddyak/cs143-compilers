#include "syntax.h"

#include <iostream>
#include <type_traits>
#include <string>

///////////////// printer zone
void PrintFormal(std::size_t offset, const Formal& formal) {
    std::cout << std::string(offset, ' ') << "#" << formal.lineOfCode << std::endl;
    std::cout << std::string(offset, ' ') << "_formal" << std::endl;
    offset += 2;
    std::cout << std::string(offset, ' ') << formal.id.value << std::endl;
    std::cout << std::string(offset, ' ') << formal.type.value << std::endl;
}

void PrintFeature(std::size_t offset, const Feature& feature) {
    if (!feature.isVar) {
        std::cout << std::string(offset, ' ') << "#" << feature.lineOfCode << std::endl;
        std::cout << std::string(offset, ' ') << "_method" << std::endl;
        offset += 2;
        std::cout << std::string(offset, ' ') << feature.id.value << std::endl;
        for (const auto& formal : feature.arguments) {
            PrintFormal(offset, formal);
        }
        std::cout << std::string(offset, ' ') << feature.type.value << std::endl;
        PrintExpression(offset, *(feature.expr));
    } else {
        std::cout << std::string(offset, ' ') << "#" << feature.lineOfCode << std::endl;
        std::cout << std::string(offset, ' ') << "_attr" << std::endl;
        offset += 2;
        std::cout << std::string(offset, ' ') << feature.id.value << std::endl;
        std::cout << std::string(offset, ' ') << feature.type.value << std::endl;
        if (!std::is_same_v<decltype(*feature.expr), NoExpr>) {
            PrintExpression(offset, *(feature.expr));
        } else {
            std::cout << std::string(offset, ' ') << "#" << 0 << std::endl;
            std::cout << std::string(offset, ' ') << "_no_expr" << std::endl;
            std::cout << std::string(offset, ' ') << ": _no_type" << std::endl;
        }
    }
}

void PrintClass(std::size_t offset, const Class& cls) {
    std::cout << std::string(offset, ' ') << "#" << cls.lineOfCode << std::endl;
    std::cout << std::string(offset, ' ') << "_class" << std::endl;
    offset += 2;
    std::cout << std::string(offset, ' ') << cls.id.value << std::endl;
    std::cout << std::string(offset, ' ') << cls.baseClass.value << std::endl;
    std::cout << std::string(offset, ' ') << '"' << cls.filename << '"' << std::endl;
    std::cout << std::string(offset, ' ') << "(" << std::endl;
    for (const auto& feature : cls.features) {
        PrintFeature(offset, *feature);
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
        PrintClass(offset, *cls);
    }
}

void PrintBranchExpr(std::size_t offset, const BranchExpr& branch) {
    std::cout << std::string(offset, ' ') << "#" << branch.lineOfCode << std::endl;
    std::cout << std::string(offset, ' ') << "_branch" << std::endl;
    offset += 2;
    std::cout << std::string(offset, ' ') << branch.id.value << std::endl;
    std::cout << std::string(offset, ' ') << branch.type.value << std::endl;
    PrintExpression(offset, *branch.expr);
}

void PrintExpression(std::size_t offset, const Expression& expression) {
    std::cout << std::string(offset, ' ');
    std::cout << "#" << expression.lineOfCode << std::endl;
    std::cout << std::string(offset, ' ');
    offset += 2;

    std::visit(overloaded{
                   [offset](const DispatchExpr& expr) {
                       if (expr.type.value == "") {
                           std::cout << "_dispatch" << std::endl;
                       } else {
                           std::cout << "_static_dispatch" << std::endl;
                       }
                       PrintExpression(offset, *expr.obj);
                       if (expr.type.value != "") {
                           std::cout << std::string(offset, ' ') << expr.type.value << std::endl;
                       }
                       std::cout << std::string(offset, ' ') << expr.id.value << std::endl;
                       std::cout << std::string(offset, ' ') << "(" << std::endl;
                       for (const auto& arg : expr.arguments) {
                           PrintExpression(offset, *arg);
                       }
                       std::cout << std::string(offset, ' ') << ")" << std::endl;
                   },
                   [offset](const LetExpr& expr) {
                       std::cout << "_let" << std::endl;
                       std::cout << std::string(offset, ' ') << expr.id.value << std::endl;
                       std::cout << std::string(offset, ' ') << expr.type.value << std::endl;
                       PrintExpression(offset, *expr.expr);
                       PrintExpression(offset, *expr.inExpr);
                   },
                   [offset](const AssignExpr& expr) {
                       std::cout << "_assign" << std::endl;
                       std::cout << std::string(offset, ' ') << expr.id.value << std::endl;
                       PrintExpression(offset, *expr.expr);
                   },
                   [offset](const WhileExpr& expr) {
                       std::cout << "_loop" << std::endl;
                       PrintExpression(offset, *expr.predicat);
                       PrintExpression(offset, *expr.trueExpr);
                   },
                   [offset](const NewExpr& expr) {
                       std::cout << "_new" << std::endl;
                       std::cout << std::string(offset, ' ') << expr.type.value << std::endl;
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

// #include <optional>

// std::optional<Expression> ReadExpression() {
//     //
// }

// std::optional<Formal> ReadFormal() {
//     // only for method
//     return {};
// }

// // todo split feature into method/attr
// std::optional<Feature> ReadFeature() {
//     Feature feature;
//     std::string str;
//     std::cin >> str;
//     feature.lineOfCode = std::stoi(str.substr(1, str.size() - 1));
//     std::cin >> str;
//     if (str == "_method") {
//         std::cin >> feature.id.value;
//         while (auto formal = ReadFormal()) {
//             // program.classes.push_back(std::make_shared<Class>(cls.value()));
//         }
//         std::cin >> feature.type.value;
//         // expr
//     } else {
//         std::cin >> feature.id.value;
//         std::cin >> feature.type.value;
//         // expr
//     }

//     return {};
// }

// std::optional<Class> ReadClass() {
//     Class cls;
//     std::string str;
//     std::cin >> str;
//     cls.lineOfCode = std::stoi(str.substr(1, str.size() - 1));
//     std::cin >> str;
//     std::cin >> cls.id.value;
//     std::cin >> cls.baseClass.value;
//     std::cin >> cls.filename;
//     std::cin >> str; // "("

//     while (auto feature = ReadFeature()) {
//         // program.classes.push_back(std::make_shared<Class>(cls.value()));
//     }

//     std::cin >> str; // ")"

//     return cls;
// }

// Program ReadProgram() {
//     Program program;
//     std::string _;
//     std::getline(std::cin, _);
//     std::getline(std::cin, _);

//     while (auto cls = ReadClass()) {
//         program.classes.push_back(std::make_shared<Class>(cls.value()));
//     }

//     return program;
// }
