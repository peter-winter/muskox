#include <catch2/catch_test_macros.hpp>

#include <associativity.h>

TEST_CASE("associativity to_string", "[associativity]")
{
    SECTION("left")
    {
        ptg::associativity a(ptg::associativity::type::left);
        REQUIRE(a.to_string() == "left");
    }

    SECTION("right")
    {
        ptg::associativity a(ptg::associativity::type::right);
        REQUIRE(a.to_string() == "right");
    }

    SECTION("static left")
    {
        auto a = ptg::associativity::left();
        REQUIRE(a.to_string() == "left");
    }

    SECTION("static right")
    {
        auto a = ptg::associativity::right();
        REQUIRE(a.to_string() == "right");
    }
}
