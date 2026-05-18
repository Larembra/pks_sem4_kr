#include <catch2/catch_test_macros.hpp>
#include <string>

#include "../src/include/contestants/player.h"
#include "../src/include/contestants/contestant.h"
#include "../src/include/weapons/revolver.h"

TEST_CASE("Scenario: player status has no placeholders with multi-digit values", "[scenario][player][status]") {
    player p;
    p.set_hp(12);
    p.set_mp(10);

    contestant enemy;
    enemy.set_hp(9);
    enemy.set_mp(11);

    revolver gun;
    gun.new_magazine();

    auto status = p.format_status(enemy, gun);

    REQUIRE(status.find("{MY_HP}") == std::string::npos);
    REQUIRE(status.find("{MY_MP}") == std::string::npos);
    REQUIRE(status.find("{ENEMY_HP}") == std::string::npos);
    REQUIRE(status.find("{ENEMY_MP}") == std::string::npos);

    REQUIRE(status.find("12") != std::string::npos);
    REQUIRE(status.find("10") != std::string::npos);
    REQUIRE(status.find("9") != std::string::npos);
    REQUIRE(status.find("11") != std::string::npos);
}

