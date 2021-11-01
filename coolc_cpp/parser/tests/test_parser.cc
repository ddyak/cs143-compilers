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

void compare_parsers(const std::vector<std::string>& files) {
    const std::string lexer = "../../resource/bin/lexer";
    const std::string original_parser = "../../resource/bin/parser";
    const std::string parser = "./parser";

    std::ostringstream imploded;
    std::copy(files.begin(), files.end(),
              std::ostream_iterator<std::string>(imploded, " "));

    const std::string files_str = imploded.str();

    const std::string reference_cmd = lexer + " " + files_str + " | " + original_parser;
    const std::string cmd = lexer + " " + files_str + " | " + parser;

    std::string reference_output = exec(reference_cmd.c_str());
    std::string output = exec(cmd.c_str());

    // on correct input produce correct ouput
    // my and referece error handling not equal
    if (reference_output.empty()) {
        ASSERT_TRUE(output.empty());
        return;
    }

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
    const std::string path = "../../stack_example/stack.cl";
    compare_parsers({path, path});
}

TEST(EndToEnd, Multiple) {
    const std::string example_stack = "../../stack_example/stack.cl";
    compare_parsers({example_stack});
}

TEST(EndToEnd, EndToEnd) {
    const std::string path = "../../parser/tests/end-to-end";
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        std::cerr << entry.path() << std::endl;
        compare_parsers({entry.path()});
    }
}
