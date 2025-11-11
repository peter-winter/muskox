#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <lr1_state.h>
#include <ruleset.h>
#include <symbol_collection.h>


using Catch::Matchers::Message;

using namespace muskox;

TEST_CASE("lr1_state", "[lr1_state]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_idx = sc.add_term("b");
    [[maybe_unused]] size_t c_idx = sc.add_term("c");
    [[maybe_unused]] size_t eof_idx = 0;
    [[maybe_unused]] size_t root_idx = 0;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t expr_idx = sc.add_nterm("Expr");
    sc.validate();
    
    ruleset rs(sc);

    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"Expr"});
    [[maybe_unused]] size_t expr_r0 = rs.add_rule("Expr", {"a", "Expr"});
    [[maybe_unused]] size_t expr_r1 = rs.add_rule("Expr", {"b"});
    [[maybe_unused]] size_t expr_r2 = rs.add_rule("Expr", {});
    rs.validate();

    auto dims = rs.get_lr1_set_item_space_dims();
    
    lr1_set_item_comp comp(rs);

    SECTION("basic construction")
    {
        lr1_sorted_set kernel(std::move(comp));
        kernel.insert({root_idx, 0, 0, eof_idx});

        lr1_state state(rs, std::move(kernel));
        
        REQUIRE(state.kernel_matches(lr1_set
        {
            {root_idx, 0, 0, eof_idx}
        }));

        REQUIRE(state.matches(lr1_set
        {
            {root_idx, 0, 0, eof_idx}
        }));
    }

    SECTION("construction with multiple kernel items")
    {
        lr1_sorted_set kernel(std::move(comp));
        kernel.insert({root_idx, 0, 0, eof_idx});
        kernel.insert({s_idx, s_r0, 0, eof_idx});

        lr1_state state(rs, std::move(kernel));
        
        REQUIRE(state.kernel_matches(lr1_set
        {
            {root_idx, 0, 0, eof_idx},
            {s_idx, s_r0, 0, eof_idx}
        }));

        REQUIRE(state.matches(lr1_set
        {
            {root_idx, 0, 0, eof_idx},
            {s_idx, s_r0, 0, eof_idx}
        }));
    }

    SECTION("add items")
    {
        lr1_sorted_set kernel(std::move(comp));
        kernel.insert({root_idx, 0, 0, eof_idx});

        lr1_state state(rs, std::move(kernel));
        
        REQUIRE(state.kernel_matches(lr1_set
        {
            {root_idx, 0, 0, eof_idx}
        }));
        
        REQUIRE(state.matches(lr1_set
        {
            {root_idx, 0, 0, eof_idx}
        }));

        ordered_bitset_nd<4> additional(dims);
        additional.add(s_idx, s_r0, 0, eof_idx);
        additional.add(expr_idx, expr_r0, 0, a_idx);

        state.add_items(additional);

        REQUIRE(state.matches(lr1_set
        {
            {expr_idx, expr_r0, 0, a_idx},
            {root_idx, 0, 0, eof_idx},
            {s_idx, s_r0, 0, eof_idx}
        }));
    }

    SECTION("add duplicate items")
    {
        lr1_sorted_set kernel(std::move(comp));
        kernel.insert({root_idx, 0, 0, eof_idx});

        lr1_state state(rs, std::move(kernel));
        
        REQUIRE(state.kernel_matches(lr1_set
        {
            {root_idx, 0, 0, eof_idx}
        }));
        
        REQUIRE(state.matches(lr1_set
        {
            {root_idx, 0, 0, eof_idx}
        }));

        ordered_bitset_nd<4> additional(dims);
        additional.add(root_idx, 0, 0, eof_idx);
        additional.add(s_idx, s_r0, 0, eof_idx);

        state.add_items(additional);

        REQUIRE(state.matches(lr1_set
        {
            {root_idx, 0, 0, eof_idx},
            {s_idx, s_r0, 0, eof_idx}
        }));
    }
    
    SECTION("to string - single item")
    {
        lr1_sorted_set kernel(std::move(comp));
        kernel.insert({root_idx, 0, 0, eof_idx});

        lr1_state state(rs, std::move(kernel));

        REQUIRE(state.to_string() == "$root -> . S / $eof");
    }

    SECTION("to string - multiple items")
    {
        lr1_sorted_set kernel(std::move(comp));
        kernel.insert({root_idx, 0, 0, eof_idx});

        lr1_state state(rs, std::move(kernel));

        ordered_bitset_nd<4> additional(dims);
        additional.add(s_idx, s_r0, 0, eof_idx);

        state.add_items(additional);

        REQUIRE(state.to_string() ==
            "$root -> . S / $eof\n"
            "S -> . Expr / $eof"
        );
    }

    SECTION("to string - shift item")
    {
        lr1_sorted_set kernel(std::move(comp));
        kernel.insert({expr_idx, expr_r0, 0, b_idx});

        lr1_state state(rs, std::move(kernel));

        REQUIRE(state.to_string() == "Expr -> . a Expr / b");
    }

    SECTION("to string - reduction item")
    {
        lr1_sorted_set kernel(std::move(comp));
        kernel.insert({expr_idx, expr_r1, 1, b_idx});

        lr1_state state(rs, std::move(kernel));

        REQUIRE(state.to_string() == "Expr -> b . / b");
    }

    SECTION("to string - empty production item")
    {
        lr1_sorted_set kernel(std::move(comp));
        kernel.insert({expr_idx, expr_r2, 0, a_idx});

        lr1_state state(rs, std::move(kernel));

        REQUIRE(state.to_string() == "Expr -> . / a");
    }

    SECTION("to string - mixed shift and reduce")
    {
        lr1_sorted_set kernel(std::move(comp));
        kernel.insert({expr_idx, expr_r0, 0, eof_idx});
        kernel.insert({expr_idx, expr_r1, 1, b_idx});

        lr1_state state(rs, std::move(kernel));

        REQUIRE(state.to_string() ==
            "Expr -> . a Expr / $eof\n"
            "Expr -> b . / b"
        );
    }

    SECTION("to string - mixed with empty production")
    {
        lr1_sorted_set kernel(std::move(comp));
        kernel.insert({expr_idx, expr_r0, 0, eof_idx});
        kernel.insert({expr_idx, expr_r2, 0, b_idx});

        lr1_state state(rs, std::move(kernel));

        REQUIRE(state.to_string() ==
            "Expr -> . a Expr / $eof\n"
            "Expr -> . / b"
        );
    }

    SECTION("to string - shift on nonterm, reduce, and empty")
    {
        lr1_sorted_set kernel(std::move(comp));
        kernel.insert({s_idx, s_r0, 0, eof_idx});
        kernel.insert({expr_idx, expr_r1, 1, a_idx});
        kernel.insert({expr_idx, expr_r2, 0, c_idx});

        lr1_state state(rs, std::move(kernel));

        REQUIRE(state.to_string() ==
            "Expr -> b . / a\n"
            "Expr -> . / c\n"
            "S -> . Expr / $eof"
        );
    }

    SECTION("to string - four mixed items")
    {
        lr1_sorted_set kernel(std::move(comp));
        kernel.insert({root_idx, 0, 0, eof_idx});
        kernel.insert({s_idx, s_r0, 0, eof_idx});
        kernel.insert({expr_idx, expr_r0, 1, b_idx});
        kernel.insert({expr_idx, expr_r2, 0, c_idx});

        lr1_state state(rs, std::move(kernel));

        REQUIRE(state.to_string() ==
            "Expr -> . / c\n"
            "$root -> . S / $eof\n"
            "S -> . Expr / $eof\n"
            "Expr -> a . Expr / b"
        );
    }
}

