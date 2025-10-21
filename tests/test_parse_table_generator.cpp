#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <grammar_error.h>
#include <parse_table_generator.h>
#include <ruleset.h>
#include <symbol_collection.h>

#include <iostream>

using Catch::Matchers::Message;

using ptg::index_subset;

TEST_CASE("parse_table_generator validate", "[parse_table_generator]")
{
    ptg::symbol_collection sc;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_idx = sc.add_nterm("B");

    SECTION("valid grammar")
    {
        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a"});
        [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"a"});

        REQUIRE_NOTHROW(ptg::parse_table_generator(rs));
    }

    SECTION("nterm no rules")
    {
        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a"});

        REQUIRE_THROWS_MATCHES(
            ptg::parse_table_generator(rs),
            ptg::grammar_error,
            Message("Nonterminal 'B' has no productions.")
        );
    }
}

TEST_CASE("parse_table_generator warnings", "[parse_table_generator]")
{
    ptg::symbol_collection sc;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
    [[maybe_unused]] size_t c_idx = sc.add_term("c");
    [[maybe_unused]] size_t u_idx = sc.add_nterm("U");
    [[maybe_unused]] size_t v_idx = sc.add_term("v");

    ptg::ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a", "B"});
    [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"c"});
    [[maybe_unused]] size_t u_r0 = rs.add_rule("U", {"v"});

    ptg::parse_table_generator lss(rs);

    const auto& warnings = lss.get_warnings();
    REQUIRE(warnings.size() == 2);
    REQUIRE(warnings[0] == "Nonterminal 'U' is unused.");
    REQUIRE(warnings[1] == "Terminal 'v' is unused.");
}

TEST_CASE("parse_table_generator states simple", "[parse_table_generator]")
{
    ptg::symbol_collection sc;
    [[maybe_unused]] size_t root_idx = 0;
    [[maybe_unused]] size_t eof_idx = 0;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t c_idx = sc.add_term("c");

    ptg::ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a", "B"});
    [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"c"});

    ptg::parse_table_generator ptg(rs);

    const auto& states = ptg.get_states();
    REQUIRE(states.size() == 5);

    auto dims = rs.get_lr1_set_item_space_dims();
    ptg::index_subset_builder<4> builder(dims);

    // State 0: kernel {$root -> . S / $eof}, items: that + {S -> . a B / $eof}
    auto exp_kernel0 = builder.reset()(root_idx, 0, 0, eof_idx).build();
    REQUIRE(states[0].kernel_matches(exp_kernel0));

    auto exp_items0 = builder.reset()
        (root_idx, 0, 0, eof_idx)
        (s_idx, s_r0, 0, eof_idx).build();
    REQUIRE(states[0].matches(exp_items0));

    // State 1: kernel {S -> a . B / $eof}, items: that + {B -> . c / $eof}
    auto exp_kernel1 = builder.reset()(s_idx, s_r0, 1, eof_idx).build();
    REQUIRE(states[1].kernel_matches(exp_kernel1));

    auto exp_items1 = builder.reset()
        (s_idx, s_r0, 1, eof_idx)
        (b_idx, b_r0, 0, eof_idx).build();
    REQUIRE(states[1].matches(exp_items1));

    // State 2: kernel {$root -> S . / $eof}
    auto exp_kernel2 = builder.reset()(root_idx, 0, 1, eof_idx).build();
    REQUIRE(states[2].kernel_matches(exp_kernel2));
    REQUIRE(states[2].matches(exp_kernel2));

    // State 3: kernel {B -> c . / $eof}
    auto exp_kernel3 = builder.reset()(b_idx, b_r0, 1, eof_idx).build();
    REQUIRE(states[3].kernel_matches(exp_kernel3));
    REQUIRE(states[3].matches(exp_kernel3));

    // State 4: kernel {S -> a B . / $eof}
    auto exp_kernel4 = builder.reset()(s_idx, s_r0, 2, eof_idx).build();
    REQUIRE(states[4].kernel_matches(exp_kernel4));
    REQUIRE(states[4].matches(exp_kernel4));
}

