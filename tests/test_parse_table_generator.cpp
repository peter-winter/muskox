#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <grammar_error.h>
#include <parse_table_generator.h>
#include <ruleset.h>
#include <symbol_collection.h>

using Catch::Matchers::Message;

using namespace muskox;
using teh = table_entry_hint;
using pte = parse_table_entry;
using st = symbol_type;

TEST_CASE("parse_table_generator rr conflict unresolved warnings", "[parse_table_generator]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t root_idx = 0;
    [[maybe_unused]] size_t eof_idx = 0;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_nterm_idx = sc.add_nterm("A");
    [[maybe_unused]] size_t b_nterm_idx = sc.add_nterm("B");
    [[maybe_unused]] size_t a_term_idx = sc.add_term("a");

    sc.validate();
    
    ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A"});
    [[maybe_unused]] size_t s_r1 = rs.add_rule("S", {"B"});
    [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"a"});
    [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"a"});

    rs.validate();
    
    parse_table_generator ptg(rs);

    SECTION("warnings")
    {
        const auto& warnings = ptg.get_warnings();
        REQUIRE(warnings.size() == 4);
        REQUIRE(warnings[0] == "Conflict in state 1 on lookahead '$eof' :");
        REQUIRE(warnings[1] == "\n    A -> a . / $eof");
        REQUIRE(warnings[2] == "\n    B -> a . / $eof");
        REQUIRE(warnings[3] == "Conflict in state 1 on lookahead '$eof' unresolved");
    }
}

TEST_CASE("parse_table_generator sr + multiple reductions conflict resolved to shift", "[parse_table_generator]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t root_idx = 0;
    [[maybe_unused]] size_t eof_idx = 0;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_nterm_idx = sc.add_nterm("A");
    [[maybe_unused]] size_t b_nterm_idx = sc.add_nterm("B");
    [[maybe_unused]] size_t c_nterm_idx = sc.add_nterm("C");
    [[maybe_unused]] size_t a_term_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_term_idx = sc.add_term("b", 2);

    sc.validate();
    
    ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A", "b"});
    [[maybe_unused]] size_t s_r1 = rs.add_rule("S", {"B", "b"});
    [[maybe_unused]] size_t s_r2 = rs.add_rule("S", {"C", "b"});
    [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"a"});
    [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"a"});
    [[maybe_unused]] size_t c_r0 = rs.add_rule("C", {"a", "b"});

    rs.validate();
    
    parse_table_generator ptg(rs);

    SECTION("warnings")
    {
        const auto& warnings = ptg.get_warnings();
        REQUIRE(warnings.size() == 5);
        REQUIRE(warnings[0] == "Conflict in state 1 on lookahead 'b' :");
        REQUIRE(warnings[1] == "\n    A -> a . / b");
        REQUIRE(warnings[2] == "\n    B -> a . / b");
        REQUIRE(warnings[3] == "\n    shift on 'b' to state 6 has the highest precedence");
        REQUIRE(warnings[4] == "Conflict in state 1 on lookahead 'b' resolved");
    }
}

TEST_CASE("parse_table_generator sr + multiple reductions conflict resolved to one of the reduce", "[parse_table_generator]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t root_idx = 0;
    [[maybe_unused]] size_t eof_idx = 0;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_nterm_idx = sc.add_nterm("A");
    [[maybe_unused]] size_t b_nterm_idx = sc.add_nterm("B");
    [[maybe_unused]] size_t c_nterm_idx = sc.add_nterm("C");
    [[maybe_unused]] size_t a_term_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_term_idx = sc.add_term("b");
    
    sc.validate();

    ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A", "b"});
    [[maybe_unused]] size_t s_r1 = rs.add_rule("S", {"B", "b"});
    [[maybe_unused]] size_t s_r2 = rs.add_rule("S", {"C", "b"});
    [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"a"}, 2);
    [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"a"});
    [[maybe_unused]] size_t c_r0 = rs.add_rule("C", {"a", "b"});

    rs.validate();
    
    parse_table_generator ptg(rs);

    SECTION("warnings")
    {
        const auto& warnings = ptg.get_warnings();
        REQUIRE(warnings.size() == 5);
        REQUIRE(warnings[0] == "Conflict in state 1 on lookahead 'b' :");
        REQUIRE(warnings[1] == "\n    A -> a . / b (highest precedence)");
        REQUIRE(warnings[2] == "\n    B -> a . / b");
        REQUIRE(warnings[3] == "\n    shift on 'b'");
        REQUIRE(warnings[4] == "Conflict in state 1 on lookahead 'b' resolved");
    }
}

