#include <catch2/catch_test_macros.hpp>

#include <index_subset.h>

#include <stdexcept>

TEST_CASE("index_subset basic operations", "[index_subset]")
{
    ptg::index_subset is(100);

    SECTION("add and contains")
    {
        is.add(42);
        REQUIRE(is.contains(42) == true);
        REQUIRE(is.contains(0) == false);
    }

    SECTION("multiple adds")
    {
        is.add(1);
        is.add(2);
        is.add(1);  // Duplicate
        REQUIRE(is.contains(1) == true);
        REQUIRE(is.contains(2) == true);
        REQUIRE(is.contains(3) == false);
    }

    SECTION("out of range")
    {
        REQUIRE_THROWS_AS(is.add(100), std::out_of_range);
        REQUIRE_THROWS_AS(is.contains(100), std::out_of_range);
    }

    SECTION("get_count")
    {
        REQUIRE(is.get_count() == 0);
        is.add(0);
        REQUIRE(is.get_count() == 1);
        is.add(50);
        REQUIRE(is.get_count() == 2);
        is.add(0);  // Duplicate
        REQUIRE(is.get_count() == 2);
    }

    SECTION("get_indices")
    {
        is.add(10);
        is.add(5);
        is.add(20);
        REQUIRE(is.get_count() == 3);
        const auto& inds = is.get_indices();
        REQUIRE(inds == std::vector<size_t>{10, 5, 20});
    }

    SECTION("get_size")
    {
        REQUIRE(is.get_size() == 100);
    }

    SECTION("zero size throws")
    {
        REQUIRE_THROWS_AS([]{ ptg::index_subset(0); }(), std::invalid_argument);
    }
}