TEST_CASE("parse_table_generator rr conflict", "[parse_table_generator]")
{
    ptg::symbol_collection sc;
    [[maybe_unused]] size_t root_idx = 0;
    [[maybe_unused]] size_t eof_idx = 0;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_nterm_idx = sc.add_nterm("A");
    [[maybe_unused]] size_t b_nterm_idx = sc.add_nterm("B");
    [[maybe_unused]] size_t a_term_idx = sc.add_term("a");

    ptg::ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A"});
    [[maybe_unused]] size_t s_r1 = rs.add_rule("S", {"B"});
    [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"a"});
    [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"a"});

    ptg::parse_table_generator ptg(rs);

    SECTION("warnings")
    {
        const auto& warnings = ptg.get_warnings();
        REQUIRE(warnings.size() == 1);
        REQUIRE(warnings[0] == "Reduce-reduce conflict in state 1 on lookahead '$eof' for productions:\n    A -> a . / $eof\n    B -> a . / $eof");
    }

    SECTION("states")
    {
        const auto& states = ptg.get_states();
        REQUIRE(states.size() == 5);

        auto dims = rs.get_lr1_set_item_space_dims();
        ptg::index_subset_builder<4> builder(dims);

        // State 0: kernel {$root -> . S / $eof}, items: that + {S -> . A / $eof, S -> . B / $eof, A -> . a / $eof, B -> . a / $eof}
        auto exp_kernel0 = builder.reset()(root_idx, 0, 0, eof_idx).build();
        REQUIRE(states[0].kernel_matches(exp_kernel0));

        auto exp_items0 = builder.reset()
            (root_idx, 0, 0, eof_idx)
            (s_idx, s_r0, 0, eof_idx)
            (s_idx, s_r1, 0, eof_idx)
            (a_nterm_idx, a_r0, 0, eof_idx)
            (b_nterm_idx, b_r0, 0, eof_idx).build();
        REQUIRE(states[0].matches(exp_items0));

        // State 1: kernel {A -> a . / $eof, B -> a . / $eof}
        auto exp_kernel1 = builder.reset()
            (a_nterm_idx, a_r0, 1, eof_idx)
            (b_nterm_idx, b_r0, 1, eof_idx).build();
        REQUIRE(states[1].kernel_matches(exp_kernel1));
        REQUIRE(states[1].matches(exp_kernel1));

        // State 2: kernel {$root -> S . / $eof}
        auto exp_kernel2 = builder.reset()(root_idx, 0, 1, eof_idx).build();
        REQUIRE(states[2].kernel_matches(exp_kernel2));
        REQUIRE(states[2].matches(exp_kernel2));

        // State 3: kernel {S -> A . / $eof}
        auto exp_kernel3 = builder.reset()(s_idx, s_r0, 1, eof_idx).build();
        REQUIRE(states[3].kernel_matches(exp_kernel3));
        REQUIRE(states[3].matches(exp_kernel3));

        // State 4: kernel {S -> B . / $eof}
        auto exp_kernel4 = builder.reset()(s_idx, s_r1, 1, eof_idx).build();
        REQUIRE(states[4].kernel_matches(exp_kernel4));
        REQUIRE(states[4].matches(exp_kernel4));
    }
}

