#pragma once

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <cassert>

enum class TokenType {
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
    PROGRAM,
    EOFILE
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

    friend inline std::ostream &operator<<(std::ostream &out, const Token &token);
    friend inline std::istream &operator>>(std::istream &in, Token &token);
};

inline std::istream &operator>>(std::istream &in, Token &token) {
    std::string sharpPart;
    if (!(in >> sharpPart)) {
        return in;
    }
    
    if (sharpPart[0] != '#') {
        assert(false);
    }
    sharpPart = sharpPart.substr(1, sharpPart.size() - 1);
     
    if (sharpPart.find_first_not_of("0123456789") != std::string::npos) {
        std::string filename;
        in >> filename;
        filename = filename.substr(1, filename.size() - 2);
        token.tokenType = TokenType::PROGRAM;
        token.rawValue = filename;
        return in;
    }
    token.lineOfCode = std::stoi(sharpPart);

    std::string tokenTypeOrLiteral;
    in >> tokenTypeOrLiteral;
    if (tokenTypeOrLiteral[0] == '\'') {
        token.tokenType = TokenType::PUNCTUATION;
        token.rawValue = tokenTypeOrLiteral.substr(1, tokenTypeOrLiteral.size() - 2);
        return in;
    }

    std::map<std::string, TokenType> NameTokenType;
    for (const auto& [k, v]: TokenTypeName) {
        NameTokenType[v] = k;
    }
    
    if (!NameTokenType.count(tokenTypeOrLiteral)) assert(false);

    token.tokenType = NameTokenType[tokenTypeOrLiteral];

    static const std::set<TokenType> print_raws_tokens = {
        TokenType::STR_CONST,
        TokenType::INT_CONST,
        TokenType::ERROR,
        TokenType::BOOL_CONST,
        TokenType::OBJECTID,
        TokenType::TYPEID,
    };

    if (print_raws_tokens.count(token.tokenType)) {
        std::getline(std::cin, token.rawValue);
        auto start = token.rawValue.find_first_not_of(' ');
        auto end = token.rawValue.find_last_not_of(' ');
        std::string trimmedString;
        token.rawValue = token.rawValue.substr(start, (end - start) + 1);
    }

    return in;
}

inline std::ostream &operator<<(std::ostream &out, const Token &token) {
    if (token.tokenType == TokenType::PROGRAM) {
        out << "#name \"" << token.rawValue << "\"";
        return out;
    }
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
            token.tokenType == TokenType::ERROR) {
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
