#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>
#include <cstdint>

#include "../src/include/contestants/nn.h"
#include "../src/include/contestants/contestant.h"
#include "../src/include/weapons/revolver.h"

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output);

static int count_substr(const std::string& s, const std::string& sub) {
    if (sub.empty()) return 0;
    int c = 0;
    size_t pos = 0;
    while ((pos = s.find(sub, pos)) != std::string::npos) {
        ++c;
        pos += sub.size();
    }
    return c;
}

TEST_CASE("WriteCallback appends bytes to output", "[nn][callback]") {
    std::string out;
    const char data[] = "abc";
    auto written = WriteCallback((void*)data, 1, 3, &out);
    REQUIRE(written == 3);
    REQUIRE(out == "abc");
}

TEST_CASE("WriteCallback accumulates multiple calls", "[nn][callback]") {
    std::string out = "x";
    const char a[] = "12";
    const char b[] = "345";
    REQUIRE(WriteCallback((void*)a, 1, 2, &out) == 2);
    REQUIRE(WriteCallback((void*)b, 1, 3, &out) == 3);
    REQUIRE(out == "x12345");
}

TEST_CASE("WriteCallback returns 0 when size is 0", "[nn][callback]") {
    std::string out = "x";
    const char data[] = "abc";
    auto written = WriteCallback((void*)data, 0, 3, &out);
    REQUIRE(written == 0);
    REQUIRE(out == "x");
}

TEST_CASE("nn::format_prompt replaces MY_HP", "[nn][format_prompt]") {
    nn bot;

    contestant enemy;
    enemy.set_hp(2);
    enemy.set_mp(4);

    revolver gun;

    bot.set_hp(1);
    bot.set_mp(7);

    auto s = bot.format_prompt(enemy, gun);

    REQUIRE(s.find("{MY_HP}") == std::string::npos);
    REQUIRE(s.find("Your HP: 1") != std::string::npos);
}

TEST_CASE("nn::format_prompt replaces MY_MP", "[nn][format_prompt]") {
    nn bot;

    contestant enemy;
    enemy.set_hp(3);
    enemy.set_mp(1);

    revolver gun;

    bot.set_hp(3);
    bot.set_mp(9);

    auto s = bot.format_prompt(enemy, gun);

    REQUIRE(s.find("{MY_MP}") == std::string::npos);
    REQUIRE(s.find("MP: 9") != std::string::npos);
}

TEST_CASE("nn::format_prompt replaces ENEMY_HP and ENEMY_MP", "[nn][format_prompt]") {
    nn bot;

    contestant enemy;
    enemy.set_hp(2);
    enemy.set_mp(6);

    revolver gun;

    bot.set_hp(3);
    bot.set_mp(1);

    auto s = bot.format_prompt(enemy, gun);

    REQUIRE(s.find("{ENEMY_HP}") == std::string::npos);
    REQUIRE(s.find("{ENEMY_MP}") == std::string::npos);

    REQUIRE(s.find("Opponent HP: 2") != std::string::npos);
    REQUIRE(s.find("Opponent HP: 2 | MP: 6") != std::string::npos);
}

TEST_CASE("nn::format_prompt has no remaining placeholders", "[nn][format_prompt]") {
    nn bot;

    contestant enemy;
    enemy.set_hp(1);
    enemy.set_mp(2);

    revolver gun;

    bot.set_hp(3);
    bot.set_mp(10);

    auto s = bot.format_prompt(enemy, gun);

    REQUIRE(s.find("{MY_HP}") == std::string::npos);
    REQUIRE(s.find("{MY_MP}") == std::string::npos);
    REQUIRE(s.find("{ENEMY_HP}") == std::string::npos);
    REQUIRE(s.find("{ENEMY_MP}") == std::string::npos);
    REQUIRE(s.find("{CHAMBERS}") == std::string::npos);
    REQUIRE(s.find("{BULLETS}") == std::string::npos);
}

TEST_CASE("nn::format_prompt contains exactly one Current status block", "[nn][format_prompt]") {
    nn bot;

    contestant enemy;
    enemy.set_hp(3);
    enemy.set_mp(3);

    revolver gun;

    bot.set_hp(3);
    bot.set_mp(3);

    auto s = bot.format_prompt(enemy, gun);

    REQUIRE(count_substr(s, "Current status:") == 1);
}

TEST_CASE("nn::format_status maps MY_* to enemy values", "[nn][format_status]") {
    nn bot;

    contestant enemy;
    enemy.set_hp(2);
    enemy.set_mp(8);

    revolver gun;

    bot.set_hp(3);
    bot.set_mp(1);

    auto s = bot.format_status(enemy, gun);

    REQUIRE(s.find("{MY_HP}") == std::string::npos);
    REQUIRE(s.find("{MY_MP}") == std::string::npos);

    REQUIRE(s.find("Your HP: 2") != std::string::npos);
    REQUIRE(s.find("MP: 8") != std::string::npos);
}

TEST_CASE("nn::format_status maps ENEMY_* to bot values", "[nn][format_status]") {
    nn bot;

    contestant enemy;
    enemy.set_hp(2);
    enemy.set_mp(8);

    revolver gun;

    bot.set_hp(1);
    bot.set_mp(5);

    auto s = bot.format_status(enemy, gun);

    REQUIRE(s.find("{ENEMY_HP}") == std::string::npos);
    REQUIRE(s.find("{ENEMY_MP}") == std::string::npos);

    REQUIRE(s.find("Opponent HP: 1") != std::string::npos);
    REQUIRE(s.find("Opponent HP: 1 |") != std::string::npos);
    REQUIRE(s.find("MP: 5") != std::string::npos);
}

TEST_CASE("nn::format_status has no remaining placeholders", "[nn][format_status]") {
    nn bot;

    contestant enemy;
    enemy.set_hp(3);
    enemy.set_mp(3);

    revolver gun;

    bot.set_hp(3);
    bot.set_mp(3);

    auto s = bot.format_status(enemy, gun);

    REQUIRE(s.find("{MY_HP}") == std::string::npos);
    REQUIRE(s.find("{MY_MP}") == std::string::npos);
    REQUIRE(s.find("{ENEMY_HP}") == std::string::npos);
    REQUIRE(s.find("{ENEMY_MP}") == std::string::npos);
}