TEST_CASE("parse_table_generator states complex lookaheads", "[parse_table_generator]")
{
    ptg::symbol_collection sc;
    [[maybe_unused]] size_t root_idx = 0;
    [[maybe_unused]] size_t eof_idx = 0;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_nterm_idx = sc.add_nterm("A");
    [[maybe_unused]] size_t c_nterm_idx = sc.add_nterm("C");
    [[maybe_unused]] size_t a_term_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_term_idx = sc.add_term("b");
    [[maybe_unused]] size_t c_term_idx = sc.add_term("c");
    [[maybe_unused]] size_t d_term_idx = sc.add_term("d");
    [[maybe_unused]] size_t e_term_idx = sc.add_term("e");

    ptg::ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a", "A", "b"});
    [[maybe_unused]] size_t s_r1 = rs.add_rule("S", {"c", "C", "d"});
    [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"e"});
    [[maybe_unused]] size_t c_r0 = rs.add_rule("C", {"e"});

    ptg::parse_table_generator ptg(rs);

    const auto& states = ptg.get_states();
    REQUIRE(states.size() == 10);

    auto dims = rs.get_lr1_set_item_space_dims();
    ptg::index_subset_builder<4> builder(dims);

    // State 0: kernel {$root -> . S / $eof}, items: that + {S -> . a A b / $eof, S -> . c C d / $eof}
    auto exp_kernel0 = builder.reset()(root_idx, 0, 0, eof_idx).build();
    REQUIRE(states[0].kernel_matches(exp_kernel0));

    auto exp_items0 = builder.reset()
        (root_idx, 0, 0, eof_idx)
        (s_idx, s_r0, 0, eof_idx)
        (s_idx, s_r1, 0, eof_idx).build();
    REQUIRE(states[0].matches(exp_items0));

    // State 1: kernel {S -> a . A b / $eof}, items: that + {A -> . e / b}
    auto exp_kernel1 = builder.reset()(s_idx, s_r0, 1, eof_idx).build();
    REQUIRE(states[1].kernel_matches(exp_kernel1));

    auto exp_items1 = builder.reset()
        (s_idx, s_r0, 1, eof_idx)
        (a_nterm_idx, a_r0, 0, b_term_idx).build();
    REQUIRE(states[1].matches(exp_items1));

    // State 2: kernel {S -> c . C d / $eof}, items: that + {C -> . e / d}
    auto exp_kernel2 = builder.reset()(s_idx, s_r1, 1, eof_idx).build();
    REQUIRE(states[2].kernel_matches(exp_kernel2));

    auto exp_items2 = builder.reset()
        (s_idx, s_r1, 1, eof_idx)
        (c_nterm_idx, c_r0, 0, d_term_idx).build();
    REQUIRE(states[2].matches(exp_items2));

    // State 3: kernel {$root -> S . / $eof}
    auto exp_kernel3 = builder.reset()(root_idx, 0, 1, eof_idx).build();
    REQUIRE(states[3].kernel_matches(exp_kernel3));
    REQUIRE(states[3].matches(exp_kernel3));

    // State 4: kernel {A -> e . / b}
    auto exp_kernel4 = builder.reset()(a_nterm_idx, a_r0, 1, b_term_idx).build();
    REQUIRE(states[4].kernel_matches(exp_kernel4));
    REQUIRE(states[4].matches(exp_kernel4));

    // State 5: kernel {S -> a A . b / $eof}
    auto exp_kernel5 = builder.reset()(s_idx, s_r0, 2, eof_idx).build();
    REQUIRE(states[5].kernel_matches(exp_kernel5));
    REQUIRE(states[5].matches(exp_kernel5));

    // State 6: kernel {C -> e . / d}
    auto exp_kernel6 = builder.reset()(c_nterm_idx, c_r0, 1, d_term_idx).build();
    REQUIRE(states[6].kernel_matches(exp_kernel6));
    REQUIRE(states[6].matches(exp_kernel6));

    // State 7: kernel {S -> c C . d / $eof}
    auto exp_kernel7 = builder.reset()(s_idx, s_r1, 2, eof_idx).build();
    REQUIRE(states[7].kernel_matches(exp_kernel7));
    REQUIRE(states[7].matches(exp_kernel7));

    // State 8: kernel {S -> a A b . / $eof}
    auto exp_kernel8 = builder.reset()(s_idx, s_r0, 3, eof_idx).build();
    REQUIRE(states[8].kernel_matches(exp_kernel8));
    REQUIRE(states[8].matches(exp_kernel8));

    // State 9: kernel {S -> c C d . / $eof}
    auto exp_kernel9 = builder.reset()(s_idx, s_r1, 3, eof_idx).build();
    REQUIRE(states[9].kernel_matches(exp_kernel9));
    REQUIRE(states[9].matches(exp_kernel9));
}

