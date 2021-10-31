#include "lexer.h"

#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>

#include "lexer/token.h"

extern std::map<TokenType, std::string> TokenTypeName;
extern std::map<std::string, TokenType> keywords;

void Lexer::ReadFile(const std::string& filename) {
    std::ifstream t(filename);
    std::stringstream buffer;
    buffer << t.rdbuf();
    source_code = buffer.str();
}

Token Lexer::ParseInteger() {
    std::size_t begin_idx = curr_idx;
    while (curr_idx + 1 != source_code.size() &&
           isdigit(source_code[curr_idx + 1])) {
        ++curr_idx;
    }
    const auto size = curr_idx - begin_idx + 1;
    const auto& rawInteger = source_code.substr(begin_idx, size);
    ++curr_idx;
    return Token{TokenType::INT_CONST, rawInteger, lineOfCode};
}

Token Lexer::ParseString() {
    ++curr_idx;
    std::string str;
    while (true) {
        if (curr_idx == source_code.size()) {
            return Token{TokenType::ERROR, "EOF in string constant", lineOfCode};
        }
        if (source_code[curr_idx] == '\n') {
            ++lineOfCode;
            curr_idx += 1;
            return Token{TokenType::ERROR, "Unterminated string constant",
                         lineOfCode};
        }
        if (source_code[curr_idx] == '\0') {
            curr_idx += 1;
            return Token{TokenType::ERROR, "String contains null character.",
                         lineOfCode};
        }
        if (source_code[curr_idx] == '\"') {
            ++curr_idx;
            return Token{TokenType::STR_CONST, str, lineOfCode};
        }

        std::map<char, std::string> escaped_chars = {
            {'\t', "t"},
            {'\f', "f"},
            {'\b', "b"},
            {char(013), "013"},
            {char(015), "015"},
            {char(022), "022"},
            {char(033), "033"},
        };

        if (escaped_chars.count(source_code[curr_idx])) {
            str += "\\";
            str += escaped_chars[source_code[curr_idx]];
            ++curr_idx;
            continue;
        }

        if (source_code[curr_idx] == '\\') {
            if (curr_idx + 1 == source_code.size()) {
                ++curr_idx;
                return Token{TokenType::ERROR, "EOF in string constant", lineOfCode};
            }
            ++curr_idx;
            const std::set<char> force_escaped_chars = {'b', 't', 'n', 'f', '\"', '\\'};
            if (force_escaped_chars.count(source_code[curr_idx])) {
                str += '\\';
                str += source_code[curr_idx];
            } else if (escaped_chars.count(source_code[curr_idx])) {
                str += "\\";
                str += escaped_chars[source_code[curr_idx]];
                ++curr_idx;
                continue;
            } else if (source_code[curr_idx] == '\n') {
                ++lineOfCode;
                str += "\\n";
            } else if (source_code[curr_idx] == '\0') {
                return Token{TokenType::ERROR, "String contains escaped null character.",
                             lineOfCode};
            } else {
                str += source_code[curr_idx];
            }
            ++curr_idx;
            continue;
        } else {
            str += source_code[curr_idx];
            ++curr_idx;
        }
    }

    return Token{TokenType::ERROR, "String not parsed", lineOfCode};
}

