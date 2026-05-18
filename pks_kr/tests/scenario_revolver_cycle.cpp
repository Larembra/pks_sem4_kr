#include <catch2/catch_test_macros.hpp>

#include "../src/include/weapons/revolver.h"

TEST_CASE("Scenario: revolver state cycle keeps invariants", "[scenario][revolver]") {
    revolver gun;
    gun.new_magazine();

    int chambers = gun.num_chambers();
    int bullets = gun.num_bullets();
    bool cell0 = gun.cell();

    gun.swap_bullet();
    bool cell1 = gun.cell();

    REQUIRE(chambers == 6);
    REQUIRE(bullets >= 1);
    REQUIRE(bullets <= 5);
    REQUIRE(cell1 != cell0);

    gun.swap_bullet();
    REQUIRE(gun.cell() == cell0);
    REQUIRE(gun.num_chambers() == chambers);
    REQUIRE(gun.num_bullets() == bullets);
}

