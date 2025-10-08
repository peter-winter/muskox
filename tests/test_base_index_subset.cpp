#include <stdexcept>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <base_index_subset.h>

using Catch::Matchers::Message;

TEST_CASE("base_index_subset basic operations", "[base_index_subset]")
{
    SECTION("default init false")
    {
        ptg::base_index_subset is(100);
        REQUIRE(is.contains(42) == false);
    }

    SECTION("init all with value true")
    {
        ptg::base_index_subset is(100, true);
        REQUIRE(is.contains(42) == true);
        REQUIRE(is.add(42) == false);  // Already set
    }

    SECTION("add and contains")
    {
        ptg::base_index_subset is(100);
        REQUIRE(is.add(42) == true);  // New
        REQUIRE(is.contains(42) == true);
        REQUIRE(is.contains(0) == false);
        REQUIRE(is.add(42) == false);  // Duplicate
    }

    SECTION("multiple adds")
    {
        ptg::base_index_subset is(100);
        REQUIRE(is.add(1) == true);
        REQUIRE(is.add(2) == true);
        REQUIRE(is.add(1) == false);  // Duplicate
        REQUIRE(is.contains(1) == true);
        REQUIRE(is.contains(2) == true);
        REQUIRE(is.contains(3) == false);
    }

    SECTION("out of range")
    {
        ptg::base_index_subset is(100);
        REQUIRE_THROWS_MATCHES(
            is.add(100),
            std::out_of_range,
            Message("Index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            is.contains(100),
            std::out_of_range,
            Message("Index out of range")
        );
    }


    SECTION("get_size")
    {
        ptg::base_index_subset is(100);
        REQUIRE(is.get_size() == 100);
    }

    SECTION("zero size throws")
    {
        REQUIRE_THROWS_MATCHES(
            []{ ptg::base_index_subset(0); }(),
            std::invalid_argument,
            Message("Size must be greater than 0")
        );
        REQUIRE_THROWS_MATCHES(
            []{ ptg::base_index_subset(0, true); }(),
            std::invalid_argument,
            Message("Size must be greater than 0")
        );
    }
}
