#include <catch2/catch_test_macros.hpp>
#include <string>

#include "../src/include/contestants/player.h"
#include "../src/include/contestants/contestant.h"
#include "../src/include/weapons/revolver.h"

TEST_CASE("Scenario: player status after weapon init", "[scenario][status]") {
    player p;
    p.set_hp(3);
    p.set_mp(5);

    contestant enemy;
    enemy.set_hp(2);
    enemy.set_mp(4);

    revolver gun;
    gun.new_magazine();

    auto status = p.format_status(enemy, gun);

    REQUIRE(status.find("{MY_HP}") == std::string::npos);
    REQUIRE(status.find("{MY_MP}") == std::string::npos);
    REQUIRE(status.find("{ENEMY_HP}") == std::string::npos);
    REQUIRE(status.find("{ENEMY_MP}") == std::string::npos);
    REQUIRE(status.find("STATUS") != std::string::npos);
}

