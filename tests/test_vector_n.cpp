#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <vector_n.h>

using Catch::Matchers::Message;

using namespace muskox;

TEST_CASE("vector_n basic operations", "[vector_n]")
{
    using VN1 = vector_n<int, 1>;
    using VN2 = vector_n<int, 2>;
    using VN3 = vector_n<int, 3>;

    SECTION("constructor 1D")
    {
        VN1 v({5}, 42);
        REQUIRE(v.get_size() == 5);
        REQUIRE(v.get(0) == 42);
        REQUIRE(v.get(4) == 42);
    }

    SECTION("get non-const 1D")
    {
        VN1 v({5}, 42);
        v.get(2) = 100;
        REQUIRE(v.get(2) == 100);
    }

    SECTION("get const 1D")
    {
        const VN1 v({5}, 42);
        REQUIRE(v.get(3) == 42);
    }

    SECTION("out of range 1D")
    {
        VN1 v({5}, 0);
        REQUIRE_THROWS_MATCHES(
            v.get(5),
            std::out_of_range,
            Message("Index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            static_cast<const VN1&>(v).get(5),
            std::out_of_range,
            Message("Index out of range")
        );
    }

    SECTION("zero size throws 1D")
    {
        REQUIRE_THROWS_MATCHES(
            []{ VN1({0}, 0); }(),
            std::invalid_argument,
            Message("Size must be greater than 0")
        );
    }

    SECTION("constructor 2D")
    {
        VN2 v({3, 4}, 10);
        REQUIRE(v.get_size() == 12);
        REQUIRE(v.get(0, 0) == 10);
        REQUIRE(v.get(2, 3) == 10);
    }

    SECTION("get non-const 2D")
    {
        VN2 v({3, 4}, 10);
        v.get(1, 2) = 50;
        REQUIRE(v.get(1, 2) == 50);
    }

    SECTION("get const 2D")
    {
        const VN2 v({3, 4}, 10);
        REQUIRE(v.get(2, 1) == 10);
    }

    SECTION("out of range 2D")
    {
        VN2 v({3, 4}, 0);
        REQUIRE_THROWS_MATCHES(
            v.get(3, 0),
            std::out_of_range,
            Message("Index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            v.get(0, 4),
            std::out_of_range,
            Message("Index out of range")
        );
    }

    SECTION("constructor 3D")
    {
        VN3 v({2, 3, 4}, 5);
        REQUIRE(v.get_size() == 24);
        REQUIRE(v.get(0, 0, 0) == 5);
        REQUIRE(v.get(1, 2, 3) == 5);
    }

    SECTION("get non-const 3D")
    {
        VN3 v({2, 3, 4}, 5);
        v.get(1, 1, 1) = 99;
        REQUIRE(v.get(1, 1, 1) == 99);
    }

    SECTION("get const 3D")
    {
        const VN3 v({2, 3, 4}, 5);
        REQUIRE(v.get(0, 2, 3) == 5);
    }

    SECTION("out of range 3D")
    {
        VN3 v({2, 3, 4}, 0);
        REQUIRE_THROWS_MATCHES(
            v.get(2, 0, 0),
            std::out_of_range,
            Message("Index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            v.get(0, 3, 0),
            std::out_of_range,
            Message("Index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            v.get(0, 0, 4),
            std::out_of_range,
            Message("Index out of range")
        );
    }

    SECTION("wrong number of indices")
    {
        VN2 v({3, 4}, 0);
        // These should not compile, but for documentation:
        // v.get(1);  // Compile error
        // v.get(1, 2, 3);  // Compile error
    }
}
