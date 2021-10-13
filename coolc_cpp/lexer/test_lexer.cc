#include <gtest/gtest.h>

#include <array>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}
void compare_lexers(const std::string& file) {
    const std::string original_lexer = "../../../bin/lexer";
    const std::string lexer = "./lexer";

    const std::string reference_cmd = original_lexer + " " + file;
    const std::string cmd = lexer + " " + file;

    std::string reference_output = exec(reference_cmd.c_str());
    std::string output = exec(cmd.c_str());

    std::istringstream ref(reference_output);
    std::istringstream lex(output);
    while (!ref.eof() || !lex.eof()) {
        std::string ref_line, lex_line;
        getline(ref, ref_line);
        getline(lex, lex_line);
        ASSERT_EQ(ref_line, lex_line);
    }
}

TEST(EndToEnd, StackAssignment) {
    const std::string example_stack = "../../PA1/stack.cl";
    compare_lexers(example_stack);
}

TEST(EndToEnd, Examples) {
    const std::string path = "../../../examples";
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        std::cerr << entry.path() << std::endl;
        compare_lexers(entry.path());
    }
}
