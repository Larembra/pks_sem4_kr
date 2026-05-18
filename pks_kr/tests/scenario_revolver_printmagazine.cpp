#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <iostream>

#include "../src/include/weapons/revolver.h"

struct CoutCaptureScenario {
    std::streambuf* old = nullptr;
    std::ostringstream ss;
    CoutCaptureScenario() {
        old = std::cout.rdbuf(ss.rdbuf());
    }
    ~CoutCaptureScenario() {
        std::cout.rdbuf(old);
    }
    std::string str() const { return ss.str(); }
};

TEST_CASE("Scenario: printmagazine shows bullets and newline", "[scenario][revolver][print]") {
    revolver gun;
    gun.new_magazine();

    CoutCaptureScenario cap;
    gun.printmagazine();
    auto out = cap.str();

    REQUIRE(out.find("Magazine [") != std::string::npos);
    REQUIRE(out.find("] Bullets: ") != std::string::npos);
    REQUIRE(!out.empty());
    REQUIRE(out.back() == '\n');
}