Token Lexer::ParseIdentifier() {
    // Type identifiers begin with a capital letter; object identifiers begin
    // with a lower case letter.
    std::size_t begin_idx = curr_idx;
    while (curr_idx + 1 != source_code.size() &&
           (std::isalnum(source_code[curr_idx + 1]) ||
            source_code[curr_idx + 1] == '_')) {
        ++curr_idx;
    }
    const auto size = curr_idx - begin_idx + 1;
    const auto& rawIdentifier = source_code.substr(begin_idx, size);
    ++curr_idx;

    auto lower_indetifier = rawIdentifier;
    std::transform(lower_indetifier.begin(), lower_indetifier.end(),
                   lower_indetifier.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (keywords.count(lower_indetifier)) {
        return Token{keywords[lower_indetifier], rawIdentifier, lineOfCode};
    }

    if (lower_indetifier[0] == rawIdentifier[0] &&
        (lower_indetifier == "false" || lower_indetifier == "true")) {
        return Token{TokenType::BOOL_CONST, lower_indetifier, lineOfCode};
    }

    TokenType type = std::islower(rawIdentifier[0]) ? TokenType::OBJECTID
                                                    : TokenType::TYPEID;

    return Token{type, rawIdentifier, lineOfCode};
}

Token Lexer::ParsePunctuation() {
    std::set<char> uno_punctuation = {'(', ')', '.', ',', ':', ';', '{', '}',
                                      '@', '~', '*', '/', '+', '-', '='};
    if (source_code[curr_idx] == '<') {
        ++curr_idx;
        if (curr_idx == source_code.size()) {
            return {TokenType::PUNCTUATION, "<", lineOfCode};
        }
        if (source_code[curr_idx] == '-') {
            ++curr_idx;
            return {TokenType::ASSIGN, "", lineOfCode};
        }
        if (source_code[curr_idx] == '=') {
            ++curr_idx;
            return {TokenType::LE, "", lineOfCode};
        }

        return {TokenType::PUNCTUATION, "<", lineOfCode};
    }

    if (curr_idx + 1 != source_code.size() && source_code[curr_idx] == '=' && source_code[curr_idx + 1] == '>') {
        curr_idx += 2;
        return {TokenType::DARROW, "=>", lineOfCode};
    }

    if (uno_punctuation.count(source_code[curr_idx])) {
        return Token{TokenType::PUNCTUATION,
                     std::string() + source_code[curr_idx++], lineOfCode};
    }

    ++curr_idx;

    return Token{TokenType::ERROR, "Punctuation not parsed", lineOfCode};
}

void Lexer::ParseLineComment() {
    while (curr_idx != source_code.size() && source_code[curr_idx] != '\n') {
        ++curr_idx;
    }

    if (curr_idx != source_code.size() && source_code[curr_idx] == '\n') {
        ++lineOfCode;
        ++curr_idx;
    }
}

bool Lexer::ParseMultiLineComment() {
    int cnt = 1;
    ++curr_idx;
    char prev_symbol = source_code[curr_idx - 1];
    char cur_symbol = source_code[curr_idx];
    while (curr_idx + 1 != source_code.size()) {
        ++curr_idx;
        prev_symbol = cur_symbol;
        cur_symbol = source_code[curr_idx];
        if (cur_symbol == ')' && prev_symbol == '*' && source_code[curr_idx - 2] != '(') {
            if (--cnt == 0) break;
        }
        if (cur_symbol == '*' && prev_symbol == '(') {
            ++cnt;
        }
        if (cur_symbol == '\n') {
            ++lineOfCode;
        }
    }
    ++curr_idx;
    return cnt == 0;
}

Token Lexer::NextToken() {
    if (curr_idx == source_code.size()) {
        return Token{TokenType::EOFILE, "", lineOfCode};
    }
    char curr_symbol = source_code[curr_idx];
    char next_symbol = (curr_idx != source_code.size()) ? source_code[curr_idx + 1] : '#';

    Token result = {};

    if (isSpaceSymbol(curr_symbol)) {
        ++curr_idx;
        return NextToken();
    } else if (curr_symbol == '-' && next_symbol == '-') {
        ParseLineComment();
        return NextToken();
    } else if (curr_symbol == '(' && next_symbol == '*') {
        if (ParseMultiLineComment()) {
            return NextToken();
        }
        return Token{TokenType::ERROR, "EOF in comment", lineOfCode};
    } else if (isNewLineSymbol(curr_symbol)) {
        ++lineOfCode;
        ++curr_idx;
        return NextToken();
    } else if (isdigit(curr_symbol)) {
        return ParseInteger();
    } else if (isalpha(curr_symbol)) {
        return ParseIdentifier();
    } else if (curr_symbol == '"') {
        return ParseString();
    } else {
        return ParsePunctuation();
    }

    throw std::invalid_argument((std::string("Not parsed at line") +
                                 std::to_string(lineOfCode) +
                                 " gen idx:" + std::to_string(curr_idx))
                                    .c_str());

    return result;
}

void Lexer::PrintResult() {
    Token token;
    while ((token = NextToken()).tokenType != TokenType::EOFILE && token.tokenType != TokenType::ERROR) {
        std::cout << token << std::endl;
    }
    if (token.tokenType == TokenType::ERROR) {
        std::cout << token << std::endl;
    }
}
