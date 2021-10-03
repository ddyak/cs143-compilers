#include "lexer.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: ./lexer <filename>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];

    std::cout << "#name \"" << argv[1] << "\"" << std::endl;

    Lexer lex;
    lex.ReadFile(argv[1]);
    lex.PrintResult();

    return 0;
}
