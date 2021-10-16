#include "lexer.h"

int main(int argc, char **argv) {
    if (argc == 1) {
        std::cerr << "WARN: There are not input files. Usage: ./lexer [files ..]" << std::endl;
        return 0;
    }

    for (int i = 1; i < argc; ++i) {
        std::string filename = argv[i];
        std::cout << "#name \"" << filename << "\"" << std::endl;
        Lexer lex;
        lex.ReadFile(filename);
        lex.PrintResult();
    }

    return 0;
}
