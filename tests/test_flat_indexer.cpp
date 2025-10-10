#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <flat_indexer.h>

using Catch::Matchers::Message;

TEST_CASE("flat_indexer basic operations", "[flat_indexer]")
{
    using FI1 = ptg::flat_indexer<1>;
    using FI2 = ptg::flat_indexer<2>;
    using FI3 = ptg::flat_indexer<3>;

    SECTION("constructor 1D")
    {
        FI1 fi({5});
        REQUIRE(fi.get_total_size() == 5);
    }

    SECTION("to_flat 1D")
    {
        FI1 fi({5});
        REQUIRE(fi.to_flat(0) == 0);
        REQUIRE(fi.to_flat(4) == 4);
    }

    SECTION("out of range 1D")
    {
        FI1 fi({5});
        REQUIRE_THROWS_MATCHES(
            fi.to_flat(5),
            std::out_of_range,
            Message("Index out of range")
        );
    }

    SECTION("zero size throws 1D")
    {
        REQUIRE_THROWS_MATCHES(
            []{ FI1({0}); }(),
            std::invalid_argument,
            Message("Size must be greater than 0")
        );
    }

    SECTION("constructor 2D")
    {
        FI2 fi({3, 4});
        REQUIRE(fi.get_total_size() == 12);
    }

    SECTION("to_flat 2D")
    {
        FI2 fi({3, 4});
        REQUIRE(fi.to_flat(0, 0) == 0);
        REQUIRE(fi.to_flat(2, 3) == 11);
        REQUIRE(fi.to_flat(1, 2) == 6);  // 1*4 + 2 = 6
    }

    SECTION("out of range 2D")
    {
        FI2 fi({3, 4});
        REQUIRE_THROWS_MATCHES(
            fi.to_flat(3, 0),
            std::out_of_range,
            Message("Index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            fi.to_flat(0, 4),
            std::out_of_range,
            Message("Index out of range")
        );
    }

    SECTION("constructor 3D")
    {
        FI3 fi({2, 3, 4});
        REQUIRE(fi.get_total_size() == 24);
    }

    SECTION("to_flat 3D")
    {
        FI3 fi({2, 3, 4});
        REQUIRE(fi.to_flat(0, 0, 0) == 0);
        REQUIRE(fi.to_flat(1, 2, 3) == 23);  // 1*12 + 2*4 + 3 = 23
        REQUIRE(fi.to_flat(0, 1, 2) == 6);   // 0*12 + 1*4 + 2 = 6
    }

    SECTION("out of range 3D")
    {
        FI3 fi({2, 3, 4});
        REQUIRE_THROWS_MATCHES(
            fi.to_flat(2, 0, 0),
            std::out_of_range,
            Message("Index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            fi.to_flat(0, 3, 0),
            std::out_of_range,
            Message("Index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            fi.to_flat(0, 0, 4),
            std::out_of_range,
            Message("Index out of range")
        );
    }

    SECTION("zero dim throws")
    {
        REQUIRE_THROWS_MATCHES(
            []{ ptg::flat_indexer<0>({}); }(),
            std::invalid_argument,
            Message("At least one dimension required")
        );
    }

    SECTION("wrong number of indices")
    {
        FI2 fi({3, 4});
        // These should not compile, but for documentation:
        // fi.to_flat(1);  // Compile error
        // fi.to_flat(1, 2, 3);  // Compile error
    }
}
