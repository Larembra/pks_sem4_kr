#include <catch2/catch_test_macros.hpp>

#include "../src/include/contestants/contestant.h"

TEST_CASE("Scenario: contestant state transitions", "[scenario][contestant]") {
    contestant c;

    c.set_max_hp(5);
    c.set_max_mp(7);
    c.set_hp(3);
    c.set_mp(4);

    REQUIRE(c.get_hp() == 3);
    REQUIRE(c.get_mp() == 4);

    c.stun();
    REQUIRE(c.is_stunned() == true);

    c.unstun();
    REQUIRE(c.is_stunned() == false);
}

