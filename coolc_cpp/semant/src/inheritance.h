#pragma once

#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "parser/syntax.h"

struct InheritanceAnalyzer {
   private:
    std::unordered_map<std::string, std::shared_ptr<Class>> id2class;
    const Program& program;

   public:
    InheritanceAnalyzer(const Program& program) : program(program) {}

    bool Initialize() {
        id2class["IO"] = std::make_shared<Class>(Class{"IO", "Object"});
        id2class["Object"] = std::make_shared<Class>(Class{"Object", "root"});
        bool status = true;
        for (const auto& cls : program.classes) {
            const std::string& id = cls->id.value;
            if (id2class.count(id)) {
                std::cerr << cls->filename << ":" << cls->lineOfCode << ": "
                          << "Class " << id << " was previously defined." << std::endl;
                status = false;
            } else {
                id2class[id] = cls;
            }
        }
        return status;
    }

    bool hasUndefined() const {
        bool status = false;
        for (const auto& [id, cls] : id2class) {
            if (id == "Object") {
                continue;
            }
            if (!id2class.count(cls->baseClass.value)) {
                std::cerr << cls->filename << ":" << cls->lineOfCode << ": "
                          << "Class " << id << " inherits from an undefined class " << cls->baseClass.value << "." << std::endl;
                status = true;
            }
        }
        return status;
    }

    bool hasBasicInheritance() const {
        bool status = false;
        static const std::set<std::string> basicClasses = {"String", "Int", "Bool", "SELF_TYPE"};
        for (const auto& [id, cls] : id2class) {
            if (basicClasses.count(cls->baseClass.value)) {
                std::cerr << cls->filename << ":" << cls->lineOfCode << ": "
                          << "Class " << id << " cannot inherit class " << cls->baseClass.value << "." << std::endl;
                status = true;
            }
        }
        return status;
    }

    // graph         /edges
    // path[] int    /vertices
    // colors[] enum /vertices
    // std::string st, end

    // 1. find cycle
    // 2. remove cycle and all daughters from graph
    // 3. repeat
    // 4. sort answers

    bool hasCycle() const {
        std::set<std::string> classesInCycles;
        for (const auto& [id, _] : id2class) {
            if (classesInCycles.count(id)) {
                continue;
            }
            std::set<std::string> path = {id};
            auto baseId = id2class.at(id)->baseClass.value;
            while (baseId != "root") {
                if (classesInCycles.count(baseId) || path.count(baseId)) {
                    std::merge(classesInCycles.begin(), classesInCycles.end(),
                               path.begin(), path.end(),
                               std::inserter(classesInCycles, classesInCycles.end()));
                    break;
                } else {
                    path.insert(baseId);
                    baseId = id2class.at(baseId)->baseClass.value;
                }
            }
        }
        std::vector<std::string> sortedClassesInCycles = {classesInCycles.begin(), classesInCycles.end()};
        std::sort(sortedClassesInCycles.begin(), sortedClassesInCycles.end(),
                  [this](const std::string& lhs, const std::string& rhs) {
                      return id2class.at(lhs)->lineOfCode > id2class.at(rhs)->lineOfCode;
                  });
        for (const auto& id : sortedClassesInCycles) {
            auto cls = id2class.at(id);
            std::cerr << cls->filename << ":" << cls->lineOfCode << ": "
                      << "Class " << id << ", or an ancestor of " << id << ", is involved in an inheritance cycle." << std::endl;
        }

        // ../../../examples/hello_world.cl:8: Class C, or an ancestor of C, is involved in an inheritance cycle.
        return true;
    }

    bool hasMain() const {
        return false;
    }

    bool checkCorrectness() {
        bool isCorrect = Initialize();
        isCorrect = !hasUndefined() && isCorrect;
        isCorrect = !hasBasicInheritance() && isCorrect;
        if (!isCorrect) return false;
        if (hasCycle()) return false;
        if (!hasMain()) return false;

        return true;
    }
};