TEST_CASE("parse_table_generator sr + multiple reductions conflict unresolved", "[parse_table_generator]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t root_idx = 0;
    [[maybe_unused]] size_t eof_idx = 0;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_nterm_idx = sc.add_nterm("A");
    [[maybe_unused]] size_t b_nterm_idx = sc.add_nterm("B");
    [[maybe_unused]] size_t c_nterm_idx = sc.add_nterm("C");
    [[maybe_unused]] size_t a_term_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_term_idx = sc.add_term("b");
    
    sc.validate();

    ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A", "b"});
    [[maybe_unused]] size_t s_r1 = rs.add_rule("S", {"B", "b"});
    [[maybe_unused]] size_t s_r2 = rs.add_rule("S", {"C", "b"});
    [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"a"});
    [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"a"});
    [[maybe_unused]] size_t c_r0 = rs.add_rule("C", {"a", "b"});

    rs.validate();
    
    parse_table_generator ptg(rs);

    SECTION("warnings")
    {
        const auto& warnings = ptg.get_warnings();
        REQUIRE(warnings.size() == 5);
        REQUIRE(warnings[0] == "Conflict in state 1 on lookahead 'b' :");
        REQUIRE(warnings[1] == "\n    A -> a . / b");
        REQUIRE(warnings[2] == "\n    B -> a . / b");
        REQUIRE(warnings[3] == "\n    shift on 'b'");
        REQUIRE(warnings[4] == "Conflict in state 1 on lookahead 'b' unresolved");
    }
}

TEST_CASE("parse_table_generator states simple", "[parse_table_generator]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t root_idx = 0;
    [[maybe_unused]] size_t eof_idx = 0;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t c_idx = sc.add_term("c");
    
    sc.validate();

    ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a", "B"});
    [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"c"});

    rs.validate();
    
    parse_table_generator ptg(rs);

    const auto& states = ptg.get_states();
    REQUIRE(states.size() == 5);

    auto dims = rs.get_lr1_set_item_space_dims();
    index_subset_builder<4> builder(dims);

    SECTION("states")
    {
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

    SECTION("hints")
    {
        const auto& hints = ptg.get_table_entry_hints();
        REQUIRE(hints.size() == 7);

        CHECK(hints[0] == teh(0, {st::terminal, a_idx}, pte::shift(1)));
        CHECK(hints[1] == teh(0, {st::non_terminal, s_idx}, pte::shift(2)));
        CHECK(hints[2] == teh(1, {st::terminal, c_idx}, pte::shift(3)));
        CHECK(hints[3] == teh(1, {st::non_terminal, b_idx}, pte::shift(4)));
        CHECK(hints[4] == teh(2, {st::terminal, eof_idx}, pte::reduce(root_idx, 0)));
        CHECK(hints[5] == teh(3, {st::terminal, eof_idx}, pte::reduce(b_idx, 0)));
        CHECK(hints[6] == teh(4, {st::terminal, eof_idx}, pte::reduce(s_idx, 0)));
    }
}

TEST_CASE("parse_table_generator rr conflict resolved", "[parse_table_generator]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t root_idx = 0;
    [[maybe_unused]] size_t eof_idx = 0;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_nterm_idx = sc.add_nterm("A");
    [[maybe_unused]] size_t b_nterm_idx = sc.add_nterm("B");
    [[maybe_unused]] size_t a_term_idx = sc.add_term("a");
    
    sc.validate();

    ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A"});
    [[maybe_unused]] size_t s_r1 = rs.add_rule("S", {"B"});
    [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"a"}, 3);
    [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"a"});

    rs.validate();
    
    parse_table_generator ptg(rs);

    SECTION("warnings")
    {
        const auto& warnings = ptg.get_warnings();
        REQUIRE(warnings.size() == 4);
        REQUIRE(warnings[0] == "Conflict in state 1 on lookahead '$eof' :");
        REQUIRE(warnings[1] == "\n    A -> a . / $eof (highest precedence)");
        REQUIRE(warnings[2] == "\n    B -> a . / $eof");
        REQUIRE(warnings[3] == "Conflict in state 1 on lookahead '$eof' resolved");
    }

    SECTION("states")
    {
        const auto& states = ptg.get_states();
        REQUIRE(states.size() == 5);

        auto dims = rs.get_lr1_set_item_space_dims();
        index_subset_builder<4> builder(dims);

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

    SECTION("hints")
    {
        const auto& hints = ptg.get_table_entry_hints();
        REQUIRE(hints.size() == 8);

        CHECK(hints[0] == teh(0, {st::terminal, a_term_idx}, pte::shift(1)));
        CHECK(hints[1] == teh(0, {st::non_terminal, s_idx}, pte::shift(2)));
        CHECK(hints[2] == teh(0, {st::non_terminal, a_nterm_idx}, pte::shift(3)));
        CHECK(hints[3] == teh(0, {st::non_terminal, b_nterm_idx}, pte::shift(4)));
        CHECK(hints[4] == teh(1, {st::terminal, eof_idx}, pte::reduce(a_nterm_idx, 0)));
        CHECK(hints[5] == teh(2, {st::terminal, eof_idx}, pte::reduce(root_idx, 0)));
        CHECK(hints[6] == teh(3, {st::terminal, eof_idx}, pte::reduce(s_idx, 0)));
        CHECK(hints[7] == teh(4, {st::terminal, eof_idx}, pte::reduce(s_idx, 1)));
    }
}

