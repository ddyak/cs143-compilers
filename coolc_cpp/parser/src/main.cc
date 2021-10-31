#include <vector>

#include "parser.h"
#include "lexer/token.h"

std::vector<Token> parseInput() {
    std::vector<Token> tokens;
    for (Token token; std::cin >> token; ) {
        tokens.push_back(token);
    }
    tokens.push_back(Token{TokenType::EOFILE, "", 0});
    return tokens;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::cerr << "WARN: Usage: ./lexer [files ..] | ./parser" << std::endl;
    }

    auto tokens = parseInput();
    auto program = Parser(tokens).parseProgram();
    PrintProgram(program);

    return EXIT_SUCCESS;
}
