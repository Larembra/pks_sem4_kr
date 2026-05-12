#include <catch2/catch_test_macros.hpp>
#include <string>

#include "../src/include/contestants/player.h"
#include "../src/include/contestants/contestant.h"
#include "../src/include/weapons/revolver.h"

TEST_CASE("player::format_status replaces MY_HP with player hp", "[player][format_status]") {
    player p;
    p.set_hp(2);
    p.set_mp(3);

    contestant enemy;
    enemy.set_hp(1);
    enemy.set_mp(9);

    revolver gun;

    auto s = p.format_status(enemy, gun);

    REQUIRE(s.find("{MY_HP}") == std::string::npos);
    REQUIRE(s.find("Your HP: 2") != std::string::npos);
}

TEST_CASE("player::format_status replaces MY_MP with player mp", "[player][format_status]") {
    player p;
    p.set_hp(3);
    p.set_mp(7);

    contestant enemy;
    enemy.set_hp(3);
    enemy.set_mp(1);

    revolver gun;

    auto s = p.format_status(enemy, gun);

    REQUIRE(s.find("{MY_MP}") == std::string::npos);
    REQUIRE(s.find("MP: 7") != std::string::npos);
}

TEST_CASE("player::format_status replaces ENEMY_HP with enemy hp", "[player][format_status]") {
    player p;
    p.set_hp(3);
    p.set_mp(3);

    contestant enemy;
    enemy.set_hp(2);
    enemy.set_mp(3);

    revolver gun;

    auto s = p.format_status(enemy, gun);

    REQUIRE(s.find("{ENEMY_HP}") == std::string::npos);
    REQUIRE(s.find("Opponent HP: 2") != std::string::npos);
}

TEST_CASE("player::format_status replaces ENEMY_MP with enemy mp", "[player][format_status]") {
    player p;
    p.set_hp(3);
    p.set_mp(3);

    contestant enemy;
    enemy.set_hp(3);
    enemy.set_mp(10);

    revolver gun;

    auto s = p.format_status(enemy, gun);

    REQUIRE(s.find("{ENEMY_MP}") == std::string::npos);
    REQUIRE(s.find("MP: 10") != std::string::npos);
}

TEST_CASE("player::format_status leaves no placeholders", "[player][format_status]") {
    player p;
    p.set_hp(1);
    p.set_mp(2);

    contestant enemy;
    enemy.set_hp(3);
    enemy.set_mp(4);

    revolver gun;

    auto s = p.format_status(enemy, gun);

    REQUIRE(s.find("{MY_HP}") == std::string::npos);
    REQUIRE(s.find("{MY_MP}") == std::string::npos);
    REQUIRE(s.find("{ENEMY_HP}") == std::string::npos);
    REQUIRE(s.find("{ENEMY_MP}") == std::string::npos);
}

TEST_CASE("player::format_status contains status header and footer", "[player][format_status]") {
    player p;
    p.set_hp(3);
    p.set_mp(3);

    contestant enemy;
    enemy.set_hp(3);
    enemy.set_mp(3);

    revolver gun;

    auto s = p.format_status(enemy, gun);

    REQUIRE(s.find("STATUS") != std::string::npos);
    REQUIRE(s.find("❗️❗️❗️STATUS❗️❗️❗️") != std::string::npos);
}

TEST_CASE("player::format_status works with multi-digit mp values", "[player][format_status]") {
    player p;
    p.set_hp(3);
    p.set_mp(10);

    contestant enemy;
    enemy.set_hp(3);
    enemy.set_mp(10);

    revolver gun;

    auto s = p.format_status(enemy, gun);

    REQUIRE(s.find("MP: 10") != std::string::npos);
}