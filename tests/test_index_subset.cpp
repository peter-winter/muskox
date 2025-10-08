#include <catch2/catch_test_macros.hpp>

#include <index_subset.h>

#include <stdexcept>

TEST_CASE("index_subset basic operations", "[index_subset]")
{
    SECTION("default init")
    {
        ptg::index_subset is(100);
        REQUIRE(is.get_count() == 0);
        REQUIRE(is.contains(42) == false);
        REQUIRE(is.get_indices().empty() == true);
    }

    SECTION("add and contains")
    {
        ptg::index_subset is(100);
        REQUIRE(is.add(42) == true);  // New
        REQUIRE(is.contains(42) == true);
        REQUIRE(is.contains(0) == false);
        REQUIRE(is.add(42) == false);  // Duplicate
    }

    SECTION("multiple adds")
    {
        ptg::index_subset is(100);
        REQUIRE(is.add(1) == true);
        REQUIRE(is.add(2) == true);
        REQUIRE(is.add(1) == false);  // Duplicate
        REQUIRE(is.contains(1) == true);
        REQUIRE(is.contains(2) == true);
        REQUIRE(is.contains(3) == false);
    }

    SECTION("out of range")
    {
        ptg::index_subset is(100);
        REQUIRE_THROWS_AS(is.add(100), std::out_of_range);
        REQUIRE_THROWS_AS(is.contains(100), std::out_of_range);
    }

    SECTION("get_count")
    {
        ptg::index_subset is(100);
        REQUIRE(is.get_count() == 0);
        REQUIRE(is.add(0) == true);
        REQUIRE(is.get_count() == 1);
        REQUIRE(is.add(50) == true);
        REQUIRE(is.get_count() == 2);
        REQUIRE(is.add(0) == false);  // Duplicate
        REQUIRE(is.get_count() == 2);
    }

    SECTION("get_indices")
    {
        ptg::index_subset is(100);
        REQUIRE(is.add(10) == true);
        REQUIRE(is.add(5) == true);
        REQUIRE(is.add(20) == true);
        REQUIRE(is.get_count() == 3);
        const auto& inds = is.get_indices();
        REQUIRE(inds == std::vector<size_t>{10, 5, 20});
        REQUIRE(is.add(10) == false);  // Duplicate, no append
        REQUIRE(is.get_count() == 3);
        REQUIRE(inds == std::vector<size_t>{10, 5, 20});
    }

    SECTION("get_size")
    {
        ptg::index_subset is(100);
        REQUIRE(is.get_size() == 100);
    }

    SECTION("zero size throws")
    {
        REQUIRE_THROWS_AS([]{ ptg::index_subset(0); }(), std::invalid_argument);
    }
}
