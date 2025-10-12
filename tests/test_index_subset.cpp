// tests/test_index_subset.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <index_subset.h>

using Catch::Matchers::Message;

TEST_CASE("index_subset basic operations", "[index_subset]")
{
    SECTION("1D specialization")
    {
        ptg::index_subset<1> is({10});
        REQUIRE(is.get_size() == 10);
        REQUIRE(is.get_count() == 0);

        bool added1 = is.add(3);
        REQUIRE(added1 == true);
        REQUIRE(is.contains(3) == true);
        REQUIRE(is.get_count() == 1);

        bool added2 = is.add(3);
        REQUIRE(added2 == false);
        REQUIRE(is.get_count() == 1);

        bool added3 = is.add(7);
        REQUIRE(added3 == true);
        REQUIRE(is.contains(7) == true);
        REQUIRE(is.get_count() == 2);

        const auto& indices = is.get_indices();
        REQUIRE(indices.size() == 2);
        REQUIRE(indices[0] == 3);
        REQUIRE(indices[1] == 7);

        ptg::index_subset<1> other({10});
        other.add(1);
        other.add(4);

        is.add(4);
        is.add(6);

        is.add(other);

        REQUIRE(is.get_count() == 5);
        REQUIRE(is.contains(1) == true);
        REQUIRE(is.contains(3) == true);
        REQUIRE(is.contains(4) == true);
        REQUIRE(is.contains(6) == true);
        REQUIRE(is.contains(7) == true);

        ptg::index_subset<1> other_diff({5});
        REQUIRE_THROWS_MATCHES(
            is.add(other_diff),
            std::invalid_argument,
            Message("Sizes don't match")
        );

        REQUIRE_THROWS_MATCHES(
            is.add(10),
            std::out_of_range,
            Message("Index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            is.contains(10),
            std::out_of_range,
            Message("Index out of range")
        );
    }

    SECTION("2D general")
    {
        ptg::index_subset<2> is({3, 4});
        REQUIRE(is.get_size() == 12);
        REQUIRE(is.get_count() == 0);

        bool added1 = is.add(1, 2);
        REQUIRE(added1 == true);
        REQUIRE(is.contains(1, 2) == true);
        REQUIRE(is.get_count() == 1);

        bool added2 = is.add(1, 2);
        REQUIRE(added2 == false);
        REQUIRE(is.get_count() == 1);

        bool added3 = is.add(0, 3);
        REQUIRE(added3 == true);
        REQUIRE(is.contains(0, 3) == true);
        REQUIRE(is.get_count() == 2);

        const auto& indices = is.get_indices();
        REQUIRE(indices.size() == 2);
        REQUIRE(indices[0] == std::array<size_t, 2>{1, 2});
        REQUIRE(indices[1] == std::array<size_t, 2>{0, 3});

        ptg::index_subset<2> other({3, 4});
        other.add(2, 1);
        other.add(0, 0);

        is.add(0, 0);
        is.add(2, 2);

        is.add(other);

        REQUIRE(is.get_count() == 5);
        REQUIRE(is.contains(0, 0) == true);
        REQUIRE(is.contains(0, 3) == true);
        REQUIRE(is.contains(1, 2) == true);
        REQUIRE(is.contains(2, 1) == true);
        REQUIRE(is.contains(2, 2) == true);

        ptg::index_subset<2> other_diff({3, 5});
        REQUIRE_THROWS_MATCHES(
            is.add(other_diff),
            std::invalid_argument,
            Message("Sizes don't match")
        );

        REQUIRE_THROWS_MATCHES(
            is.add(3, 0),
            std::out_of_range,
            Message("Index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            is.contains(0, 4),
            std::out_of_range,
            Message("Index out of range")
        );
    }

    SECTION("3D general")
    {
        ptg::index_subset<3> is({2, 3, 4});
        REQUIRE(is.get_size() == 24);
        REQUIRE(is.get_count() == 0);

        bool added1 = is.add(1, 2, 3);
        REQUIRE(added1 == true);
        REQUIRE(is.contains(1, 2, 3) == true);
        REQUIRE(is.get_count() == 1);

        bool added2 = is.add(1, 2, 3);
        REQUIRE(added2 == false);
        REQUIRE(is.get_count() == 1);

        bool added3 = is.add(0, 0, 0);
        REQUIRE(added3 == true);
        REQUIRE(is.contains(0, 0, 0) == true);
        REQUIRE(is.get_count() == 2);

        const auto& indices = is.get_indices();
        REQUIRE(indices.size() == 2);
        REQUIRE(indices[0] == std::array<size_t, 3>{1, 2, 3});
        REQUIRE(indices[1] == std::array<size_t, 3>{0, 0, 0});

        ptg::index_subset<3> other({2, 3, 4});
        other.add(1, 1, 1);
        other.add(0, 2, 3);

        is.add(0, 2, 3);
        is.add(1, 0, 2);

        is.add(other);

        REQUIRE(is.get_count() == 5);
        REQUIRE(is.contains(0, 0, 0) == true);
        REQUIRE(is.contains(0, 2, 3) == true);
        REQUIRE(is.contains(1, 0, 2) == true);
        REQUIRE(is.contains(1, 1, 1) == true);
        REQUIRE(is.contains(1, 2, 3) == true);

        ptg::index_subset<3> other_diff({2, 3, 5});
        REQUIRE_THROWS_MATCHES(
            is.add(other_diff),
            std::invalid_argument,
            Message("Sizes don't match")
        );

        REQUIRE_THROWS_MATCHES(
            is.add(2, 0, 0),
            std::out_of_range,
            Message("Index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            is.contains(0, 3, 0),
            std::out_of_range,
            Message("Index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            is.contains(0, 0, 4),
            std::out_of_range,
            Message("Index out of range")
        );
    }
}