TEST_CASE("parse_table_generator expr grammar sr conflicts", "[parse_table_generator]")
{
    ptg::symbol_collection sc;
    [[maybe_unused]] size_t root_idx = 0;
    [[maybe_unused]] size_t eof_idx = 0;
    [[maybe_unused]] size_t expr_idx = sc.add_nterm("Expr");
    [[maybe_unused]] size_t id_idx = sc.add_term("id");
    [[maybe_unused]] size_t plus_idx = sc.add_term("+", ptg::associativity::left(), 0);
    [[maybe_unused]] size_t minus_idx = sc.add_term("-", ptg::associativity::left(), 0);
    [[maybe_unused]] size_t mul_idx = sc.add_term("*", ptg::associativity::left(), 1);

    ptg::ruleset rs(sc);
    [[maybe_unused]] size_t expr_plus = rs.add_rule("Expr", {"Expr", "+", "Expr"});
    [[maybe_unused]] size_t expr_minus = rs.add_rule("Expr", {"Expr", "-", "Expr"});
    [[maybe_unused]] size_t expr_mul = rs.add_rule("Expr", {"Expr", "*", "Expr"});
    [[maybe_unused]] size_t expr_unary = rs.add_rule("Expr", {"-", "Expr"}, 2);
    [[maybe_unused]] size_t expr_id = rs.add_rule("Expr", {"id"});

    ptg::parse_table_generator ptg(rs);

    const auto& warnings = ptg.get_warnings();
    REQUIRE(warnings.empty());

    const auto& states = ptg.get_states();
    REQUIRE(states.size() == 11);

    auto dims = rs.get_lr1_set_item_space_dims();
    ptg::index_subset_builder<4> builder(dims);

    std::vector<size_t> all_la = {eof_idx, plus_idx, minus_idx, mul_idx};
    std::vector<size_t> all_rs = {expr_plus, expr_minus, expr_mul, expr_unary, expr_id};
    std::vector<size_t> bin_rs = {expr_plus, expr_minus, expr_mul};

    SECTION("states")
    {
        // State 0: initial
        builder.reset();
        builder(root_idx, 0, 0, eof_idx);
        auto exp_kernel0 = builder.build();
        REQUIRE(states[0].kernel_matches(exp_kernel0));
        for (auto la : all_la) {
            for (auto r : all_rs) {
                builder(expr_idx, r, 0, la);
            }
        }
        auto exp_items0 = builder.build();
        REQUIRE(states[0].matches(exp_items0));

        // State 1: Expr -> id . / all_la
        builder.reset();
        for (auto la : all_la) {
            builder(expr_idx, expr_id, 1, la);
        }
        auto exp_kernel1 = builder.build();
        REQUIRE(states[1].kernel_matches(exp_kernel1));
        REQUIRE(states[1].matches(exp_kernel1));

        // State 2: Expr -> - . Expr / all_la
        builder.reset();
        for (auto la : all_la) {
            builder(expr_idx, expr_unary, 1, la);
        }
        auto exp_kernel2 = builder.build();
        REQUIRE(states[2].kernel_matches(exp_kernel2));
        for (auto la : all_la) {
            for (auto r : all_rs) {
                builder(expr_idx, r, 0, la);
            }
        }
        auto exp_items2 = builder.build();
        REQUIRE(states[2].matches(exp_items2));

        // State 3: $root -> Expr . / eof, Expr -> Expr . + Expr / all_la, Expr -> Expr . - Expr / all_la, Expr -> Expr . * Expr / all_la
        builder.reset();
        builder(root_idx, 0, 1, eof_idx);
        for (auto la : all_la) {
            for (auto r : bin_rs) {
                builder(expr_idx, r, 1, la);
            }
        }
        auto exp_kernel3 = builder.build();
        REQUIRE(states[3].kernel_matches(exp_kernel3));
        REQUIRE(states[3].matches(exp_kernel3));

        // State 4: Expr -> - Expr . / all_la, Expr -> Expr . + Expr / all_la, Expr -> Expr . - Expr / all_la, Expr -> Expr . * Expr / all_la
        builder.reset();
        for (auto la : all_la) {
            builder(expr_idx, expr_unary, 2, la);
            for (auto r : bin_rs) {
                builder(expr_idx, r, 1, la);
            }
        }
        auto exp_kernel4 = builder.build();
        REQUIRE(states[4].kernel_matches(exp_kernel4));
        REQUIRE(states[4].matches(exp_kernel4));

        // State 5: Expr -> Expr + . Expr / all_la
        builder.reset();
        for (auto la : all_la) {
            builder(expr_idx, expr_plus, 2, la);
        }
        auto exp_kernel5 = builder.build();
        REQUIRE(states[5].kernel_matches(exp_kernel5));
        for (auto la : all_la) {
            for (auto r : all_rs) {
                builder(expr_idx, r, 0, la);
            }
        }
        auto exp_items5 = builder.build();
        REQUIRE(states[5].matches(exp_items5));

        // State 6: Expr -> Expr - . Expr / all_la
        builder.reset();
        for (auto la : all_la) {
            builder(expr_idx, expr_minus, 2, la);
        }
        auto exp_kernel6 = builder.build();
        REQUIRE(states[6].kernel_matches(exp_kernel6));
        for (auto la : all_la) {
            for (auto r : all_rs) {
                builder(expr_idx, r, 0, la);
            }
        }
        auto exp_items6 = builder.build();
        REQUIRE(states[6].matches(exp_items6));

        // State 7: Expr -> Expr * . Expr / all_la
        builder.reset();
        for (auto la : all_la) {
            builder(expr_idx, expr_mul, 2, la);
        }
        auto exp_kernel7 = builder.build();
        REQUIRE(states[7].kernel_matches(exp_kernel7));
        for (auto la : all_la) {
            for (auto r : all_rs) {
                builder(expr_idx, r, 0, la);
            }
        }
        auto exp_items7 = builder.build();
        REQUIRE(states[7].matches(exp_items7));

        // State 8: Expr -> Expr + Expr . / all_la, Expr -> Expr . + Expr / all_la, Expr -> Expr . - Expr / all_la, Expr -> Expr . * Expr / all_la
        builder.reset();
        for (auto la : all_la) {
            builder(expr_idx, expr_plus, 3, la);
            for (auto r : bin_rs) {
                builder(expr_idx, r, 1, la);
            }
        }
        auto exp_kernel8 = builder.build();
        REQUIRE(states[8].kernel_matches(exp_kernel8));
        REQUIRE(states[8].matches(exp_kernel8));

        // State 9: Expr -> Expr - Expr . / all_la, Expr -> Expr . + Expr / all_la, Expr -> Expr . - Expr / all_la, Expr -> Expr . * Expr / all_la
        builder.reset();
        for (auto la : all_la) {
            builder(expr_idx, expr_minus, 3, la);
            for (auto r : bin_rs) {
                builder(expr_idx, r, 1, la);
            }
        }
        auto exp_kernel9 = builder.build();
        REQUIRE(states[9].kernel_matches(exp_kernel9));
        REQUIRE(states[9].matches(exp_kernel9));

        // State 10: Expr -> Expr * Expr . / all_la, Expr -> Expr . + Expr / all_la, Expr -> Expr . - Expr / all_la, Expr -> Expr . * Expr / all_la
        builder.reset();
        for (auto la : all_la) {
            builder(expr_idx, expr_mul, 3, la);
            for (auto r : bin_rs) {
                builder(expr_idx, r, 1, la);
            }
        }
        auto exp_kernel10 = builder.build();
        REQUIRE(states[10].kernel_matches(exp_kernel10));
        REQUIRE(states[10].matches(exp_kernel10));
    }

    SECTION("conflict info")
    {
        const auto& infos = ptg.get_infos();
        REQUIRE(infos.size() == 12);
        REQUIRE(infos[0] == "Shift-reduce conflict in state 4 on symbol '+' resolved to reduction using Expr -> - Expr . / +");
        REQUIRE(infos[1] == "Shift-reduce conflict in state 4 on symbol '-' resolved to reduction using Expr -> - Expr . / -");
        REQUIRE(infos[2] == "Shift-reduce conflict in state 4 on symbol '*' resolved to reduction using Expr -> - Expr . / *");
        REQUIRE(infos[3] == "Shift-reduce conflict in state 8 on symbol '+' resolved to reduction using Expr -> Expr + Expr . / +");
        REQUIRE(infos[4] == "Shift-reduce conflict in state 8 on symbol '-' resolved to reduction using Expr -> Expr + Expr . / -");
        REQUIRE(infos[5] == "Shift-reduce conflict in state 8 on symbol '*' resolved to shift to state 7");
        REQUIRE(infos[6] == "Shift-reduce conflict in state 9 on symbol '+' resolved to reduction using Expr -> Expr - Expr . / +");
        REQUIRE(infos[7] == "Shift-reduce conflict in state 9 on symbol '-' resolved to reduction using Expr -> Expr - Expr . / -");
        REQUIRE(infos[8] == "Shift-reduce conflict in state 9 on symbol '*' resolved to shift to state 7");
        REQUIRE(infos[9] == "Shift-reduce conflict in state 10 on symbol '+' resolved to reduction using Expr -> Expr * Expr . / +");
        REQUIRE(infos[10] == "Shift-reduce conflict in state 10 on symbol '-' resolved to reduction using Expr -> Expr * Expr . / -");
        REQUIRE(infos[11] == "Shift-reduce conflict in state 10 on symbol '*' resolved to reduction using Expr -> Expr * Expr . / *");
    }
}
