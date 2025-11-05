#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <ordered_bitset_nd.h>

#include <ranges>

using Catch::Matchers::Message;

using namespace muskox;

TEST_CASE("ordered_bitset_nd<1> basic operations", "[ordered_bitset_nd]")
{
    ordered_bitset_nd<1> ob(10);
    REQUIRE(ob.get_size() == 10);
    REQUIRE(ob.get_count() == 0);

    bool added1 = ob.add(3);
    REQUIRE(added1 == true);
    REQUIRE(ob.contains(3) == true);
    REQUIRE(ob.get_count() == 1);

    bool added2 = ob.add(3);
    REQUIRE(added2 == false);
    REQUIRE(ob.get_count() == 1);

    bool added3 = ob.add(7);
    REQUIRE(added3 == true);
    REQUIRE(ob.contains(7) == true);
    REQUIRE(ob.get_count() == 2);

    const auto& indices = ob.get_indices();
    REQUIRE(indices.size() == 2);
    REQUIRE(indices[0] == 3);
    REQUIRE(indices[1] == 7);

    ordered_bitset_nd<1> other(10);
    other.add(1);
    other.add(4);

    ob.add(4);
    ob.add(6);

    ob.add(other);

    REQUIRE(ob.get_count() == 5);
    REQUIRE(ob.contains(1) == true);
    REQUIRE(ob.contains(3) == true);
    REQUIRE(ob.contains(4) == true);
    REQUIRE(ob.contains(6) == true);
    REQUIRE(ob.contains(7) == true);

    ordered_bitset_nd<1> other_diff(5);
    REQUIRE_THROWS_MATCHES(
        ob.add(other_diff),
        std::invalid_argument,
        Message("Sizes don't match")
    );

    REQUIRE_THROWS_MATCHES(
        ob.add(10),
        std::out_of_range,
        Message("Index out of range")
    );
    REQUIRE_THROWS_MATCHES(
        ob.contains(10),
        std::out_of_range,
        Message("Index out of range")
    );
}

TEST_CASE("ordered_bitset_nd<3> basic operations", "[ordered_bitset_nd]")
{
    std::array<size_t, 3> sizes = {2, 3, 4};
    ordered_bitset_nd<3> ob(sizes);
    REQUIRE(ob.get_size() == 24);
    REQUIRE(ob.get_count() == 0);

    bool added1 = ob.add(1, 2, 3);
    REQUIRE(added1 == true);
    REQUIRE(ob.contains(1, 2, 3) == true);
    REQUIRE(ob.get_count() == 1);

    bool added2 = ob.add(1, 2, 3);
    REQUIRE(added2 == false);
    REQUIRE(ob.get_count() == 1);

    bool added3 = ob.add(0, 0, 0);
    REQUIRE(added3 == true);
    REQUIRE(ob.contains(0, 0, 0) == true);
    REQUIRE(ob.get_count() == 2);

    const auto& indices = ob.get_indices();
    REQUIRE(indices.size() == 2);
    REQUIRE(indices[0] == std::array<size_t, 3>{1, 2, 3});
    REQUIRE(indices[1] == std::array<size_t, 3>{0, 0, 0});

    ordered_bitset_nd<3> other(sizes);
    other.add(1, 1, 1);
    other.add(0, 2, 3);

    ob.add(0, 2, 3);
    ob.add(1, 0, 2);

    ob.add(other);

    REQUIRE(ob.get_count() == 5);
    REQUIRE(ob.contains(0, 0, 0) == true);
    REQUIRE(ob.contains(0, 2, 3) == true);
    REQUIRE(ob.contains(1, 0, 2) == true);
    REQUIRE(ob.contains(1, 1, 1) == true);
    REQUIRE(ob.contains(1, 2, 3) == true);

    ordered_bitset_nd<3> other_diff({2, 3, 5});
    REQUIRE_THROWS_MATCHES(
        ob.add(other_diff),
        std::invalid_argument,
        Message("Sizes don't match")
    );

    REQUIRE_THROWS_MATCHES(
        ob.add(2, 0, 0),
        std::out_of_range,
        Message("Index out of range")
    );
    REQUIRE_THROWS_MATCHES(
        ob.contains(0, 3, 0),
        std::out_of_range,
        Message("Index out of range")
    );
    REQUIRE_THROWS_MATCHES(
        ob.contains(0, 0, 4),
        std::out_of_range,
        Message("Index out of range")
    );
}

