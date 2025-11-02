#include <catch2/catch_test_macros.hpp>

#include <parse_context.h>
#include <ruleset.h>
#include <symbol_collection.h>

#include <stdexcept>
#include <string>
#include <vector>

using namespace muskox;

TEST_CASE("parse_context basics", "[parse_context]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t e_idx = sc.add_nterm("E");
    sc.validate();

    ruleset rs(sc);
    rs.add_rule("S", {"a", "E"});
    rs.add_rule("E", {});
    rs.validate();

    parse_context ctx(rs);

    SECTION("initial state")
    {
        REQUIRE(ctx.stack_empty(0));
        REQUIRE_THROWS_AS(ctx.get_back_state(0), std::out_of_range);
        REQUIRE(ctx.pop_n_from_stack(0, 0));
        REQUIRE_FALSE(ctx.pop_n_from_stack(0, 1));
    }

    SECTION("push and get back")
    {
        ctx.push_state_on_stack(0, 42);
        REQUIRE_FALSE(ctx.stack_empty(0));
        REQUIRE(ctx.get_back_state(0) == 42);

        ctx.push_state_on_stack(0, 100);
        REQUIRE(ctx.get_back_state(0) == 100);
    }

    SECTION("pop_n_from_stack")
    {
        ctx.push_state_on_stack(0, 1);
        ctx.push_state_on_stack(0, 2);
        ctx.push_state_on_stack(0, 3);

        REQUIRE(ctx.get_back_state(0) == 3);

        REQUIRE(ctx.pop_n_from_stack(0, 1));
        REQUIRE(ctx.get_back_state(0) == 2);

        REQUIRE(ctx.pop_n_from_stack(0, 2));
        REQUIRE(ctx.stack_empty(0));

        REQUIRE_FALSE(ctx.pop_n_from_stack(0, 1));
    }

    SECTION("pop_n_from_stack with n=0")
    {
        REQUIRE(ctx.pop_n_from_stack(0, 0));
        ctx.push_state_on_stack(0, 5);
        REQUIRE(ctx.pop_n_from_stack(0, 0));
        REQUIRE(ctx.get_back_state(0) == 5);
    }

    SECTION("errors handling")
    {
        REQUIRE(ctx.take_errors().empty());

        ctx.add_error("error1");
        ctx.add_error("error2");

        auto errs = ctx.take_errors();
        REQUIRE(errs.size() == 2);
        REQUIRE(errs[0] == "error1");
        REQUIRE(errs[1] == "error2");

        REQUIRE(ctx.take_errors().empty());
    }
}
