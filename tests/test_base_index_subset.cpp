#include <stdexcept>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <base_index_subset.h>

using Catch::Matchers::Message;

TEST_CASE("base_index_subset basic operations", "[base_index_subset]")
{
    using BIS1 = ptg::base_index_subset<1>;
    using BIS2 = ptg::base_index_subset<2>;
    using BIS3 = ptg::base_index_subset<3>;

    SECTION("default init false 1D")
    {
        BIS1 is({100});
        REQUIRE(is.contains(42) == false);
    }

    SECTION("init all with value true 1D")
    {
        BIS1 is({100}, true);
        REQUIRE(is.contains(42) == true);
        REQUIRE(is.add(42) == false);  // Already set
    }

    SECTION("add and contains 1D")
    {
        BIS1 is({100});
        REQUIRE(is.add(42) == true);  // New
        REQUIRE(is.contains(42) == true);
        REQUIRE(is.contains(0) == false);
        REQUIRE(is.add(42) == false);  // Duplicate
    }

    SECTION("multiple adds 1D")
    {
        BIS1 is({100});
        REQUIRE(is.add(1) == true);
        REQUIRE(is.add(2) == true);
        REQUIRE(is.add(1) == false);  // Duplicate
        REQUIRE(is.contains(1) == true);
        REQUIRE(is.contains(2) == true);
        REQUIRE(is.contains(3) == false);
    }

    SECTION("out of range 1D")
    {
        BIS1 is({100});
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

    SECTION("get_size 1D")
    {
        BIS1 is({100});
        REQUIRE(is.get_size() == 100);
    }

    SECTION("zero size throws 1D")
    {
        REQUIRE_THROWS_MATCHES(
            []{ BIS1({0}); }(),
            std::invalid_argument,
            Message("Size must be greater than 0")
        );
        REQUIRE_THROWS_MATCHES(
            []{ BIS1({0}, true); }(),
            std::invalid_argument,
            Message("Size must be greater than 0")
        );
    }

    SECTION("remove unset index 1D")
    {
        BIS1 is({100});
        REQUIRE(is.contains(42) == false);
        REQUIRE(is.remove(42) == false);  // Not removed, was false
        REQUIRE(is.contains(42) == false);
    }

    SECTION("remove set index 1D")
    {
        BIS1 is({100});
        REQUIRE(is.add(42) == true);
        REQUIRE(is.contains(42) == true);
        REQUIRE(is.remove(42) == true);  // Removed
        REQUIRE(is.contains(42) == false);
        REQUIRE(is.remove(42) == false);  // Already removed
    }

    SECTION("remove from init true 1D")
    {
        BIS1 is({100}, true);
        REQUIRE(is.contains(42) == true);
        REQUIRE(is.remove(42) == true);  // Removed
        REQUIRE(is.contains(42) == false);
        REQUIRE(is.remove(42) == false);  // Already removed
    }

    SECTION("multiple removes 1D")
    {
        BIS1 is({100});
        REQUIRE(is.add(1) == true);
        REQUIRE(is.add(2) == true);
        REQUIRE(is.contains(1) == true);
        REQUIRE(is.contains(2) == true);

        REQUIRE(is.remove(1) == true);
        REQUIRE(is.contains(1) == false);
        REQUIRE(is.contains(2) == true);

        REQUIRE(is.remove(2) == true);
        REQUIRE(is.contains(1) == false);
        REQUIRE(is.contains(2) == false);

        REQUIRE(is.remove(1) == false);  // Already removed
        REQUIRE(is.remove(3) == false);  // Never set
    }

    SECTION("remove out of range 1D")
    {
        BIS1 is({100});
        REQUIRE_THROWS_MATCHES(
            is.remove(100),
            std::out_of_range,
            Message("Index out of range")
        );
    }

    SECTION("zero dimension throws")
    {
        REQUIRE_THROWS_MATCHES(
            []{ ptg::base_index_subset<0> is({}); }(),
            std::invalid_argument,
            Message("At least one dimension required")
        );
    }

    SECTION("basic 2D")
    {
        BIS2 is({5, 10});
        REQUIRE(is.get_size() == 50);
        REQUIRE(is.contains(0, 0) == false);
        REQUIRE(is.add(2, 3) == true);
        REQUIRE(is.contains(2, 3) == true);
        REQUIRE(is.contains(2, 4) == false);
        REQUIRE(is.add(2, 3) == false);  // Duplicate
    }

    SECTION("out of range 2D")
    {
        BIS2 is({5, 10});
        REQUIRE_THROWS_MATCHES(
            is.add(5, 0),
            std::out_of_range,
            Message("Index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            is.add(0, 10),
            std::out_of_range,
            Message("Index out of range")
        );
    }

    SECTION("remove 2D")
    {
        BIS2 is({5, 10});
        REQUIRE(is.add(1, 2) == true);
        REQUIRE(is.contains(1, 2) == true);
        REQUIRE(is.remove(1, 2) == true);
        REQUIRE(is.contains(1, 2) == false);
        REQUIRE(is.remove(1, 2) == false);
    }

    SECTION("init true 2D")
    {
        BIS2 is({5, 10}, true);
        REQUIRE(is.contains(4, 9) == true);
        REQUIRE(is.remove(4, 9) == true);
        REQUIRE(is.contains(4, 9) == false);
    }

    SECTION("basic 3D")
    {
        BIS3 is({2, 3, 4});
        REQUIRE(is.get_size() == 24);
        REQUIRE(is.add(1, 2, 3) == true);
        REQUIRE(is.contains(1, 2, 3) == true);
        REQUIRE(is.contains(0, 0, 0) == false);
    }

    SECTION("out of range 3D")
    {
        BIS3 is({2, 3, 4});
        REQUIRE_THROWS_MATCHES(
            is.add(2, 0, 0),
            std::out_of_range,
            Message("Index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            is.add(0, 3, 0),
            std::out_of_range,
            Message("Index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            is.add(0, 0, 4),
            std::out_of_range,
            Message("Index out of range")
        );
    }

    SECTION("strides and flat indexing 3D")
    {
        BIS3 is({2, 3, 4});
        REQUIRE(is.add(0, 0, 0) == true);
        REQUIRE(is.add(1, 2, 3) == true);
        REQUIRE(is.contains(0, 0, 0) == true);
        REQUIRE(is.contains(1, 2, 3) == true);
        REQUIRE(is.contains(0, 0, 1) == false);
    }

    SECTION("large dimensions")
    {
        ptg::base_index_subset<4> is({2, 2, 2, 2});
        REQUIRE(is.get_size() == 16);
        REQUIRE(is.add(1, 1, 1, 1) == true);
        REQUIRE(is.contains(1, 1, 1, 1) == true);
    }

    SECTION("wrong number of indices")
    {
        BIS2 is({5, 10});
        // These should not compile, but for documentation:
        // is.add(1);  // Compile error: wrong number
        // is.contains(1, 2, 3);  // Compile error
    }
}
