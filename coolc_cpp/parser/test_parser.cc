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
void compare_parsers(const std::string& file) {
    const std::string lexer = "../../../bin/lexer";

    const std::string original_parser = "../../../bin/parser";
    const std::string parser = "./parser";

    const std::string reference_cmd = lexer + " " + file + " | " + original_parser;
    const std::string cmd = lexer + " " + file + " | " + parser;

    std::string reference_output = exec(reference_cmd.c_str());
    std::string output = exec(cmd.c_str());

    std::istringstream ref(reference_output);
    std::istringstream my(output);
    while (!ref.eof() || !my.eof()) {
        std::string ref_line, my_line;
        getline(ref, ref_line);
        getline(my, my_line);
        ASSERT_EQ(ref_line, my_line);
    }
}

TEST(EndToEnd, StackAssignment) {
    const std::string example_stack = "../../PA1/stack.cl";
    compare_parsers(example_stack);
}

TEST(EndToEnd, Examples) {
    const std::string path = "../../../examples";
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        std::cerr << entry.path() << std::endl;
        compare_parsers(entry.path());
    }
}
