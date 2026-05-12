#include <catch2/catch_test_macros.hpp>
#include "../src/include/weapons/revolver.h"
#include <sstream>
#include <iostream>

struct CoutCapture {
    std::streambuf* old = nullptr;
    std::ostringstream ss;
    CoutCapture() {
        old = std::cout.rdbuf(ss.rdbuf());
    }
    ~CoutCapture() {
        std::cout.rdbuf(old);
    }
    std::string str() const { return ss.str(); }
};

TEST_CASE("new_magazine: bullets count is between 1 and 5", "[revolver][new_magazine]") {
    revolver g;
    g.new_magazine();
    REQUIRE(g.num_bullets() >= 1);
    REQUIRE(g.num_bullets() <= 5);
}

TEST_CASE("new_magazine: chambers count is 6 initially", "[revolver][new_magazine]") {
    revolver g;
    g.new_magazine();
    REQUIRE(g.num_chambers() == 6);
}

TEST_CASE("new_magazine: cell() returns a valid boolean and does not throw", "[revolver][new_magazine]") {
    revolver g;
    g.new_magazine();
    bool v = g.cell();
    REQUIRE((v == true || v == false));
}

TEST_CASE("new_magazine: swap_bullet still works right after initialization", "[revolver][new_magazine]") {
    revolver g;
    g.new_magazine();
    bool before = g.cell();
    g.swap_bullet();
    REQUIRE(g.cell() != before);
}

TEST_CASE("new_magazine: multiple initializations keep invariants", "[revolver][new_magazine]") {
    revolver g;
    for (int i = 0; i < 10; ++i) {
        g.new_magazine();
        REQUIRE(g.num_bullets() >= 1);
        REQUIRE(g.num_bullets() <= 5);
        REQUIRE(g.num_chambers() == 6);
    }
}

TEST_CASE("cell: toggles to opposite after one swap_bullet", "[revolver][cell]") {
    revolver g;
    g.new_magazine();
    bool before = g.cell();
    g.swap_bullet();
    REQUIRE(g.cell() == !before);
}

TEST_CASE("cell: returns to original after two swap_bullet calls", "[revolver][cell]") {
    revolver g;
    g.new_magazine();
    bool before = g.cell();
    g.swap_bullet();
    g.swap_bullet();
    REQUIRE(g.cell() == before);
}

TEST_CASE("cell: four toggles returns to original state", "[revolver][cell]") {
    revolver g;
    g.new_magazine();
    bool before = g.cell();
    for (int i = 0; i < 4; ++i) g.swap_bullet();
    REQUIRE(g.cell() == before);
}

TEST_CASE("cell: odd number of toggles flips state", "[revolver][cell]") {
    revolver g;
    g.new_magazine();
    bool before = g.cell();
    for (int i = 0; i < 7; ++i) g.swap_bullet();
    REQUIRE(g.cell() == !before);
}

TEST_CASE("cell: even number of toggles keeps state", "[revolver][cell]") {
    revolver g;
    g.new_magazine();
    bool before = g.cell();
    for (int i = 0; i < 8; ++i) g.swap_bullet();
    REQUIRE(g.cell() == before);
}

TEST_CASE("num_bullets: after new_magazine value is between 1 and 5", "[revolver][num_bullets]") {
    revolver g;
    g.new_magazine();
    int b = g.num_bullets();
    REQUIRE(b >= 1);
    REQUIRE(b <= 5);
}

TEST_CASE("num_bullets: swap_bullet does not change bullets count", "[revolver][num_bullets]") {
    revolver g;
    g.new_magazine();
    int b0 = g.num_bullets();
    for (int i = 0; i < 20; ++i) g.swap_bullet();
    REQUIRE(g.num_bullets() == b0);
}

TEST_CASE("num_bullets: multiple calls are stable without shoot/new_magazine", "[revolver][num_bullets]") {
    revolver g;
    g.new_magazine();
    int b0 = g.num_bullets();
    REQUIRE(g.num_bullets() == b0);
    REQUIRE(g.num_bullets() == b0);
    REQUIRE(g.num_bullets() == b0);
}

TEST_CASE("num_bullets: reinitialization changes or keeps within range", "[revolver][num_bullets]") {
    revolver g;
    g.new_magazine();
    int b1 = g.num_bullets();
    g.new_magazine();
    int b2 = g.num_bullets();
    REQUIRE(b1 >= 1);
    REQUIRE(b1 <= 5);
    REQUIRE(b2 >= 1);
    REQUIRE(b2 <= 5);
}

