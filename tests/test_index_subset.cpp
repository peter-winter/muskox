#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <index_subset.h>

using Catch::Matchers::Message;

TEST_CASE("index_subset basic operations", "[index_subset]")
{
    using IS1 = ptg::index_subset<1>;
    using IS2 = ptg::index_subset<2>;
    using IS3 = ptg::index_subset<3>;

    SECTION("add and contains 1D")
    {
        IS1 is({100});
        REQUIRE(is.add(42) == true);  // New
        REQUIRE(is.contains(42) == true);
        REQUIRE(is.contains(0) == false);
        REQUIRE(is.add(42) == false);  // Duplicate
    }

    SECTION("multiple adds 1D")
    {
        IS1 is({100});
        REQUIRE(is.add(1) == true);
        REQUIRE(is.add(2) == true);
        REQUIRE(is.add(1) == false);  // Duplicate
        REQUIRE(is.contains(1) == true);
        REQUIRE(is.contains(2) == true);
        REQUIRE(is.contains(3) == false);
    }

    SECTION("out of range 1D")
    {
        IS1 is({100});
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

    SECTION("get_count 1D")
    {
        IS1 is({100});
        REQUIRE(is.get_count() == 0);
        REQUIRE(is.add(0) == true);
        REQUIRE(is.get_count() == 1);
        REQUIRE(is.add(50) == true);
        REQUIRE(is.get_count() == 2);
        REQUIRE(is.add(0) == false);  // Duplicate
        REQUIRE(is.get_count() == 2);
    }

    SECTION("get_indices 1D")
    {
        IS1 is({100});
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

    SECTION("get_size 1D")
    {
        IS1 is({100});
        REQUIRE(is.get_size() == 100);
    }

    SECTION("zero size throws 1D")
    {
        REQUIRE_THROWS_MATCHES(
            []{ IS1({0}); }(),
            std::invalid_argument,
            Message("Size must be greater than 0")
        );
    }

    SECTION("zero dimension throws")
    {
        REQUIRE_THROWS_MATCHES(
            []{ ptg::index_subset<0>({}); }(),
            std::invalid_argument,
            Message("At least one dimension required")
        );
    }

    SECTION("basic 2D")
    {
        IS2 is({5, 10});
        REQUIRE(is.get_size() == 50);
        REQUIRE(is.add(2, 3) == true);
        REQUIRE(is.contains(2, 3) == true);
        REQUIRE(is.contains(2, 4) == false);
        REQUIRE(is.add(2, 3) == false);  // Duplicate
        REQUIRE(is.get_count() == 1);
        const auto& inds = is.get_indices();
        REQUIRE(inds == std::vector<std::array<size_t, 2>>{{2, 3}});
    }

    SECTION("multiple adds 2D")
    {
        IS2 is({5, 10});
        REQUIRE(is.add(0, 0) == true);
        REQUIRE(is.add(4, 9) == true);
        REQUIRE(is.add(0, 0) == false);
        REQUIRE(is.get_count() == 2);
        const auto& inds = is.get_indices();
        REQUIRE(inds == std::vector<std::array<size_t, 2>>{{0, 0}, {4, 9}});
    }

    SECTION("out of range 2D")
    {
        IS2 is({5, 10});
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

    SECTION("basic 3D")
    {
        IS3 is({2, 3, 4});
        REQUIRE(is.get_size() == 24);
        REQUIRE(is.add(1, 2, 3) == true);
        REQUIRE(is.contains(1, 2, 3) == true);
        REQUIRE(is.contains(0, 0, 0) == false);
        REQUIRE(is.get_count() == 1);
        const auto& inds = is.get_indices();
        REQUIRE(inds == std::vector<std::array<size_t, 3>>{{1, 2, 3}});
    }

    SECTION("out of range 3D")
    {
        IS3 is({2, 3, 4});
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

    SECTION("multiple adds 3D")
    {
        IS3 is({2, 3, 4});
        REQUIRE(is.add(0, 0, 0) == true);
        REQUIRE(is.add(1, 2, 3) == true);
        REQUIRE(is.add(0, 0, 0) == false);
        REQUIRE(is.get_count() == 2);
        const auto& inds = is.get_indices();
        REQUIRE(inds == std::vector<std::array<size_t, 3>>{{0, 0, 0}, {1, 2, 3}});
    }

    SECTION("wrong number of indices")
    {
        IS2 is({5, 10});
        // These should not compile, but for documentation:
        // is.add(1);  // Compile error: wrong number
        // is.contains(1, 2, 3);  // Compile error
    }
}
