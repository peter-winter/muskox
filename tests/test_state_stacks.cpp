#include <catch2/catch_test_macros.hpp>

#include <state_stacks.h>

#include <stdexcept>

namespace ptg
{

TEST_CASE("state_stacks basics", "[state_stacks]")
{
    state_stacks ss;

    SECTION("initial state")
    {
        REQUIRE(ss.stack_empty(0));
        REQUIRE_THROWS_AS(ss.get_back_state(0), std::out_of_range);
        REQUIRE(ss.pop_n_from_stack(0, 0));
        REQUIRE_FALSE(ss.pop_n_from_stack(0, 1));
        REQUIRE_THROWS_AS(ss.stack_empty(1), std::out_of_range);
    }

    SECTION("push and get back")
    {
        ss.push_state_on_stack(0, 42);
        REQUIRE_FALSE(ss.stack_empty(0));
        REQUIRE(ss.get_back_state(0) == 42);

        ss.push_state_on_stack(0, 100);
        REQUIRE(ss.get_back_state(0) == 100);
    }

    SECTION("pop_n_from_stack")
    {
        ss.push_state_on_stack(0, 1);
        ss.push_state_on_stack(0, 2);
        ss.push_state_on_stack(0, 3);

        REQUIRE(ss.get_back_state(0) == 3);

        REQUIRE(ss.pop_n_from_stack(0, 1));
        REQUIRE(ss.get_back_state(0) == 2);

        REQUIRE(ss.pop_n_from_stack(0, 2));
        REQUIRE(ss.stack_empty(0));

        REQUIRE_FALSE(ss.pop_n_from_stack(0, 1));
    }

    SECTION("pop_n_from_stack with n=0")
    {
        REQUIRE(ss.pop_n_from_stack(0, 0));
        ss.push_state_on_stack(0, 5);
        REQUIRE(ss.pop_n_from_stack(0, 0));
        REQUIRE(ss.get_back_state(0) == 5);
    }

    SECTION("out of range index")
    {
        REQUIRE_THROWS_AS(ss.push_state_on_stack(1, 10), std::out_of_range);
        REQUIRE_THROWS_AS(ss.stack_empty(1), std::out_of_range);
        REQUIRE_THROWS_AS(ss.get_back_state(1), std::out_of_range);
        REQUIRE_THROWS_AS(ss.pop_n_from_stack(1, 0), std::out_of_range);
    }
}

} // namespace ptg
