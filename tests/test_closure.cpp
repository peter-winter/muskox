#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <closure.h>
#include <firsts.h>
#include <grammar_error.h>
#include <nullable.h>
#include <ruleset.h>
#include <symbol_collection.h>

using Catch::Matchers::Message;

using namespace muskox;

TEST_CASE("closure calculate", "[closure]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_idx = sc.add_term("b");
    [[maybe_unused]] size_t c_idx = sc.add_term("c");
    [[maybe_unused]] size_t eof_idx = 0;  // $eof

    [[maybe_unused]] size_t root_idx = 0;  // $root
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t expr_idx = sc.add_nterm("Expr");

    ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"Expr"});
    [[maybe_unused]] size_t expr_r0 = rs.add_rule("Expr", {"a", "Expr"});
    [[maybe_unused]] size_t expr_r1 = rs.add_rule("Expr", {"b"});
    [[maybe_unused]] size_t expr_r2 = rs.add_rule("Expr", {});

    closure cl(rs);

    SECTION("root item")
    {
        lr1_set_item item(root_idx, 0, 0, eof_idx);  // $root -> . S / $eof
        const auto& res = cl.calculate(item);
        REQUIRE(res.has_value());
        REQUIRE(res.value().get_count() == 2);
        REQUIRE(res.value().contains(root_idx, 0, 0, eof_idx));
        REQUIRE(res.value().contains(s_idx, s_r0, 0, eof_idx));
    }

    SECTION("item with term after dot")
    {
        lr1_set_item item(expr_idx, expr_r0, 0, c_idx);  // Expr -> . a Expr / c
        const auto& res = cl.calculate(item);
        REQUIRE(res.has_value());
        REQUIRE(res.value().get_count() == 1);
        REQUIRE(res.value().contains(expr_idx, expr_r0, 0, c_idx));
    }

    SECTION("item with nterm after dot")
    {
        lr1_set_item item(s_idx, s_r0, 0, eof_idx);  // S -> . Expr / $eof
        const auto& res = cl.calculate(item);
        REQUIRE(res.has_value());
        REQUIRE(res.value().get_count() == 4);
        REQUIRE(res.value().contains(s_idx, s_r0, 0, eof_idx));
        REQUIRE(res.value().contains(expr_idx, expr_r0, 0, eof_idx));
        REQUIRE(res.value().contains(expr_idx, expr_r1, 0, eof_idx));
        REQUIRE(res.value().contains(expr_idx, expr_r2, 0, eof_idx));
    }

    SECTION("item in middle")
    {
        lr1_set_item item(expr_idx, expr_r0, 1, c_idx);  // Expr -> a . Expr / c
        const auto& res = cl.calculate(item);
        REQUIRE(res.has_value());
        REQUIRE(res.value().get_count() == 4);
        REQUIRE(res.value().contains(expr_idx, expr_r0, 1, c_idx));
        REQUIRE(res.value().contains(expr_idx, expr_r0, 0, c_idx));
        REQUIRE(res.value().contains(expr_idx, expr_r1, 0, c_idx));
        REQUIRE(res.value().contains(expr_idx, expr_r2, 0, c_idx));
    }

    SECTION("item at end")
    {
        lr1_set_item item(expr_idx, expr_r1, 1, c_idx);  // Expr -> b . / c
        const auto& res = cl.calculate(item);
        REQUIRE(res.has_value());
        REQUIRE(res.value().get_count() == 1);
        REQUIRE(res.value().contains(expr_idx, expr_r1, 1, c_idx));
    }

    SECTION("empty production at start")
    {
        lr1_set_item item(expr_idx, expr_r2, 0, c_idx);  // Expr -> . / c
        const auto& res = cl.calculate(item);
        REQUIRE(res.has_value());
        REQUIRE(res.value().get_count() == 1);
        REQUIRE(res.value().contains(expr_idx, expr_r2, 0, c_idx));
    }

    SECTION("non-empty beta with multiple firsts")
    {
        symbol_collection multi_sc;
        [[maybe_unused]] size_t multi_a_idx = multi_sc.add_term("a");
        [[maybe_unused]] size_t multi_d_idx = multi_sc.add_term("d");
        [[maybe_unused]] size_t multi_e_idx = multi_sc.add_term("e");
        [[maybe_unused]] size_t multi_eof_idx = 0;

        [[maybe_unused]] size_t multi_root_idx = 0;
        [[maybe_unused]] size_t multi_s_idx = multi_sc.add_nterm("S");
        [[maybe_unused]] size_t multi_a_nterm_idx = multi_sc.add_nterm("A");
        [[maybe_unused]] size_t multi_b_idx = multi_sc.add_nterm("B");
        [[maybe_unused]] size_t multi_c_idx = multi_sc.add_nterm("C");

        ruleset multi_rs(multi_sc);
        [[maybe_unused]] size_t multi_s_r0 = multi_rs.add_rule("S", {"A", "C"});
        [[maybe_unused]] size_t multi_a_r0 = multi_rs.add_rule("A", {"B"});
        [[maybe_unused]] size_t multi_b_r0 = multi_rs.add_rule("B", {"a"});
        [[maybe_unused]] size_t multi_c_r0 = multi_rs.add_rule("C", {"d"});
        [[maybe_unused]] size_t multi_c_r1 = multi_rs.add_rule("C", {"e"});

        closure multi_cl(multi_rs);

        lr1_set_item multi_item(multi_s_idx, multi_s_r0, 0, multi_eof_idx);  // S -> . A C / $eof
        const auto& multi_res = multi_cl.calculate(multi_item);
        REQUIRE(multi_res.has_value());
        REQUIRE(multi_res.value().get_count() == 3);
        REQUIRE(multi_res.value().contains(multi_s_idx, multi_s_r0, 0, multi_eof_idx));
        REQUIRE(multi_res.value().contains(multi_a_nterm_idx, multi_a_r0, 0, multi_d_idx));
        REQUIRE(multi_res.value().contains(multi_a_nterm_idx, multi_a_r0, 0, multi_e_idx));
    }

    SECTION("non-empty beta with multiple firsts and nullable")
    {
        symbol_collection multi_sc;
        [[maybe_unused]] size_t multi_a_idx = multi_sc.add_term("a");
        [[maybe_unused]] size_t multi_d_idx = multi_sc.add_term("d");
        [[maybe_unused]] size_t multi_e_idx = multi_sc.add_term("e");
        [[maybe_unused]] size_t multi_eof_idx = 0;

        [[maybe_unused]] size_t multi_root_idx = 0;
        [[maybe_unused]] size_t multi_s_idx = multi_sc.add_nterm("S");
        [[maybe_unused]] size_t multi_a_nterm_idx = multi_sc.add_nterm("A");
        [[maybe_unused]] size_t multi_b_idx = multi_sc.add_nterm("B");
        [[maybe_unused]] size_t multi_c_idx = multi_sc.add_nterm("C");

        ruleset multi_rs(multi_sc);
        [[maybe_unused]] size_t multi_s_r0 = multi_rs.add_rule("S", {"A", "C"});
        [[maybe_unused]] size_t multi_a_r0 = multi_rs.add_rule("A", {"B"});
        [[maybe_unused]] size_t multi_b_r0 = multi_rs.add_rule("B", {"a"});
        [[maybe_unused]] size_t multi_c_r0 = multi_rs.add_rule("C", {"d"});
        [[maybe_unused]] size_t multi_c_r1 = multi_rs.add_rule("C", {"e"});
        [[maybe_unused]] size_t multi_c_r2 = multi_rs.add_rule("C", {});

        closure multi_cl(multi_rs);

        lr1_set_item multi_item(multi_s_idx, multi_s_r0, 0, multi_eof_idx);  // S -> . A C / $eof
        const auto& multi_res = multi_cl.calculate(multi_item);
        REQUIRE(multi_res.has_value());
        REQUIRE(multi_res.value().get_count() == 4);
        REQUIRE(multi_res.value().contains(multi_s_idx, multi_s_r0, 0, multi_eof_idx));
        REQUIRE(multi_res.value().contains(multi_a_nterm_idx, multi_a_r0, 0, multi_d_idx));
        REQUIRE(multi_res.value().contains(multi_a_nterm_idx, multi_a_r0, 0, multi_e_idx));
        REQUIRE(multi_res.value().contains(multi_a_nterm_idx, multi_a_r0, 0, multi_eof_idx));
    }
}