TEST_CASE("ordered_bitset_nd add with array", "[ordered_bitset_nd]")
{
    SECTION("1D - no array add, using regular add")
    {
        ordered_bitset_nd<1> ob(10);
        ob.add(5);
        REQUIRE(ob.contains(5) == true);
    }

    SECTION("2D")
    {
        std::array<size_t, 2> sizes2 = {5, 10};
        ordered_bitset_nd<2> is2(sizes2);

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
        ordered_bitset_nd<3> is3(sizes3);

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

TEST_CASE("ordered_bitset_nd contains with array", "[ordered_bitset_nd]")
{
    SECTION("2D")
    {
        std::array<size_t, 2> sizes = {5, 10};
        ordered_bitset_nd<2> ob(sizes);
        ob.add(1, 3);
        ob.add(4, 7);

        REQUIRE(ob.contains({1, 3}) == true);
        REQUIRE(ob.contains({4, 7}) == true);
        REQUIRE(ob.contains({0, 0}) == false);
        REQUIRE(ob.contains({2, 5}) == false);
        REQUIRE(ob.contains({4, 9}) == false);
    }

    SECTION("3D")
    {
        std::array<size_t, 3> sizes = {4, 5, 6};
        ordered_bitset_nd<3> ob(sizes);
        ob.add(0, 1, 2);
        ob.add(3, 4, 5);

        REQUIRE(ob.contains({0, 1, 2}) == true);
        REQUIRE(ob.contains({3, 4, 5}) == true);
        REQUIRE(ob.contains({0, 0, 0}) == false);
        REQUIRE(ob.contains({1, 2, 3}) == false);
        REQUIRE(ob.contains({2, 3, 4}) == false);
    }
}

TEST_CASE("ordered_bitset_nd<1> contains_all", "[ordered_bitset_nd]")
{
    ordered_bitset_nd<1> ob(10);
    ob.add(1);
    ob.add(3);
    ob.add(5);

    SECTION("empty other")
    {
        ordered_bitset_nd<1> other(10);
        REQUIRE(ob.contains_all(other) == true);
    }

    SECTION("subset")
    {
        ordered_bitset_nd<1> other(10);
        other.add(3);
        other.add(5);

        REQUIRE(ob.contains_all(other) == true);
    }

    SECTION("not subset")
    {
        ordered_bitset_nd<1> other(10);
        other.add(3);
        other.add(7);

        REQUIRE(ob.contains_all(other) == false);
    }

    SECTION("same")
    {
        ordered_bitset_nd<1> other(10);
        other.add(1);
        other.add(3);
        other.add(5);

        REQUIRE(ob.contains_all(other) == true);
    }

    SECTION("size mismatch")
    {
        ordered_bitset_nd<1> other(5);
        REQUIRE_THROWS_MATCHES(
            ob.contains_all(other),
            std::invalid_argument,
            Message("Sizes don't match")
        );
    }
}

TEST_CASE("ordered_bitset_nd<2> contains_all", "[ordered_bitset_nd]")
{
    std::array<size_t, 2> sizes = {5, 10};
    ordered_bitset_nd<2> ob(sizes);
    ob.add(0, 1);
    ob.add(2, 3);
    ob.add(4, 5);

    SECTION("empty other")
    {
        ordered_bitset_nd<2> other(sizes);
        REQUIRE(ob.contains_all(other) == true);
    }

    SECTION("subset")
    {
        ordered_bitset_nd<2> other(sizes);
        other.add(2, 3);
        other.add(4, 5);

        REQUIRE(ob.contains_all(other) == true);
    }

    SECTION("not subset")
    {
        ordered_bitset_nd<2> other(sizes);
        other.add(2, 3);
        other.add(3, 7);

        REQUIRE(ob.contains_all(other) == false);
    }

    SECTION("same")
    {
        ordered_bitset_nd<2> other(sizes);
        other.add(0, 1);
        other.add(2, 3);
        other.add(4, 5);

        REQUIRE(ob.contains_all(other) == true);
    }

    SECTION("size mismatch")
    {
        std::array<size_t, 2> other_sizes = {3, 10};
        ordered_bitset_nd<2> other(other_sizes);
        REQUIRE_THROWS_MATCHES(
            ob.contains_all(other),
            std::invalid_argument,
            Message("Sizes don't match")
        );
    }
}

TEST_CASE("ordered_bitset_nd<3> contains_all", "[ordered_bitset_nd]")
{
    std::array<size_t, 3> sizes = {4, 5, 6};
    ordered_bitset_nd<3> ob(sizes);
    ob.add(0, 1, 2);
    ob.add(1, 2, 3);
    ob.add(2, 3, 4);

    SECTION("empty other")
    {
        ordered_bitset_nd<3> other(sizes);
        REQUIRE(ob.contains_all(other) == true);
    }

    SECTION("subset")
    {
        ordered_bitset_nd<3> other(sizes);
        other.add(1, 2, 3);
        other.add(2, 3, 4);

        REQUIRE(ob.contains_all(other) == true);
    }

    SECTION("not subset")
    {
        ordered_bitset_nd<3> other(sizes);
        other.add(1, 2, 3);
        other.add(3, 4, 5);

        REQUIRE(ob.contains_all(other) == false);
    }

    SECTION("same")
    {
        ordered_bitset_nd<3> other(sizes);
        other.add(0, 1, 2);
        other.add(1, 2, 3);
        other.add(2, 3, 4);

        REQUIRE(ob.contains_all(other) == true);
    }

    SECTION("size mismatch")
    {
        std::array<size_t, 3> other_sizes = {4, 5, 7};
        ordered_bitset_nd<3> other(other_sizes);
        REQUIRE_THROWS_MATCHES(
            ob.contains_all(other),
            std::invalid_argument,
            Message("Sizes don't match")
        );
    }
}

TEST_CASE("ordered_bitset_nd<1> contains_only_items", "[ordered_bitset_nd]")
{
    ordered_bitset_nd<1> ob(10);
    ob.add(1);
    ob.add(3);
    ob.add(5);

    SECTION("empty other")
    {
        ordered_bitset_nd<1> other(10);
        REQUIRE(ob.contains_only_items(other) == false);
    }

    SECTION("subset")
    {
        ordered_bitset_nd<1> other(10);
        other.add(1);
        other.add(3);

        REQUIRE(ob.contains_only_items(other) == false);
    }

    SECTION("not subset")
    {
        ordered_bitset_nd<1> other(10);
        other.add(1);
        other.add(7);

        REQUIRE(ob.contains_only_items(other) == false);
    }

    SECTION("same")
    {
        ordered_bitset_nd<1> other(10);
        other.add(1);
        other.add(3);
        other.add(5);

        REQUIRE(ob.contains_only_items(other) == true);
    }

    SECTION("same but different order")
    {
        ordered_bitset_nd<1> other(10);
        other.add(5);
        other.add(1);
        other.add(3);

        REQUIRE(ob.contains_only_items(other) == true);
    }

    SECTION("size mismatch")
    {
        ordered_bitset_nd<1> other(5);
        REQUIRE_THROWS_MATCHES(
            ob.contains_only_items(other),
            std::invalid_argument,
            Message("Sizes don't match")
        );
    }
}

TEST_CASE("ordered_bitset_nd<2> contains_only_items", "[ordered_bitset_nd]")
{
    std::array<size_t, 2> sizes = {5, 10};
    ordered_bitset_nd<2> ob(sizes);
    ob.add(0, 1);
    ob.add(2, 3);
    ob.add(4, 5);

    SECTION("empty other")
    {
        ordered_bitset_nd<2> other(sizes);
        REQUIRE(ob.contains_only_items(other) == false);
    }

    SECTION("subset")
    {
        ordered_bitset_nd<2> other(sizes);
        other.add(2, 3);
        other.add(4, 5);

        REQUIRE(ob.contains_only_items(other) == false);
    }

    SECTION("not subset")
    {
        ordered_bitset_nd<2> other(sizes);
        other.add(2, 3);
        other.add(3, 7);

        REQUIRE(ob.contains_only_items(other) == false);
    }

    SECTION("same")
    {
        ordered_bitset_nd<2> other(sizes);
        other.add(0, 1);
        other.add(2, 3);
        other.add(4, 5);

        REQUIRE(ob.contains_only_items(other) == true);
    }

    SECTION("same but different order")
    {
        ordered_bitset_nd<2> other(sizes);
        other.add(4, 5);
        other.add(0, 1);
        other.add(2, 3);

        REQUIRE(ob.contains_only_items(other) == true);
    }

    SECTION("size mismatch")
    {
        std::array<size_t, 2> other_sizes = {3, 10};
        ordered_bitset_nd<2> other(other_sizes);
        REQUIRE_THROWS_MATCHES(
            ob.contains_only_items(other),
            std::invalid_argument,
            Message("Sizes don't match")
        );
    }
}

TEST_CASE("ordered_bitset_nd<3> contains_only_items", "[ordered_bitset_nd]")
{
    std::array<size_t, 3> sizes = {4, 5, 6};
    ordered_bitset_nd<3> ob(sizes);
    ob.add(0, 1, 2);
    ob.add(1, 2, 3);
    ob.add(2, 3, 4);

    SECTION("empty other")
    {
        ordered_bitset_nd<3> other(sizes);
        REQUIRE(ob.contains_only_items(other) == false);
    }

    SECTION("subset")
    {
        ordered_bitset_nd<3> other(sizes);
        other.add(1, 2, 3);
        other.add(2, 3, 4);

        REQUIRE(ob.contains_only_items(other) == false);
    }

    SECTION("not subset")
    {
        ordered_bitset_nd<3> other(sizes);
        other.add(1, 2, 3);
        other.add(3, 4, 5);

        REQUIRE(ob.contains_only_items(other) == false);
    }

    SECTION("same")
    {
        ordered_bitset_nd<3> other(sizes);
        other.add(0, 1, 2);
        other.add(1, 2, 3);
        other.add(2, 3, 4);

        REQUIRE(ob.contains_only_items(other) == true);
    }

    SECTION("same but different order")
    {
        ordered_bitset_nd<3> other(sizes);
        other.add(2, 3, 4);
        other.add(0, 1, 2);
        other.add(1, 2, 3);

        REQUIRE(ob.contains_only_items(other) == true);
    }

    SECTION("size mismatch")
    {
        std::array<size_t, 3> other_sizes = {4, 5, 7};
        ordered_bitset_nd<3> other(other_sizes);
        REQUIRE_THROWS_MATCHES(
            ob.contains_only_items(other),
            std::invalid_argument,
            Message("Sizes don't match")
        );
    }
}

TEST_CASE("ordered_bitset<N> comp", "[ordered_bitset_nd]")
{
    auto f = [](const auto& i1, const auto& i2){ return i1[0] + i1[1] < i2[0] + i2[1]; };
    using ob_type = ordered_bitset_nd<2, decltype(f)>;
    ob_type ob({10, 10}, f);
    
    auto equiv = [&f](const auto& a, const auto& b) {
        return !f(a, b) && !f(b, a);
    };
    
    ob.add(0, 0);
    ob.add(2, 3);
    ob.add(5, 6);
    ob.add(4, 7);
    ob.add(1, 4);
    
    auto grouped = ob.get_indices() | std::views::chunk_by(equiv);

    std::vector<std::vector<std::pair<int, int>>> pairs;
    
    for (const auto& group : grouped)
    {
        if (!group.empty())
        {
            pairs.push_back({});
            for (const auto& elem : group)
            {
                pairs.back().push_back({elem[0], elem[1]});
            }
        }
    }
    
    REQUIRE(pairs.size() == 3);
    REQUIRE(pairs[0].size() == 1);
    REQUIRE(pairs[0][0] == std::pair{0, 0});
    REQUIRE(pairs[1].size() == 2);
    REQUIRE(pairs[1][0] == std::pair{2, 3});
    REQUIRE(pairs[1][1] == std::pair{1, 4});
    REQUIRE(pairs[2].size() == 2);
    REQUIRE(pairs[2][0] == std::pair{5, 6});
    REQUIRE(pairs[2][1] == std::pair{4, 7});
}

TEST_CASE("ordered_bitset<1> comp", "[ordered_bitset_nd]")
{
    auto f = [](size_t a, size_t b){ return a % 3 < b % 3; };
    using ob_type = ordered_bitset_nd<1, decltype(f)>;
    ob_type ob(10, f);
    
    auto equiv = [&f](size_t a, size_t b) {
        return !f(a, b) && !f(b, a);
    };
    
    ob.add(0);
    ob.add(1);
    ob.add(2);
    ob.add(3);
    ob.add(4);
    ob.add(5);
    
    auto grouped = ob.get_indices() | std::views::chunk_by(equiv);

    std::vector<std::vector<size_t>> numbers;
    
    for (const auto& group : grouped)
    {
        if (!group.empty())
        {
            numbers.push_back({});
            for (size_t n : group)
            {
                numbers.back().push_back(n);
            }
        }
    }
    
    REQUIRE(numbers.size() == 3);
    REQUIRE(numbers[0].size() == 2);
    REQUIRE(numbers[0][0] == 0);
    REQUIRE(numbers[0][1] == 3);
    REQUIRE(numbers[1].size() == 2);
    REQUIRE(numbers[1][0] == 1);
    REQUIRE(numbers[1][1] == 4);
    REQUIRE(numbers[2].size() == 2);
    REQUIRE(numbers[2][0] == 2);
    REQUIRE(numbers[2][1] == 5);
}
