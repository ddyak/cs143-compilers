#pragma once

#include <iostream>
#include <map>
#include <set>
#include <string>

enum TokenType {
    CLASS,
    ELSE,
    FI,
    IF,
    IN,
    INHERITS,
    ISVOID,
    LET,
    LOOP,
    POOL,
    THEN,
    WHILE,
    CASE,
    ESAC,
    DARROW,
    NEW,
    OF,
    NOT,
    STR_CONST,
    INT_CONST,
    BOOL_CONST,
    TYPEID,
    OBJECTID,
    ERROR,
    LE,
    ASSIGN,
    PUNCTUATION,
};

inline std::map<TokenType, std::string> TokenTypeName = {
    {TokenType::CLASS, "CLASS"},
    {TokenType::ELSE, "ELSE"},
    {TokenType::FI, "FI"},
    {TokenType::IF, "IF"},
    {TokenType::IN, "IN"},
    {TokenType::INHERITS, "INHERITS"},
    {TokenType::ISVOID, "ISVOID"},
    {TokenType::LET, "LET"},
    {TokenType::LOOP, "LOOP"},
    {TokenType::POOL, "POOL"},
    {TokenType::THEN, "THEN"},
    {TokenType::WHILE, "WHILE"},
    {TokenType::CASE, "CASE"},
    {TokenType::ESAC, "ESAC"},
    {TokenType::DARROW, "DARROW"},
    {TokenType::NEW, "NEW"},
    {TokenType::OF, "OF"},
    {TokenType::NOT, "NOT"},
    {TokenType::STR_CONST, "STR_CONST"},
    {TokenType::INT_CONST, "INT_CONST"},
    {TokenType::BOOL_CONST, "BOOL_CONST"},
    {TokenType::TYPEID, "TYPEID"},
    {TokenType::OBJECTID, "OBJECTID"},
    {TokenType::ERROR, "ERROR"},
    {TokenType::LE, "LE"},
    {TokenType::ASSIGN, "ASSIGN"},
    {TokenType::PUNCTUATION, ""},
};

inline std::map<std::string, TokenType> keywords = {
    {"class", TokenType::CLASS},
    {"else", TokenType::ELSE},
    {"fi", TokenType::FI},
    {"if", TokenType::IF},
    {"in", TokenType::IN},
    {"inherits", TokenType::INHERITS},
    {"isvoid", TokenType::ISVOID},
    {"let", TokenType::LET},
    {"loop", TokenType::LOOP},
    {"pool", TokenType::POOL},
    {"then", TokenType::THEN},
    {"while", TokenType::WHILE},
    {"case", TokenType::CASE},
    {"esac", TokenType::ESAC},
    {"new", TokenType::NEW},
    {"of", TokenType::OF},
    {"not", TokenType::NOT},
};

struct Token {
    TokenType tokenType;
    std::string rawValue;
    std::size_t lineOfCode;

    friend inline std::ostream &operator<<(std::ostream &out, const Token &point);
};

inline std::ostream &operator<<(std::ostream &out, const Token &token) {
    static const std::set<TokenType> print_raws_tokens = {
        TokenType::STR_CONST,
        TokenType::INT_CONST,
        TokenType::ERROR,
        TokenType::BOOL_CONST,
        TokenType::OBJECTID,
        TokenType::TYPEID,
    };
    if (print_raws_tokens.count(token.tokenType)) {
        if (token.tokenType == TokenType::STR_CONST ||
            token.tokenType == ERROR) {
            out << "#" << token.lineOfCode << " "
                << TokenTypeName[token.tokenType] << " \"" << token.rawValue
                << "\"";
            ;
        } else {
            out << "#" << token.lineOfCode << " "
                << TokenTypeName[token.tokenType] << " " << token.rawValue;
        }
    } else {
        if (token.tokenType == TokenType::PUNCTUATION) {
            out << "#" << token.lineOfCode << " '" << token.rawValue << "'";
        } else {
            out << "#" << token.lineOfCode << " "
                << TokenTypeName[token.tokenType];
        }
    }

    return out;
}