TEST_CASE("parse_table_generator states complex lookaheads", "[parse_table_generator]")
{
    symbol_collection sc;
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
    
    sc.validate();

    ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a", "A", "b"});
    [[maybe_unused]] size_t s_r1 = rs.add_rule("S", {"c", "C", "d"});
    [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"e"});
    [[maybe_unused]] size_t c_r0 = rs.add_rule("C", {"e"});
    
    rs.validate();

    parse_table_generator ptg(rs);

    const auto& states = ptg.get_states();
    REQUIRE(states.size() == 10);

    auto dims = rs.get_lr1_set_item_space_dims();
    index_subset_builder<4> builder(dims);

    SECTION("states")
    {
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

    SECTION("hints")
    {
        const auto& hints = ptg.get_table_entry_hints();
        REQUIRE(hints.size() == 14);

        CHECK(hints[0] == teh(0, {st::terminal, a_term_idx}, pte::shift(1)));
        CHECK(hints[1] == teh(0, {st::terminal, c_term_idx}, pte::shift(2)));
        CHECK(hints[2] == teh(0, {st::non_terminal, s_idx}, pte::shift(3)));
        CHECK(hints[3] == teh(1, {st::terminal, e_term_idx}, pte::shift(4)));
        CHECK(hints[4] == teh(1, {st::non_terminal, a_nterm_idx}, pte::shift(5)));
        CHECK(hints[5] == teh(2, {st::terminal, e_term_idx}, pte::shift(6)));
        CHECK(hints[6] == teh(2, {st::non_terminal, c_nterm_idx}, pte::shift(7)));
        CHECK(hints[7] == teh(3, {st::terminal, eof_idx}, pte::reduce(root_idx, 0)));
        CHECK(hints[8] == teh(4, {st::terminal, b_term_idx}, pte::reduce(a_nterm_idx, 0)));
        CHECK(hints[9] == teh(5, {st::terminal, b_term_idx}, pte::shift(8)));
        CHECK(hints[10] == teh(6, {st::terminal, d_term_idx}, pte::reduce(c_nterm_idx, 0)));
        CHECK(hints[11] == teh(7, {st::terminal, d_term_idx}, pte::shift(9)));
        CHECK(hints[12] == teh(8, {st::terminal, eof_idx}, pte::reduce(s_idx, 0)));
        CHECK(hints[13] == teh(9, {st::terminal, eof_idx}, pte::reduce(s_idx, 1)));
    }
}

