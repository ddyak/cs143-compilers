#pragma once

#include <set>

#include "token.h"

class Lexer {
   public:
    Lexer() : lineOfCode(1), curr_idx(0), isEof(false) {}

    void ReadFile(const std::string& filename);

    Token ParseInteger();
    Token ParseString();
    Token ParseIdentifier();
    Token ParsePunctuation();
    void ParseLineComment();
    bool ParseMultiLineComment();

    // !contract: NextToken produce token and set curr_idx to next symbol after token
    Token NextToken();

    void PrintResult();

   private:
    bool isSpaceSymbol(char ch) const {
        const std::set<char> spaceSymbols{'\f', '\r', '\t', '\v', ' '};
        return spaceSymbols.count(ch);
    }

    bool isNewLineSymbol(char ch) const {
        return ch == '\n';
    }

   private:
    std::string filename;
    std::string source_code;
    std::size_t lineOfCode;
    std::size_t curr_idx;
    bool isEof;
};