TEST_CASE("num_bullets: reinitialize many times always within range", "[revolver][num_bullets]") {
    revolver g;
    for (int i = 0; i < 30; ++i) {
        g.new_magazine();
        REQUIRE(g.num_bullets() >= 1);
        REQUIRE(g.num_bullets() <= 5);
    }
}

TEST_CASE("num_chambers: after new_magazine equals 6", "[revolver][num_chambers]") {
    revolver g;
    g.new_magazine();
    REQUIRE(g.num_chambers() == 6);
}

TEST_CASE("num_chambers: stable across swap_bullet", "[revolver][num_chambers]") {
    revolver g;
    g.new_magazine();
    int c0 = g.num_chambers();
    for (int i = 0; i < 10; ++i) g.swap_bullet();
    REQUIRE(g.num_chambers() == c0);
}

TEST_CASE("num_chambers: multiple calls return same value", "[revolver][num_chambers]") {
    revolver g;
    g.new_magazine();
    int c0 = g.num_chambers();
    REQUIRE(g.num_chambers() == c0);
    REQUIRE(g.num_chambers() == c0);
    REQUIRE(g.num_chambers() == c0);
}

TEST_CASE("num_chambers: many initializations always set to 6", "[revolver][num_chambers]") {
    revolver g;
    for (int i = 0; i < 15; ++i) {
        g.new_magazine();
        REQUIRE(g.num_chambers() == 6);
    }
}

TEST_CASE("num_chambers: never negative after init", "[revolver][num_chambers]") {
    revolver g;
    g.new_magazine();
    REQUIRE(g.num_chambers() >= 0);
}

TEST_CASE("swap_bullet: toggles cell state", "[revolver][swap_bullet]") {
    revolver g;
    g.new_magazine();
    bool before = g.cell();
    g.swap_bullet();
    REQUIRE(g.cell() != before);
}

TEST_CASE("swap_bullet: two toggles restore state", "[revolver][swap_bullet]") {
    revolver g;
    g.new_magazine();
    bool before = g.cell();
    g.swap_bullet();
    g.swap_bullet();
    REQUIRE(g.cell() == before);
}

TEST_CASE("swap_bullet: odd toggles flip state", "[revolver][swap_bullet]") {
    revolver g;
    g.new_magazine();
    bool before = g.cell();
    for (int i = 0; i < 9; ++i) g.swap_bullet();
    REQUIRE(g.cell() == !before);
}

TEST_CASE("swap_bullet: even toggles keep state", "[revolver][swap_bullet]") {
    revolver g;
    g.new_magazine();
    bool before = g.cell();
    for (int i = 0; i < 10; ++i) g.swap_bullet();
    REQUIRE(g.cell() == before);
}

TEST_CASE("swap_bullet: does not affect bullet count variable", "[revolver][swap_bullet]") {
    revolver g;
    g.new_magazine();
    int b0 = g.num_bullets();
    g.swap_bullet();
    REQUIRE(g.num_bullets() == b0);
}

TEST_CASE("printmagazine: prints header and bullets count", "[revolver][printmagazine]") {
    revolver g;
    g.new_magazine();

    CoutCapture cap;
    g.printmagazine();
    auto out = cap.str();

    REQUIRE(out.find("Magazine [") != std::string::npos);
    REQUIRE(out.find("] Bullets: ") != std::string::npos);
}

TEST_CASE("printmagazine: output contains exactly one '(' and one ')'", "[revolver][printmagazine]") {
    revolver g;
    g.new_magazine();

    CoutCapture cap;
    g.printmagazine();
    auto out = cap.str();

    int open = 0, close = 0;
    for (char ch : out) {
        if (ch == '(') ++open;
        if (ch == ')') ++close;
    }

    REQUIRE(open == 1);
    REQUIRE(close == 1);
}

TEST_CASE("printmagazine: output ends with newline", "[revolver][printmagazine]") {
    revolver g;
    g.new_magazine();

    CoutCapture cap;
    g.printmagazine();
    auto out = cap.str();

    REQUIRE(!out.empty());
    REQUIRE(out.back() == '\n');
}

TEST_CASE("printmagazine: can be called multiple times", "[revolver][printmagazine]") {
    revolver g;
    g.new_magazine();

    CoutCapture cap;
    g.printmagazine();
    g.printmagazine();
    auto out = cap.str();

    REQUIRE(out.find("Magazine [") != std::string::npos);
}

TEST_CASE("printmagazine: output mentions bullets number digits", "[revolver][printmagazine]") {
    revolver g;
    g.new_magazine();
    int b = g.num_bullets();

    CoutCapture cap;
    g.printmagazine();
    auto out = cap.str();

    REQUIRE(out.find("Bullets: " + std::to_string(b)) != std::string::npos);
}