TEST_CASE("parse_table_generator unary minus grammar sr conflicts", "[parse_table_generator]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t root_idx = 0;
    [[maybe_unused]] size_t eof_idx = 0;
    [[maybe_unused]] size_t expr_idx = sc.add_nterm("Expr");
    [[maybe_unused]] size_t id_idx = sc.add_term("id");
    [[maybe_unused]] size_t plus_idx = sc.add_term("+", 1);
    [[maybe_unused]] size_t minus_idx = sc.add_term("-", 1);
    [[maybe_unused]] size_t mul_idx = sc.add_term("*", 2);
    
    sc.validate();

    ruleset rs(sc);
    [[maybe_unused]] size_t expr_plus = rs.add_rule("Expr", {"Expr", "+", "Expr"});
    [[maybe_unused]] size_t expr_minus = rs.add_rule("Expr", {"Expr", "-", "Expr"});
    [[maybe_unused]] size_t expr_mul = rs.add_rule("Expr", {"Expr", "*", "Expr"});
    [[maybe_unused]] size_t expr_neg = rs.add_rule("Expr", {"-", "Expr"}, 3);
    [[maybe_unused]] size_t expr_id = rs.add_rule("Expr", {"id"});

    rs.validate();
    
    parse_table_generator ptg(rs);

    const auto& states = ptg.get_states();
    REQUIRE(states.size() == 11);

    auto dims = rs.get_lr1_set_item_space_dims();
    index_subset_builder<4> builder(dims);

    std::vector<size_t> all_la = {eof_idx, plus_idx, minus_idx, mul_idx};
    std::vector<size_t> all_rs = {expr_plus, expr_minus, expr_mul, expr_neg, expr_id};
    std::vector<size_t> bin_rs = {expr_plus, expr_minus, expr_mul};

    SECTION("states")
    {
        // State 0: kernel {$root -> . Expr / $eof}, items: Expr -> . Expr + Expr / all_la, Expr -> . Expr - Expr / all_la, Expr -> . Expr * Expr / all_la, Expr -> . - Expr / all_la, Expr -> . id / all_la
        builder.reset();
        builder(root_idx, 0, 0, eof_idx);
        auto exp_kernel0 = builder.build();
        REQUIRE(states[0].kernel_matches(exp_kernel0));
        for (auto la : all_la)
        {
            for (auto r : all_rs)
            {
                builder(expr_idx, r, 0, la);
            }
        }
        auto exp_items0 = builder.build();
        REQUIRE(states[0].matches(exp_items0));

        // State 1: kernel {Expr -> id . / all_la}
        builder.reset();
        for (auto la : all_la)
        {
            builder(expr_idx, expr_id, 1, la);
        }
        auto exp_kernel1 = builder.build();
        REQUIRE(states[1].kernel_matches(exp_kernel1));
        REQUIRE(states[1].matches(exp_kernel1));

        // State 2: kernel {Expr -> - . Expr / all_la}, items: Expr -> . Expr + Expr / all_la, Expr -> . Expr - Expr / all_la, Expr -> . Expr * Expr / all_la, Expr -> . - Expr / all_la, Expr -> . id / all_la
        builder.reset();
        for (auto la : all_la)
        {
            builder(expr_idx, expr_neg, 1, la);
        }
        auto exp_kernel2 = builder.build();
        REQUIRE(states[2].kernel_matches(exp_kernel2));
        for (auto la : all_la)
        {
            for (auto r : all_rs)
            {
                builder(expr_idx, r, 0, la);
            }
        }
        auto exp_items2 = builder.build();
        REQUIRE(states[2].matches(exp_items2));

        // State 3: kernel {$root -> Expr . / $eof}, Expr -> Expr . + Expr / all_la, Expr -> Expr . - Expr / all_la, Expr -> Expr . * Expr / all_la
        builder.reset();
        builder(root_idx, 0, 1, eof_idx);
        for (auto la : all_la)
        {
            for (auto r : bin_rs)
            {
                builder(expr_idx, r, 1, la);
            }
        }
        auto exp_kernel3 = builder.build();
        REQUIRE(states[3].kernel_matches(exp_kernel3));
        REQUIRE(states[3].matches(exp_kernel3));

        // State 4: kernel {Expr -> - Expr . / all_la}, Expr -> Expr . + Expr / all_la, Expr -> Expr . - Expr / all_la, Expr -> Expr . * Expr / all_la
        builder.reset();
        for (auto la : all_la)
        {
            builder(expr_idx, expr_neg, 2, la);
            for (auto r : bin_rs)
            {
                builder(expr_idx, r, 1, la);
            }
        }
        auto exp_kernel4 = builder.build();
        REQUIRE(states[4].kernel_matches(exp_kernel4));
        REQUIRE(states[4].matches(exp_kernel4));

        // State 5: kernel {Expr -> Expr + . Expr / all_la}, items: Expr -> . Expr + Expr / all_la, Expr -> . Expr - Expr / all_la, Expr -> . Expr * Expr / all_la, Expr -> . - Expr / all_la, Expr -> . id / all_la
        builder.reset();
        for (auto la : all_la)
        {
            builder(expr_idx, expr_plus, 2, la);
        }
        auto exp_kernel5 = builder.build();
        REQUIRE(states[5].kernel_matches(exp_kernel5));
        for (auto la : all_la)
        {
            for (auto r : all_rs)
            {
                builder(expr_idx, r, 0, la);
            }
        }
        auto exp_items5 = builder.build();
        REQUIRE(states[5].matches(exp_items5));

        // State 6: kernel {Expr -> Expr - . Expr / all_la}, items: Expr -> . Expr + Expr / all_la, Expr -> . Expr - Expr / all_la, Expr -> . Expr * Expr / all_la, Expr -> . - Expr / all_la, Expr -> . id / all_la
        builder.reset();
        for (auto la : all_la)
        {
            builder(expr_idx, expr_minus, 2, la);
        }
        auto exp_kernel6 = builder.build();
        REQUIRE(states[6].kernel_matches(exp_kernel6));
        for (auto la : all_la)
        {
            for (auto r : all_rs)
            {
                builder(expr_idx, r, 0, la);
            }
        }
        auto exp_items6 = builder.build();
        REQUIRE(states[6].matches(exp_items6));

        // State 7: kernel {Expr -> Expr * . Expr / all_la}, items: Expr -> . Expr + Expr / all_la, Expr -> . Expr - Expr / all_la, Expr -> . Expr * Expr / all_la, Expr -> . - Expr / all_la, Expr -> . id / all_la
        builder.reset();
        for (auto la : all_la)
        {
            builder(expr_idx, expr_mul, 2, la);
        }
        auto exp_kernel7 = builder.build();
        REQUIRE(states[7].kernel_matches(exp_kernel7));
        for (auto la : all_la)
        {
            for (auto r : all_rs)
            {
                builder(expr_idx, r, 0, la);
            }
        }
        auto exp_items7 = builder.build();
        REQUIRE(states[7].matches(exp_items7));

        // State 8: Expr -> Expr + Expr . / all_la, Expr -> Expr . + Expr / all_la, Expr -> Expr . - Expr / all_la, Expr -> Expr . * Expr / all_la
        builder.reset();
        for (auto la : all_la)
        {
            builder(expr_idx, expr_plus, 3, la);
            for (auto r : bin_rs)
            {
                builder(expr_idx, r, 1, la);
            }
        }
        auto exp_kernel8 = builder.build();
        REQUIRE(states[8].kernel_matches(exp_kernel8));
        REQUIRE(states[8].matches(exp_kernel8));

        // State 9: Expr -> Expr - Expr . / all_la, Expr -> Expr . + Expr / all_la, Expr -> Expr . - Expr / all_la, Expr -> Expr . * Expr / all_la
        builder.reset();
        for (auto la : all_la)
        {
            builder(expr_idx, expr_minus, 3, la);
            for (auto r : bin_rs)
            {
                builder(expr_idx, r, 1, la);
            }
        }
        auto exp_kernel9 = builder.build();
        REQUIRE(states[9].kernel_matches(exp_kernel9));
        REQUIRE(states[9].matches(exp_kernel9));

        // State 10: Expr -> Expr * Expr . / all_la, Expr -> Expr . + Expr / all_la, Expr -> Expr . - Expr / all_la, Expr -> Expr . * Expr / all_la
        builder.reset();
        for (auto la : all_la)
        {
            builder(expr_idx, expr_mul, 3, la);
            for (auto r : bin_rs)
            {
                builder(expr_idx, r, 1, la);
            }
        }
        auto exp_kernel10 = builder.build();
        REQUIRE(states[10].kernel_matches(exp_kernel10));
        REQUIRE(states[10].matches(exp_kernel10));
    }

    SECTION("conflict warnings")
    {
        const auto& warnings = ptg.get_warnings();
        REQUIRE(warnings.size() == 48);
        REQUIRE(warnings[ 0] == "Conflict in state 4 on lookahead '+' :");
        REQUIRE(warnings[ 1] == "\n    Expr -> - Expr . / + (highest precedence)");
        REQUIRE(warnings[ 2] == "\n    shift on '+'");
        REQUIRE(warnings[ 3] == "Conflict in state 4 on lookahead '+' resolved");
        
        REQUIRE(warnings[ 4] == "Conflict in state 4 on lookahead '-' :");
        REQUIRE(warnings[ 5] == "\n    Expr -> - Expr . / - (highest precedence)");
        REQUIRE(warnings[ 6] == "\n    shift on '-'");
        REQUIRE(warnings[ 7] == "Conflict in state 4 on lookahead '-' resolved");
        
        REQUIRE(warnings[ 8] == "Conflict in state 4 on lookahead '*' :");
        REQUIRE(warnings[ 9] == "\n    Expr -> - Expr . / * (highest precedence)");
        REQUIRE(warnings[10] == "\n    shift on '*'");
        REQUIRE(warnings[11] == "Conflict in state 4 on lookahead '*' resolved");
        
        REQUIRE(warnings[12] == "Conflict in state 8 on lookahead '+' :");
        REQUIRE(warnings[13] == "\n    Expr -> Expr + Expr . / + (highest precedence)");
        REQUIRE(warnings[14] == "\n    shift on '+'");
        REQUIRE(warnings[15] == "Conflict in state 8 on lookahead '+' resolved");
        
        REQUIRE(warnings[16] == "Conflict in state 8 on lookahead '-' :");
        REQUIRE(warnings[17] == "\n    Expr -> Expr + Expr . / - (highest precedence)");
        REQUIRE(warnings[18] == "\n    shift on '-'");
        REQUIRE(warnings[19] == "Conflict in state 8 on lookahead '-' resolved");
        
        REQUIRE(warnings[20] == "Conflict in state 8 on lookahead '*' :");
        REQUIRE(warnings[21] == "\n    Expr -> Expr + Expr . / *");
        REQUIRE(warnings[22] == "\n    shift on '*' to state 7 has the highest precedence");
        REQUIRE(warnings[23] == "Conflict in state 8 on lookahead '*' resolved");
        
        REQUIRE(warnings[24] == "Conflict in state 9 on lookahead '+' :");
        REQUIRE(warnings[25] == "\n    Expr -> Expr - Expr . / + (highest precedence)");
        REQUIRE(warnings[26] == "\n    shift on '+'");
        REQUIRE(warnings[27] == "Conflict in state 9 on lookahead '+' resolved");
        
        REQUIRE(warnings[28] == "Conflict in state 9 on lookahead '-' :");
        REQUIRE(warnings[29] == "\n    Expr -> Expr - Expr . / - (highest precedence)");
        REQUIRE(warnings[30] == "\n    shift on '-'");
        REQUIRE(warnings[31] == "Conflict in state 9 on lookahead '-' resolved");
        
        REQUIRE(warnings[32] == "Conflict in state 9 on lookahead '*' :");
        REQUIRE(warnings[33] == "\n    Expr -> Expr - Expr . / *");
        REQUIRE(warnings[34] == "\n    shift on '*' to state 7 has the highest precedence");
        REQUIRE(warnings[35] == "Conflict in state 9 on lookahead '*' resolved");
        
        REQUIRE(warnings[36] == "Conflict in state 10 on lookahead '+' :");
        REQUIRE(warnings[37] == "\n    Expr -> Expr * Expr . / + (highest precedence)");
        REQUIRE(warnings[38] == "\n    shift on '+'");
        REQUIRE(warnings[39] == "Conflict in state 10 on lookahead '+' resolved");
        
        REQUIRE(warnings[40] == "Conflict in state 10 on lookahead '-' :");
        REQUIRE(warnings[41] == "\n    Expr -> Expr * Expr . / - (highest precedence)");
        REQUIRE(warnings[42] == "\n    shift on '-'");
        REQUIRE(warnings[43] == "Conflict in state 10 on lookahead '-' resolved");
        
        REQUIRE(warnings[44] == "Conflict in state 10 on lookahead '*' :");
        REQUIRE(warnings[45] == "\n    Expr -> Expr * Expr . / * (highest precedence)");
        REQUIRE(warnings[46] == "\n    shift on '*'");
        REQUIRE(warnings[47] == "Conflict in state 10 on lookahead '*' resolved");
    }

    SECTION("hints")
    {
        const auto& hints = ptg.get_table_entry_hints();
        REQUIRE(hints.size() == 39);  // As per previous completion

        // State 0 shifts
        CHECK(hints[0] == teh(0, {st::terminal, id_idx}, pte::shift(1)));
        CHECK(hints[1] == teh(0, {st::terminal, minus_idx}, pte::shift(2)));
        CHECK(hints[2] == teh(0, {st::non_terminal, expr_idx}, pte::shift(3)));

        // Reduces from state 1 for all_la
        CHECK(hints[3] == teh(1, {st::terminal, all_la[0]}, pte::reduce(expr_idx, expr_id)));
        CHECK(hints[4] == teh(1, {st::terminal, all_la[1]}, pte::reduce(expr_idx, expr_id)));
        CHECK(hints[5] == teh(1, {st::terminal, all_la[2]}, pte::reduce(expr_idx, expr_id)));
        CHECK(hints[6] == teh(1, {st::terminal, all_la[3]}, pte::reduce(expr_idx, expr_id)));

        // State 2 shifts
        CHECK(hints[7] == teh(2, {st::terminal, id_idx}, pte::shift(1)));
        CHECK(hints[8] == teh(2, {st::terminal, minus_idx}, pte::shift(2)));
        CHECK(hints[9] == teh(2, {st::non_terminal, expr_idx}, pte::shift(4)));

        // State 3 reduce
        CHECK(hints[10] == teh(3, {st::terminal, eof_idx}, pte::reduce(root_idx, 0)));

        // State 3 shifts
        CHECK(hints[11] == teh(3, {st::terminal, plus_idx}, pte::shift(5)));
        CHECK(hints[12] == teh(3, {st::terminal, minus_idx}, pte::shift(6)));
        CHECK(hints[13] == teh(3, {st::terminal, mul_idx}, pte::shift(7)));

        // State 4 resolved reduces
        CHECK(hints[14] == teh(4, {st::terminal, eof_idx}, pte::reduce(expr_idx, expr_neg)));        
        CHECK(hints[15] == teh(4, {st::terminal, plus_idx}, pte::reduce(expr_idx, expr_neg)));
        CHECK(hints[16] == teh(4, {st::terminal, minus_idx}, pte::reduce(expr_idx, expr_neg)));
        CHECK(hints[17] == teh(4, {st::terminal, mul_idx}, pte::reduce(expr_idx, expr_neg)));

        // State 5 shifts
        CHECK(hints[18] == teh(5, {st::terminal, id_idx}, pte::shift(1)));
        CHECK(hints[19] == teh(5, {st::terminal, minus_idx}, pte::shift(2)));
        CHECK(hints[20] == teh(5, {st::non_terminal, expr_idx}, pte::shift(8)));

        // State 6 shifts
        CHECK(hints[21] == teh(6, {st::terminal, id_idx}, pte::shift(1)));
        CHECK(hints[22] == teh(6, {st::terminal, minus_idx}, pte::shift(2)));
        CHECK(hints[23] == teh(6, {st::non_terminal, expr_idx}, pte::shift(9)));

        // State 7 shifts
        CHECK(hints[24] == teh(7, {st::terminal, id_idx}, pte::shift(1)));
        CHECK(hints[25] == teh(7, {st::terminal, minus_idx}, pte::shift(2)));
        CHECK(hints[26] == teh(7, {st::non_terminal, expr_idx}, pte::shift(10)));

        // State 8 resolved
        CHECK(hints[27] == teh(8, {st::terminal, eof_idx}, pte::reduce(expr_idx, expr_plus)));
        CHECK(hints[28] == teh(8, {st::terminal, plus_idx}, pte::reduce(expr_idx, expr_plus)));
        CHECK(hints[29] == teh(8, {st::terminal, minus_idx}, pte::reduce(expr_idx, expr_plus)));

        CHECK(hints[30] == teh(8, {st::terminal, mul_idx}, pte::shift(7)));

        // State 9 resolved
        CHECK(hints[31] == teh(9, {st::terminal, eof_idx}, pte::reduce(expr_idx, expr_minus)));
        CHECK(hints[32] == teh(9, {st::terminal, plus_idx}, pte::reduce(expr_idx, expr_minus)));
        CHECK(hints[33] == teh(9, {st::terminal, minus_idx}, pte::reduce(expr_idx, expr_minus)));

        CHECK(hints[34] == teh(9, {st::terminal, mul_idx}, pte::shift(7)));

        // State 10 resolved
        CHECK(hints[35] == teh(10, {st::terminal, eof_idx}, pte::reduce(expr_idx, expr_mul)));
        CHECK(hints[36] == teh(10, {st::terminal, plus_idx}, pte::reduce(expr_idx, expr_mul)));
        CHECK(hints[37] == teh(10, {st::terminal, minus_idx}, pte::reduce(expr_idx, expr_mul)));
        CHECK(hints[38] == teh(10, {st::terminal, mul_idx}, pte::reduce(expr_idx, expr_mul)));
    }
}

