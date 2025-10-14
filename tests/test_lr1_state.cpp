#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <grammar_error.h>
#include <index_subset.h>
#include <lr1_set_item.h>
#include <lr1_state.h>
#include <ruleset.h>
#include <symbol_collection.h>

using Catch::Matchers::Message;

TEST_CASE("lr1_state construction and add_items", "[lr1_state]")
{
    ptg::symbol_collection sc;
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_idx = sc.add_term("b");
    [[maybe_unused]] size_t c_idx = sc.add_term("c");
    [[maybe_unused]] size_t eof_idx = 0;  // $eof

    [[maybe_unused]] size_t root_idx = 0;  // $root
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t expr_idx = sc.add_nterm("Expr");

    ptg::ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"Expr"});
    [[maybe_unused]] size_t expr_r0 = rs.add_rule("Expr", {"a", "Expr"});
    [[maybe_unused]] size_t expr_r1 = rs.add_rule("Expr", {"b"});
    [[maybe_unused]] size_t expr_r2 = rs.add_rule("Expr", {});

    std::array<size_t, 4> dims = rs.get_lr1_set_item_space_dims();

    SECTION("construction with kernel")
    {
        ptg::index_subset<4> kernel(dims);
        kernel.add(root_idx, 0, 0, eof_idx);  // $root -> . S / $eof

        ptg::lr1_state state(rs, std::move(kernel));

        REQUIRE(state.contains_all_items(kernel) == true);
    }

    SECTION("add_items and contains_all_items")
    {
        ptg::index_subset<4> kernel(dims);
        kernel.add(root_idx, 0, 0, eof_idx);  // $root -> . S / $eof

        ptg::lr1_state state(rs, std::move(kernel));

        ptg::index_subset<4> additional(dims);
        additional.add(s_idx, s_r0, 0, eof_idx);  // S -> . Expr / $eof
        additional.add(expr_idx, expr_r0, 0, eof_idx);  // Expr -> . a Expr / $eof
        additional.add(expr_idx, expr_r1, 0, eof_idx);  // Expr -> . b / $eof
        additional.add(expr_idx, expr_r2, 0, eof_idx);  // Expr -> . / $eof

        state.add_items(additional);

        REQUIRE(state.contains_all_items(additional) == true);
    }

    SECTION("get_symbol_items for terminal")
    {
        ptg::index_subset<4> kernel(dims);
        kernel.add(expr_idx, expr_r0, 1, c_idx);  // Expr -> a . Expr / c

        ptg::lr1_state state(rs, std::move(kernel));

        ptg::index_subset<4> closure(dims);
        closure.add(expr_idx, expr_r0, 0, c_idx);  // Expr -> . a Expr / c
        closure.add(expr_idx, expr_r1, 0, c_idx);  // Expr -> . b / c
        closure.add(expr_idx, expr_r2, 0, c_idx);  // Expr -> . / c

        state.add_items(closure);

        // Items where 'a' is after dot: Expr -> . a Expr / c
        auto a_ref = ptg::symbol_ref{ptg::symbol_type::terminal, a_idx};
        const auto& a_items = state.get_symbol_items(a_ref);
        REQUIRE(a_items.size() == 1);
        REQUIRE(a_items[0] == std::array<size_t, 4>{expr_idx, expr_r0, 0, c_idx});

        // Items where 'b' is after dot: Expr -> . b / c
        auto b_ref = ptg::symbol_ref{ptg::symbol_type::terminal, b_idx};
        const auto& b_items = state.get_symbol_items(b_ref);
        REQUIRE(b_items.size() == 1);
        REQUIRE(b_items[0] == std::array<size_t, 4>{expr_idx, expr_r1, 0, c_idx});
    }

    SECTION("get_symbol_items for non-terminal")
    {
        ptg::index_subset<4> kernel(dims);
        kernel.add(root_idx, 0, 0, eof_idx);  // $root -> . S / $eof

        ptg::lr1_state state(rs, std::move(kernel));

        ptg::index_subset<4> closure(dims);
        closure.add(s_idx, s_r0, 0, eof_idx);  // S -> . Expr / $eof

        state.add_items(closure);

        // Items where 'S' is after dot: $root -> . S / $eof
        auto s_ref = ptg::symbol_ref{ptg::symbol_type::non_terminal, s_idx};
        const auto& s_items = state.get_symbol_items(s_ref);
        REQUIRE(s_items.size() == 1);
        REQUIRE(s_items[0] == std::array<size_t, 4>{root_idx, 0, 0, eof_idx});

        // Items where 'Expr' is after dot: S -> . Expr / $eof
        auto expr_ref = ptg::symbol_ref{ptg::symbol_type::non_terminal, expr_idx};
        const auto& expr_items = state.get_symbol_items(expr_ref);
        REQUIRE(expr_items.size() == 1);
        REQUIRE(expr_items[0] == std::array<size_t, 4>{s_idx, s_r0, 0, eof_idx});
    }

    SECTION("end items categorization")
    {
        ptg::index_subset<4> kernel(dims);
        kernel.add(expr_idx, expr_r2, 0, c_idx);  // Expr -> . / c (dot at end for empty production)

        ptg::lr1_state state(rs, std::move(kernel));

        const auto& c_term_end_items = state.get_end_items(c_idx);
        REQUIRE(c_term_end_items.size() == 1);
        REQUIRE(c_term_end_items[0] == std::array<size_t, 4>{expr_idx, expr_r2, 0, c_idx});

        // Add another end item
        ptg::index_subset<4> additional(dims);
        additional.add(expr_idx, expr_r1, 1, eof_idx);  // Expr -> b . / $eof

        state.add_items(additional);

        const auto& eof_term_end_items = state.get_end_items(eof_idx);
        REQUIRE(eof_term_end_items.size() == 1);
        REQUIRE(eof_term_end_items[0] == std::array<size_t, 4>{expr_idx, expr_r1, 1, eof_idx});
    }

    SECTION("mixed items addition")
    {
        ptg::index_subset<4> kernel(dims);
        kernel.add(s_idx, s_r0, 0, eof_idx);  // S -> . Expr / $eof

        ptg::lr1_state state(rs, std::move(kernel));

        ptg::index_subset<4> closure(dims);
        closure.add(expr_idx, expr_r0, 0, eof_idx);  // Expr -> . a Expr / $eof
        closure.add(expr_idx, expr_r1, 0, eof_idx);  // Expr -> . b / $eof
        closure.add(expr_idx, expr_r2, 0, eof_idx);  // Expr -> . / $eof

        state.add_items(closure);

        // Check non-terminal items: none in this closure, kernel has Expr
        auto expr_ref = ptg::symbol_ref{ptg::symbol_type::non_terminal, expr_idx};
        const auto& expr_items = state.get_symbol_items(expr_ref);
        REQUIRE(expr_items.size() == 1);
        REQUIRE(expr_items[0] == std::array<size_t, 4>{s_idx, s_r0, 0, eof_idx});

        // Check terminal 'a'
        auto a_ref = ptg::symbol_ref{ptg::symbol_type::terminal, a_idx};
        const auto& a_items = state.get_symbol_items(a_ref);
        REQUIRE(a_items.size() == 1);
        REQUIRE(a_items[0] == std::array<size_t, 4>{expr_idx, expr_r0, 0, eof_idx});

        // Check terminal 'b'
        auto b_ref = ptg::symbol_ref{ptg::symbol_type::terminal, b_idx};
        const auto& b_items = state.get_symbol_items(b_ref);
        REQUIRE(b_items.size() == 1);
        REQUIRE(b_items[0] == std::array<size_t, 4>{expr_idx, expr_r1, 0, eof_idx});

        const auto& eof_term_end_items = state.get_end_items(eof_idx);
        REQUIRE(eof_term_end_items.size() == 1);
        REQUIRE(eof_term_end_items[0] == std::array<size_t, 4>{expr_idx, expr_r2, 0, eof_idx});
    }

    SECTION("out of range errors")
    {
        ptg::index_subset<4> kernel(dims);
        kernel.add(root_idx, 0, 0, eof_idx);

        ptg::lr1_state state(rs, std::move(kernel));

        // Invalid symbol ref
        ptg::symbol_ref invalid_term{ptg::symbol_type::terminal, rs.get_term_count()};
        REQUIRE_THROWS_AS(state.get_symbol_items(invalid_term), std::out_of_range);

        ptg::symbol_ref invalid_nterm{ptg::symbol_type::non_terminal, rs.get_nterm_count()};
        REQUIRE_THROWS_AS(state.get_symbol_items(invalid_nterm), std::out_of_range);
        
        size_t invalid_term_idx{rs.get_term_count()};
        REQUIRE_THROWS_AS(state.get_end_items(invalid_term_idx), std::out_of_range);
    }
}
