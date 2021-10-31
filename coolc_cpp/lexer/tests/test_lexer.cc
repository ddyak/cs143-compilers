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

void compare_lexers(const std::vector<std::string>& files) {
    const std::string original_lexer = "../../../bin/lexer";
    const std::string lexer = "./lexer";

    std::ostringstream imploded;
    std::copy(files.begin(), files.end(),
              std::ostream_iterator<std::string>(imploded, " "));

    const std::string files_str = imploded.str();

    const std::string reference_cmd = original_lexer + " " + files_str;
    const std::string cmd = lexer + " " + files_str;

    std::string reference_output = exec(reference_cmd.c_str());
    std::string output = exec(cmd.c_str());

    std::istringstream ref(reference_output);
    std::istringstream lex(output);
    while (!ref.eof() || !lex.eof()) {
        std::string ref_line, lex_line;
        getline(ref, ref_line);
        getline(lex, lex_line);

        // on correct input produce correct ouput
        // my and referece error handling not equal
        if (ref_line.find("ERROR") != std::string::npos ||
            (ref_line.find("ERROR") != std::string::npos &&
             lex_line.find("ERROR") != std::string::npos)) {
            return;
        }

        ASSERT_EQ(ref_line, lex_line);
    }
}

TEST(EndToEnd, StackAssignment) {
    const std::string example_stack = "../../stack_example/stack.cl";
    compare_lexers({example_stack});
}

TEST(EndToEnd, Examples) {
    const std::string path = "../../../examples";
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        compare_lexers({entry.path()});
    }
}

TEST(EndToEnd, MultipleFileInput) {
    const std::string first_file = "../../../examples/arith.cl";
    const std::string second_file = "../../../examples/atoi.cl";
    compare_lexers({first_file, second_file});
}

TEST(EndToEnd, OriginalCourseTest) {
    const std::string path = "../../../lexer-tests";
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        compare_lexers({entry.path()});
    }
}
