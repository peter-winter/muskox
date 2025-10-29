#include <catch2/catch_test_macros.hpp>

#include <associativity.h>

using namespace muskox;

TEST_CASE("associativity to_string", "[associativity]")
{
    SECTION("left")
    {
        associativity a(associativity::type::left);
        REQUIRE(a.to_string() == "left");
    }

    SECTION("right")
    {
        associativity a(associativity::type::right);
        REQUIRE(a.to_string() == "right");
    }

    SECTION("static left")
    {
        auto a = associativity::left();
        REQUIRE(a.to_string() == "left");
    }

    SECTION("static right")
    {
        auto a = associativity::right();
        REQUIRE(a.to_string() == "right");
    }
}
