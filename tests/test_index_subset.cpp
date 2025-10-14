// tests/test_index_subset.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <index_subset.h>

using Catch::Matchers::Message;

TEST_CASE("index_subset<1> basic operations", "[index_subset]")
{
    ptg::index_subset<1> is(10);
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

    ptg::index_subset<1> other(10);
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

    ptg::index_subset<1> other_diff(5);
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

TEST_CASE("index_subset<3> basic operations", "[index_subset]")
{
    std::array<size_t, 3> sizes = {2, 3, 4};
    ptg::index_subset<3> is(sizes);
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

    ptg::index_subset<3> other(sizes);
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

TEST_CASE("index_subset add with array", "[index_subset]")
{
    SECTION("1D - no array add, using regular add")
    {
        ptg::index_subset<1> is(10);
        is.add(5);
        REQUIRE(is.contains(5) == true);
    }

    SECTION("2D")
    {
        std::array<size_t, 2> sizes2 = {5, 10};
        ptg::index_subset<2> is2(sizes2);

        std::array<size_t, 2> arr1 = {1, 3};
        is2.add(arr1);
        REQUIRE(is2.contains(arr1) == true);
        REQUIRE(is2.get_count() == 1);

        std::array<size_t, 2> arr2 = {4, 7};
        is2.add(arr2);
        REQUIRE(is2.contains(arr2) == true);
        REQUIRE(is2.get_count() == 2);

        const auto& indices2 = is2.get_indices();
        REQUIRE(indices2[0] == arr1);
        REQUIRE(indices2[1] == arr2);
    }

    SECTION("3D")
    {
        std::array<size_t, 3> sizes3 = {4, 5, 6};
        ptg::index_subset<3> is3(sizes3);

        std::array<size_t, 3> arr1 = {0, 1, 2};
        is3.add(arr1);
        REQUIRE(is3.contains(arr1) == true);
        REQUIRE(is3.get_count() == 1);

        std::array<size_t, 3> arr2 = {3, 4, 5};
        is3.add(arr2);
        REQUIRE(is3.contains(arr2) == true);
        REQUIRE(is3.get_count() == 2);

        const auto& indices3 = is3.get_indices();
        REQUIRE(indices3[0] == arr1);
        REQUIRE(indices3[1] == arr2);
    }
}

TEST_CASE("index_subset contains with array", "[index_subset]")
{
    SECTION("2D")
    {
        std::array<size_t, 2> sizes = {5, 10};
        ptg::index_subset<2> is(sizes);
        is.add(1, 3);
        is.add(4, 7);

        REQUIRE(is.contains({1, 3}) == true);
        REQUIRE(is.contains({4, 7}) == true);
        REQUIRE(is.contains({0, 0}) == false);
        REQUIRE(is.contains({2, 5}) == false);
        REQUIRE(is.contains({4, 9}) == false);
    }

    SECTION("3D")
    {
        std::array<size_t, 3> sizes = {4, 5, 6};
        ptg::index_subset<3> is(sizes);
        is.add(0, 1, 2);
        is.add(3, 4, 5);

        REQUIRE(is.contains({0, 1, 2}) == true);
        REQUIRE(is.contains({3, 4, 5}) == true);
        REQUIRE(is.contains({0, 0, 0}) == false);
        REQUIRE(is.contains({1, 2, 3}) == false);
        REQUIRE(is.contains({2, 3, 4}) == false);
    }
}

TEST_CASE("index_subset<1> contains_all", "[index_subset]")
{
    ptg::index_subset<1> is(10);
    is.add(1);
    is.add(3);
    is.add(5);

    SECTION("empty other")
    {
        ptg::index_subset<1> other(10);
        REQUIRE(is.contains_all(other) == true);
    }

    SECTION("subset")
    {
        ptg::index_subset<1> other(10);
        other.add(3);
        other.add(5);

        REQUIRE(is.contains_all(other) == true);
    }

    SECTION("not subset")
    {
        ptg::index_subset<1> other(10);
        other.add(3);
        other.add(7);

        REQUIRE(is.contains_all(other) == false);
    }

    SECTION("same")
    {
        ptg::index_subset<1> other(10);
        other.add(1);
        other.add(3);
        other.add(5);

        REQUIRE(is.contains_all(other) == true);
    }

    SECTION("size mismatch")
    {
        ptg::index_subset<1> other(5);
        REQUIRE_THROWS_MATCHES(
            is.contains_all(other),
            std::invalid_argument,
            Message("Sizes don't match")
        );
    }
}

TEST_CASE("index_subset<2> contains_all", "[index_subset]")
{
    std::array<size_t, 2> sizes = {5, 10};
    ptg::index_subset<2> is(sizes);
    is.add(0, 1);
    is.add(2, 3);
    is.add(4, 5);

    SECTION("empty other")
    {
        ptg::index_subset<2> other(sizes);
        REQUIRE(is.contains_all(other) == true);
    }

    SECTION("subset")
    {
        ptg::index_subset<2> other(sizes);
        other.add(2, 3);
        other.add(4, 5);

        REQUIRE(is.contains_all(other) == true);
    }

    SECTION("not subset")
    {
        ptg::index_subset<2> other(sizes);
        other.add(2, 3);
        other.add(3, 7);

        REQUIRE(is.contains_all(other) == false);
    }

    SECTION("same")
    {
        ptg::index_subset<2> other(sizes);
        other.add(0, 1);
        other.add(2, 3);
        other.add(4, 5);

        REQUIRE(is.contains_all(other) == true);
    }

    SECTION("size mismatch")
    {
        std::array<size_t, 2> other_sizes = {3, 10};
        ptg::index_subset<2> other(other_sizes);
        REQUIRE_THROWS_MATCHES(
            is.contains_all(other),
            std::invalid_argument,
            Message("Sizes don't match")
        );
    }
}

TEST_CASE("index_subset<3> contains_all", "[index_subset]")
{
    std::array<size_t, 3> sizes = {4, 5, 6};
    ptg::index_subset<3> is(sizes);
    is.add(0, 1, 2);
    is.add(1, 2, 3);
    is.add(2, 3, 4);

    SECTION("empty other")
    {
        ptg::index_subset<3> other(sizes);
        REQUIRE(is.contains_all(other) == true);
    }

    SECTION("subset")
    {
        ptg::index_subset<3> other(sizes);
        other.add(1, 2, 3);
        other.add(2, 3, 4);

        REQUIRE(is.contains_all(other) == true);
    }

    SECTION("not subset")
    {
        ptg::index_subset<3> other(sizes);
        other.add(1, 2, 3);
        other.add(3, 4, 5);

        REQUIRE(is.contains_all(other) == false);
    }

    SECTION("same")
    {
        ptg::index_subset<3> other(sizes);
        other.add(0, 1, 2);
        other.add(1, 2, 3);
        other.add(2, 3, 4);

        REQUIRE(is.contains_all(other) == true);
    }

    SECTION("size mismatch")
    {
        std::array<size_t, 3> other_sizes = {4, 5, 7};
        ptg::index_subset<3> other(other_sizes);
        REQUIRE_THROWS_MATCHES(
            is.contains_all(other),
            std::invalid_argument,
            Message("Sizes don't match")
        );
    }
}
