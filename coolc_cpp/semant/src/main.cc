#include <iostream>

#include "parser/syntax.h"
#include "inheritance.h"


int main() {
    Program program = ReadProgram();
    // PrintProgram(program);
    InheritanceAnalyzer inherAnalyzer(program);
    if (!inherAnalyzer.checkCorrectness()) {
        std::cerr << "Compilation halted due to static semantic errors." << std::endl;
        return 1;
    }
    return 0;
}