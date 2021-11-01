#include <iostream>

struct A {
    int a;
};

int main() {
    std::cout << std::is_same_v<decltype(A{1}), int> << '\n'; // true
    return 0;
}