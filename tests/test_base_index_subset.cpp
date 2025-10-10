#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <base_index_subset.h>

using Catch::Matchers::Message;

TEST_CASE("base_index_subset basic operations", "[base_index_subset]")
{
    using BIS1 = ptg::base_index_subset<1>;
    using BIS2 = ptg::base_index_subset<2>;
    using BIS3 = ptg::base_index_subset<3>;

    SECTION("constructor 1D")
    {
        BIS1 bis({5});
        REQUIRE(bis.get_size() == 5);
        REQUIRE(bis.contains(0) == false);
        REQUIRE(bis.contains(4) == false);
    }

    SECTION("constructor with true 1D")
    {
        BIS1 bis({5}, true);
        REQUIRE(bis.get_size() == 5);
        REQUIRE(bis.contains(0) == true);
        REQUIRE(bis.contains(4) == true);
    }

    SECTION("add 1D")
    {
        BIS1 bis({5});
        REQUIRE(bis.add(2) == true);
        REQUIRE(bis.contains(2) == true);
        REQUIRE(bis.add(2) == false);  // Already present
    }

    SECTION("remove 1D")
    {
        BIS1 bis({5});
        bis.add(3);
        REQUIRE(bis.remove(3) == true);
        REQUIRE(bis.contains(3) == false);
        REQUIRE(bis.remove(3) == false);  // Not present
    }

    SECTION("out of range 1D")
    {
        BIS1 bis({5});
        REQUIRE_THROWS_MATCHES(
            bis.add(5),
            std::out_of_range,
            Message("Index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            bis.contains(5),
            std::out_of_range,
            Message("Index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            bis.remove(5),
            std::out_of_range,
            Message("Index out of range")
        );
    }

    SECTION("zero size throws 1D")
    {
        REQUIRE_THROWS_MATCHES(
            []{ BIS1({0}); }(),
            std::invalid_argument,
            Message("Size must be greater than 0")
        );
    }

    SECTION("constructor 2D")
    {
        BIS2 bis({3, 4});
        REQUIRE(bis.get_size() == 12);
    }

    SECTION("add 2D")
    {
        BIS2 bis({3, 4});
        REQUIRE(bis.add(1, 2) == true);
        REQUIRE(bis.contains(1, 2) == true);
        REQUIRE(bis.add(1, 2) == false);
    }

    SECTION("remove 2D")
    {
        BIS2 bis({3, 4});
        bis.add(2, 3);
        REQUIRE(bis.remove(2, 3) == true);
        REQUIRE(bis.contains(2, 3) == false);
    }

    SECTION("out of range 2D")
    {
        BIS2 bis({3, 4});
        REQUIRE_THROWS_MATCHES(
            bis.add(3, 0),
            std::out_of_range,
            Message("Index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            bis.add(0, 4),
            std::out_of_range,
            Message("Index out of range")
        );
    }

    SECTION("constructor 3D")
    {
        BIS3 bis({2, 3, 4});
        REQUIRE(bis.get_size() == 24);
    }

    SECTION("add 3D")
    {
        BIS3 bis({2, 3, 4});
        REQUIRE(bis.add(1, 2, 3) == true);
        REQUIRE(bis.contains(1, 2, 3) == true);
    }

    SECTION("bulk add from another")
    {
        BIS2 bis1({3, 4});
        BIS2 bis2({3, 4});

        bis1.add(0, 0);
        bis1.add(1, 1);
        bis1.add(0, 0);  // Duplicate, no effect

        bis2.add(bis1);
        REQUIRE(bis2.contains(0, 0) == true);
        REQUIRE(bis2.contains(1, 1) == true);

        // Adding again should have no effect beyond what's already there
        bis2.add(bis1);
        
        REQUIRE(bis2.contains(0, 0) == true);
        REQUIRE(bis2.contains(1, 1) == true);
    }

    SECTION("bulk add mismatch sizes")
    {
        BIS2 bis1({3, 4});
        BIS2 bis2({2, 5});  // Different sizes, total_size 10 != 12

        REQUIRE_THROWS_MATCHES(
            bis1.add(bis2),
            std::invalid_argument,
            Message("Sizes don't match")
        );
    }

    SECTION("wrong number of indices")
    {
        BIS2 bis({3, 4});
        // bis.add(1);  // Compile error
        // bis.add(1, 2, 3);  // Compile error
    }
}