TEST_CASE("parse_table_generator right assoc grammar sr conflicts", "[parse_table_generator]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t root_idx = 0;
    [[maybe_unused]] size_t eof_idx = 0;
    [[maybe_unused]] size_t expr_idx = sc.add_nterm("Expr");
    [[maybe_unused]] size_t id_idx = sc.add_term("id");
    [[maybe_unused]] size_t plus_idx = sc.add_term("+", 1);
    [[maybe_unused]] size_t pow_idx = sc.add_term("^", 2, associativity::right());
    
    sc.validate();

    ruleset rs(sc);
    [[maybe_unused]] size_t expr_plus = rs.add_rule("Expr", {"Expr", "+", "Expr"});
    [[maybe_unused]] size_t expr_pow = rs.add_rule("Expr", {"Expr", "^", "Expr"});
    [[maybe_unused]] size_t expr_id = rs.add_rule("Expr", {"id"});

    rs.validate();
    
    parse_table_generator ptg(rs);

    const auto& states = ptg.get_states();
    REQUIRE(states.size() == 7);

    auto dims = rs.get_lr1_set_item_space_dims();
    index_subset_builder<4> builder(dims);

    std::vector<size_t> all_la = {eof_idx, plus_idx, pow_idx};
    std::vector<size_t> all_rs = {expr_plus, expr_pow, expr_id};
    std::vector<size_t> bin_rs = {expr_plus, expr_pow};

    SECTION("states")
    {
        // State 0: initial
        builder.reset();
        builder(root_idx, 0, 0, eof_idx);
        auto exp_kernel0 = builder.build();
        REQUIRE(states[0].kernel_matches(exp_kernel0));
        for (auto la : all_la)
        {
            for (auto r : all_rs)
            {
                builder(expr_idx, r, 0, la);
            }
        }
        auto exp_items0 = builder.build();
        REQUIRE(states[0].matches(exp_items0));

        // State 1: Expr -> id . / all_la
        builder.reset();
        for (auto la : all_la)
        {
            builder(expr_idx, expr_id, 1, la);
        }
        auto exp_kernel1 = builder.build();
        REQUIRE(states[1].kernel_matches(exp_kernel1));
        REQUIRE(states[1].matches(exp_kernel1));

        // State 2: $root -> Expr . / eof, Expr -> Expr . + Expr / all_la, Expr -> Expr . ^ Expr / all_la
        builder.reset();
        builder(root_idx, 0, 1, eof_idx);
        for (auto la : all_la)
        {
            for (auto r : bin_rs)
            {
                builder(expr_idx, r, 1, la);
            }
        }
        auto exp_kernel2 = builder.build();
        REQUIRE(states[2].kernel_matches(exp_kernel2));
        REQUIRE(states[2].matches(exp_kernel2));

        // State 3: Expr -> Expr + . Expr / all_la
        builder.reset();
        for (auto la : all_la)
        {
            builder(expr_idx, expr_plus, 2, la);
        }
        auto exp_kernel3 = builder.build();
        REQUIRE(states[3].kernel_matches(exp_kernel3));
        for (auto la : all_la)
        {
            for (auto r : all_rs)
            {
                builder(expr_idx, r, 0, la);
            }
        }
        auto exp_items3 = builder.build();
        REQUIRE(states[3].matches(exp_items3));

        // State 4: Expr -> Expr ^ . Expr / all_la
        builder.reset();
        for (auto la : all_la)
        {
            builder(expr_idx, expr_pow, 2, la);
        }
        auto exp_kernel4 = builder.build();
        REQUIRE(states[4].kernel_matches(exp_kernel4));
        for (auto la : all_la)
        {
            for (auto r : all_rs)
            {
                builder(expr_idx, r, 0, la);
            }
        }
        auto exp_items4 = builder.build();
        REQUIRE(states[4].matches(exp_items4));

        // State 5: Expr -> Expr + Expr . / all_la, Expr -> Expr . + Expr / all_la, Expr -> Expr . ^ Expr / all_la
        builder.reset();
        for (auto la : all_la)
        {
            builder(expr_idx, expr_plus, 3, la);
            for (auto r : bin_rs)
            {
                builder(expr_idx, r, 1, la);
            }
        }
        auto exp_kernel5 = builder.build();
        REQUIRE(states[5].kernel_matches(exp_kernel5));
        REQUIRE(states[5].matches(exp_kernel5));

        // State 6: Expr -> Expr ^ Expr . / all_la, Expr -> Expr . + Expr / all_la, Expr -> Expr . ^ Expr / all_la
        builder.reset();
        for (auto la : all_la)
        {
            builder(expr_idx, expr_pow, 3, la);
            for (auto r : bin_rs)
            {
                builder(expr_idx, r, 1, la);
            }
        }
        auto exp_kernel6 = builder.build();
        REQUIRE(states[6].kernel_matches(exp_kernel6));
        REQUIRE(states[6].matches(exp_kernel6));
    }
    
    SECTION("conflict warnings")
    {
        const auto& warnings = ptg.get_warnings();
        REQUIRE(warnings.size() == 16);
        
        REQUIRE(warnings[ 0] == "Conflict in state 5 on lookahead '+' :");
        REQUIRE(warnings[ 1] == "\n    Expr -> Expr + Expr . / + (highest precedence)");
        REQUIRE(warnings[ 2] == "\n    shift on '+'");
        REQUIRE(warnings[ 3] == "Conflict in state 5 on lookahead '+' resolved");
        
        REQUIRE(warnings[ 4] == "Conflict in state 5 on lookahead '^' :");
        REQUIRE(warnings[ 5] == "\n    Expr -> Expr + Expr . / ^");
        REQUIRE(warnings[ 6] == "\n    shift on '^' to state 4 has the highest precedence");
        REQUIRE(warnings[ 7] == "Conflict in state 5 on lookahead '^' resolved");
        
        REQUIRE(warnings[ 8] == "Conflict in state 6 on lookahead '+' :");
        REQUIRE(warnings[ 9] == "\n    Expr -> Expr ^ Expr . / + (highest precedence)");
        REQUIRE(warnings[10] == "\n    shift on '+'");
        REQUIRE(warnings[11] == "Conflict in state 6 on lookahead '+' resolved");
        
        REQUIRE(warnings[12] == "Conflict in state 6 on lookahead '^' :");
        REQUIRE(warnings[13] == "\n    Expr -> Expr ^ Expr . / ^");
        REQUIRE(warnings[14] == "\n    shift on '^' to state 4 has the highest precedence");
        REQUIRE(warnings[15] == "Conflict in state 6 on lookahead '^' resolved");
    }

    SECTION("table")
    {
        const auto& hints = ptg.get_table_entry_hints();
        REQUIRE(hints.size() == 18);

        CHECK(hints[0] == teh(0, {st::terminal, id_idx}, pte::shift(1)));
        CHECK(hints[1] == teh(0, {st::non_terminal, expr_idx}, pte::shift(2)));

        // Reduces from state 1 for all_la
        for (size_t i = 0; i < 3; ++i)
        {
            CHECK(hints[2 + i] == teh(1, {st::terminal, all_la[i]}, pte::reduce(expr_idx, expr_id)));
        }

        CHECK(hints[5] == teh(2, {st::terminal, eof_idx}, pte::reduce(root_idx, 0)));        
        CHECK(hints[6] == teh(2, {st::terminal, plus_idx}, pte::shift(3)));
        CHECK(hints[7] == teh(2, {st::terminal, pow_idx}, pte::shift(4)));

        CHECK(hints[8] == teh(3, {st::terminal, id_idx}, pte::shift(1)));
        CHECK(hints[9] == teh(3, {st::non_terminal, expr_idx}, pte::shift(5)));

        CHECK(hints[10] == teh(4, {st::terminal, id_idx}, pte::shift(1)));
        CHECK(hints[11] == teh(4, {st::non_terminal, expr_idx}, pte::shift(6)));

        CHECK(hints[12] == teh(5, {st::terminal, eof_idx}, pte::reduce(expr_idx, expr_plus)));        
        CHECK(hints[13] == teh(5, {st::terminal, plus_idx}, pte::reduce(expr_idx, expr_plus)));
        CHECK(hints[14] == teh(5, {st::terminal, pow_idx}, pte::shift(4)));

        CHECK(hints[15] == teh(6, {st::terminal, eof_idx}, pte::reduce(expr_idx, expr_pow)));        
        CHECK(hints[16] == teh(6, {st::terminal, plus_idx}, pte::reduce(expr_idx, expr_pow)));
        CHECK(hints[17] == teh(6, {st::terminal, pow_idx}, pte::shift(4)));
    }
}