TEST_CASE("closure calculate_full", "[closure]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_idx = sc.add_term("b");
    [[maybe_unused]] size_t c_idx = sc.add_term("c");
    [[maybe_unused]] size_t eof_idx = 0;  // $eof

    [[maybe_unused]] size_t root_idx = 0;  // $root
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t expr_idx = sc.add_nterm("Expr");

    ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"Expr"});
    [[maybe_unused]] size_t expr_r0 = rs.add_rule("Expr", {"a", "Expr"});
    [[maybe_unused]] size_t expr_r1 = rs.add_rule("Expr", {"b"});
    [[maybe_unused]] size_t expr_r2 = rs.add_rule("Expr", {});

    closure cl(rs);

    SECTION("root item")
    {
        lr1_set_item item(root_idx, 0, 0, eof_idx);  // $root -> . S / $eof
        const auto& res = cl.calculate_full(item);
        REQUIRE(res.has_value());
        REQUIRE(res.value().get_count() == 5);
        REQUIRE(res.value().contains(root_idx, 0, 0, eof_idx));
        REQUIRE(res.value().contains(s_idx, s_r0, 0, eof_idx));
        REQUIRE(res.value().contains(expr_idx, expr_r0, 0, eof_idx));
        REQUIRE(res.value().contains(expr_idx, expr_r1, 0, eof_idx));
        REQUIRE(res.value().contains(expr_idx, expr_r2, 0, eof_idx));
    }

    SECTION("item with term after dot")
    {
        lr1_set_item item(expr_idx, expr_r0, 0, c_idx);  // Expr -> . a Expr / c
        const auto& res = cl.calculate_full(item);
        REQUIRE(res.has_value());
        REQUIRE(res.value().get_count() == 1);
        REQUIRE(res.value().contains(expr_idx, expr_r0, 0, c_idx));
    }

    SECTION("item with nterm after dot")
    {
        lr1_set_item item(s_idx, s_r0, 0, eof_idx);  // S -> . Expr / $eof
        const auto& res = cl.calculate_full(item);
        REQUIRE(res.has_value());
        REQUIRE(res.value().get_count() == 4);
        REQUIRE(res.value().contains(s_idx, s_r0, 0, eof_idx));
        REQUIRE(res.value().contains(expr_idx, expr_r0, 0, eof_idx));
        REQUIRE(res.value().contains(expr_idx, expr_r1, 0, eof_idx));
        REQUIRE(res.value().contains(expr_idx, expr_r2, 0, eof_idx));
    }

    SECTION("item in middle")
    {
        lr1_set_item item(expr_idx, expr_r0, 1, c_idx);  // Expr -> a . Expr / c
        const auto& res = cl.calculate_full(item);
        REQUIRE(res.has_value());
        REQUIRE(res.value().get_count() == 4);
        REQUIRE(res.value().contains(expr_idx, expr_r0, 1, c_idx));
        REQUIRE(res.value().contains(expr_idx, expr_r0, 0, c_idx));
        REQUIRE(res.value().contains(expr_idx, expr_r1, 0, c_idx));
        REQUIRE(res.value().contains(expr_idx, expr_r2, 0, c_idx));
    }

    SECTION("item at end")
    {
        lr1_set_item item(expr_idx, expr_r1, 1, c_idx);  // Expr -> b . / c
        const auto& res = cl.calculate_full(item);
        REQUIRE(res.has_value());
        REQUIRE(res.value().get_count() == 1);
        REQUIRE(res.value().contains(expr_idx, expr_r1, 1, c_idx));
    }

    SECTION("empty production at start")
    {
        lr1_set_item item(expr_idx, expr_r2, 0, c_idx);  // Expr -> . / c
        const auto& res = cl.calculate_full(item);
        REQUIRE(res.has_value());
        REQUIRE(res.value().get_count() == 1);
        REQUIRE(res.value().contains(expr_idx, expr_r2, 0, c_idx));
    }

    SECTION("chained nonterminals")
    {
        symbol_collection chain_sc;
        [[maybe_unused]] size_t chain_c_idx = chain_sc.add_term("c");
        [[maybe_unused]] size_t chain_eof_idx = 0;
        [[maybe_unused]] size_t chain_root_idx = 0;
        [[maybe_unused]] size_t chain_s_idx = chain_sc.add_nterm("S");
        [[maybe_unused]] size_t chain_a_idx = chain_sc.add_nterm("A");
        [[maybe_unused]] size_t chain_b_idx = chain_sc.add_nterm("B");

        ruleset chain_rs(chain_sc);
        [[maybe_unused]] size_t chain_s_r0 = chain_rs.add_rule("S", {"A"});
        [[maybe_unused]] size_t chain_a_r0 = chain_rs.add_rule("A", {"B"});
        [[maybe_unused]] size_t chain_b_r0 = chain_rs.add_rule("B", {"c"});

        closure chain_cl(chain_rs);

        lr1_set_item chain_item(chain_root_idx, 0, 0, chain_eof_idx);  // $root -> . S / $eof
        const auto& chain_res = chain_cl.calculate_full(chain_item);
        REQUIRE(chain_res.has_value());
        REQUIRE(chain_res.value().get_count() == 4);
        REQUIRE(chain_res.value().contains(chain_root_idx, 0, 0, chain_eof_idx));
        REQUIRE(chain_res.value().contains(chain_s_idx, chain_s_r0, 0, chain_eof_idx));
        REQUIRE(chain_res.value().contains(chain_a_idx, chain_a_r0, 0, chain_eof_idx));
        REQUIRE(chain_res.value().contains(chain_b_idx, chain_b_r0, 0, chain_eof_idx));
    }

    SECTION("chained nonterminals with nullable")
    {
        symbol_collection chain_sc;
        [[maybe_unused]] size_t chain_c_idx = chain_sc.add_term("c");
        [[maybe_unused]] size_t chain_eof_idx = 0;
        [[maybe_unused]] size_t chain_root_idx = 0;
        [[maybe_unused]] size_t chain_s_idx = chain_sc.add_nterm("S");
        [[maybe_unused]] size_t chain_a_idx = chain_sc.add_nterm("A");
        [[maybe_unused]] size_t chain_b_idx = chain_sc.add_nterm("B");

        ruleset chain_rs(chain_sc);
        [[maybe_unused]] size_t chain_s_r0 = chain_rs.add_rule("S", {"A"});
        [[maybe_unused]] size_t chain_a_r0 = chain_rs.add_rule("A", {"B"});
        [[maybe_unused]] size_t chain_b_r0 = chain_rs.add_rule("B", {"c"});
        [[maybe_unused]] size_t chain_b_r1 = chain_rs.add_rule("B", {});

        closure chain_cl(chain_rs);

        lr1_set_item chain_item(chain_root_idx, 0, 0, chain_eof_idx);  // $root -> . S / $eof
        const auto& chain_res = chain_cl.calculate_full(chain_item);
        REQUIRE(chain_res.has_value());
        REQUIRE(chain_res.value().get_count() == 5);
        REQUIRE(chain_res.value().contains(chain_root_idx, 0, 0, chain_eof_idx));
        REQUIRE(chain_res.value().contains(chain_s_idx, chain_s_r0, 0, chain_eof_idx));
        REQUIRE(chain_res.value().contains(chain_a_idx, chain_a_r0, 0, chain_eof_idx));
        REQUIRE(chain_res.value().contains(chain_b_idx, chain_b_r0, 0, chain_eof_idx));
        REQUIRE(chain_res.value().contains(chain_b_idx, chain_b_r1, 0, chain_eof_idx));
    }

    SECTION("chained nonterminals with nullable suffix")
    {
        symbol_collection chain_sc;
        [[maybe_unused]] size_t chain_d_idx = chain_sc.add_term("d");
        [[maybe_unused]] size_t chain_eof_idx = 0;
        [[maybe_unused]] size_t chain_root_idx = 0;
        [[maybe_unused]] size_t chain_s_idx = chain_sc.add_nterm("S");
        [[maybe_unused]] size_t chain_a_idx = chain_sc.add_nterm("A");
        [[maybe_unused]] size_t chain_b_idx = chain_sc.add_nterm("B");
        [[maybe_unused]] size_t chain_e_idx = chain_sc.add_nterm("E");

        ruleset chain_rs(chain_sc);
        [[maybe_unused]] size_t chain_s_r0 = chain_rs.add_rule("S", {"A", "E"});
        [[maybe_unused]] size_t chain_a_r0 = chain_rs.add_rule("A", {"B"});
        [[maybe_unused]] size_t chain_b_r0 = chain_rs.add_rule("B", {"d"});
        [[maybe_unused]] size_t chain_e_r0 = chain_rs.add_rule("E", {});

        closure chain_cl(chain_rs);

        lr1_set_item chain_item(chain_root_idx, 0, 0, chain_eof_idx);  // $root -> . S / $eof
        const auto& chain_res = chain_cl.calculate_full(chain_item);
        REQUIRE(chain_res.has_value());
        REQUIRE(chain_res.value().get_count() == 4);
        REQUIRE(chain_res.value().contains(chain_root_idx, 0, 0, chain_eof_idx));
        REQUIRE(chain_res.value().contains(chain_s_idx, chain_s_r0, 0, chain_eof_idx));
        REQUIRE(chain_res.value().contains(chain_a_idx, chain_a_r0, 0, chain_eof_idx));
        REQUIRE(chain_res.value().contains(chain_b_idx, chain_b_r0, 0, chain_eof_idx));
    }

    SECTION("non-empty beta with multiple firsts")
    {
        symbol_collection multi_sc;
        [[maybe_unused]] size_t multi_a_idx = multi_sc.add_term("a");
        [[maybe_unused]] size_t multi_d_idx = multi_sc.add_term("d");
        [[maybe_unused]] size_t multi_e_idx = multi_sc.add_term("e");
        [[maybe_unused]] size_t multi_eof_idx = 0;

        [[maybe_unused]] size_t multi_root_idx = 0;
        [[maybe_unused]] size_t multi_s_idx = multi_sc.add_nterm("S");
        [[maybe_unused]] size_t multi_a_nterm_idx = multi_sc.add_nterm("A");
        [[maybe_unused]] size_t multi_b_idx = multi_sc.add_nterm("B");
        [[maybe_unused]] size_t multi_c_idx = multi_sc.add_nterm("C");

        ruleset multi_rs(multi_sc);
        [[maybe_unused]] size_t multi_s_r0 = multi_rs.add_rule("S", {"A", "C"});
        [[maybe_unused]] size_t multi_a_r0 = multi_rs.add_rule("A", {"B"});
        [[maybe_unused]] size_t multi_b_r0 = multi_rs.add_rule("B", {"a"});
        [[maybe_unused]] size_t multi_c_r0 = multi_rs.add_rule("C", {"d"});
        [[maybe_unused]] size_t multi_c_r1 = multi_rs.add_rule("C", {"e"});

        closure multi_cl(multi_rs);

        lr1_set_item multi_item(multi_s_idx, multi_s_r0, 0, multi_eof_idx);  // S -> . A C / $eof
        const auto& multi_res = multi_cl.calculate_full(multi_item);
        REQUIRE(multi_res.has_value());
        REQUIRE(multi_res.value().get_count() == 5);
        REQUIRE(multi_res.value().contains(multi_s_idx, multi_s_r0, 0, multi_eof_idx));
        REQUIRE(multi_res.value().contains(multi_a_nterm_idx, multi_a_r0, 0, multi_d_idx));
        REQUIRE(multi_res.value().contains(multi_a_nterm_idx, multi_a_r0, 0, multi_e_idx));
        REQUIRE(multi_res.value().contains(multi_b_idx, multi_b_r0, 0, multi_d_idx));
        REQUIRE(multi_res.value().contains(multi_b_idx, multi_b_r0, 0, multi_e_idx));
    }

    SECTION("non-empty beta with multiple firsts and nullable")
    {
        symbol_collection multi_sc;
        [[maybe_unused]] size_t multi_a_idx = multi_sc.add_term("a");
        [[maybe_unused]] size_t multi_d_idx = multi_sc.add_term("d");
        [[maybe_unused]] size_t multi_e_idx = multi_sc.add_term("e");
        [[maybe_unused]] size_t multi_eof_idx = 0;

        [[maybe_unused]] size_t multi_root_idx = 0;
        [[maybe_unused]] size_t multi_s_idx = multi_sc.add_nterm("S");
        [[maybe_unused]] size_t multi_a_nterm_idx = multi_sc.add_nterm("A");
        [[maybe_unused]] size_t multi_b_idx = multi_sc.add_nterm("B");
        [[maybe_unused]] size_t multi_c_idx = multi_sc.add_nterm("C");

        ruleset multi_rs(multi_sc);
        [[maybe_unused]] size_t multi_s_r0 = multi_rs.add_rule("S", {"A", "C"});
        [[maybe_unused]] size_t multi_a_r0 = multi_rs.add_rule("A", {"B"});
        [[maybe_unused]] size_t multi_b_r0 = multi_rs.add_rule("B", {"a"});
        [[maybe_unused]] size_t multi_c_r0 = multi_rs.add_rule("C", {"d"});
        [[maybe_unused]] size_t multi_c_r1 = multi_rs.add_rule("C", {"e"});
        [[maybe_unused]] size_t multi_c_r2 = multi_rs.add_rule("C", {});

        closure multi_cl(multi_rs);

        lr1_set_item multi_item(multi_s_idx, multi_s_r0, 0, multi_eof_idx);  // S -> . A C / $eof
        const auto& multi_res = multi_cl.calculate_full(multi_item);
        REQUIRE(multi_res.has_value());
        REQUIRE(multi_res.value().get_count() == 7);
        REQUIRE(multi_res.value().contains(multi_s_idx, multi_s_r0, 0, multi_eof_idx));
        REQUIRE(multi_res.value().contains(multi_a_nterm_idx, multi_a_r0, 0, multi_d_idx));
        REQUIRE(multi_res.value().contains(multi_a_nterm_idx, multi_a_r0, 0, multi_e_idx));
        REQUIRE(multi_res.value().contains(multi_a_nterm_idx, multi_a_r0, 0, multi_eof_idx));
        REQUIRE(multi_res.value().contains(multi_b_idx, multi_b_r0, 0, multi_d_idx));
        REQUIRE(multi_res.value().contains(multi_b_idx, multi_b_r0, 0, multi_e_idx));
        REQUIRE(multi_res.value().contains(multi_b_idx, multi_b_r0, 0, multi_eof_idx));
    }
}
