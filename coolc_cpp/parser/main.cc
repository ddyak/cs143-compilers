#include <vector>

#include "parser.h"
#include "token.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./parser <lexer_output>" << std::endl;
        return 1;
    }

    TokenType tokenType;
    std::string rawValue;
    std::size_t lineOfCode;

    TokenType token;
    std::vector<Token> tokens = {
        Token{TokenType::INT_CONST, "1", 1},
        Token{TokenType::PUNCTUATION, "+", 2},
        Token{TokenType::STR_CONST, "2", 3},
        Token{TokenType::PUNCTUATION, "*", 4},
        Token{TokenType::STR_CONST, "4", 5},

        // Token{TokenType::PUNCTUATION, "~", 1},
        // Token{TokenType::INT_CONST, "1", 1},
        // Token{TokenType::PUNCTUATION, "+", 1},
        // Token{TokenType::STR_CONST, "1", 1},
        // Token{TokenType::PUNCTUATION, ")", 1},
        // Token{TokenType::PUNCTUATION, "+", 1},
        // Token{TokenType::NOT, "+", 1},
        // Token{TokenType::INT_CONST, "2", 1},
        // Token{TokenType::PUNCTUATION, "*", 1},
        // Token{TokenType::BOOL_CONST, "false", 1},
        // Token{TokenType::LE, "<", 1},
        // Token{TokenType::OBJECTID, "var_a", 1},
        Token{TokenType::EOFILE, "EOF", 1},
    };

    auto expr = Parser(tokens).parseExpression();  //Program();
    PrintExpression(0, expr);

    return EXIT_SUCCESS;
}
