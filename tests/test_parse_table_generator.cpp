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

struct lr1_set_builder
{
    lr1_set_builder(const ruleset& rs)
        :set_(lr1_set_item_comp(rs))
    {}
    
    template <typename... Idx>
    lr1_set_builder& operator()(Idx... indices)
    {
        set_.insert(lr1_set_item(static_cast<size_t>(indices)...));
        return *this;
    }
    
    lr1_set_builder& reset()
    {
        set_.clear();
        return *this;
    }
    
    lr1_set build()
    {
        return set_.get_all();
    }
    
    sorted_grouped_vector<lr1_set_item, lr1_set_item_comp> set_;
};

TEST_CASE("parse_table_generator basic", "[parse_table_generator]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t a_nterm_idx = sc.add_nterm("A");
    [[maybe_unused]] size_t a_term_idx = sc.add_term("a");
    [[maybe_unused]] size_t root_idx = 0;
    [[maybe_unused]] size_t eof_idx = 0;
    sc.validate();
    
    ruleset rs(sc);
    [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"a"});
    
    rs.validate();
    
    parse_table_generator ptg(rs);
    
    const auto& states = ptg.get_states();
    REQUIRE(states.size() == 3);

    lr1_set_builder builder(rs);
    
    SECTION("states")
    {
        // State 0: kernel {$root -> . A / $eof}, items: that + {A -> . a / $eof}
        auto exp_kernel0 = builder.reset()(root_idx, 0, 0, eof_idx).build();
        REQUIRE(states[0].kernel_matches(exp_kernel0));

        auto exp_items0 = builder.reset()
            (root_idx, 0, 0, eof_idx)
            (a_nterm_idx, a_r0, 0, eof_idx).build();
        REQUIRE(states[0].matches(exp_items0));

        // State 1: kernel {A -> a . / $eof}, items == kernel
        auto exp_kernel1 = builder.reset()(a_nterm_idx, a_r0, 1, eof_idx).build();
        REQUIRE(states[1].kernel_matches(exp_kernel1));
        REQUIRE(states[1].matches(exp_kernel1));

        // State 2: kernel {$root -> A . / $eof}
        auto exp_kernel2 = builder.reset()(root_idx, 0, 1, eof_idx).build();
        REQUIRE(states[2].kernel_matches(exp_kernel2));
        REQUIRE(states[2].matches(exp_kernel2));
    }
    
    SECTION("hints")
    {
        const auto& hints = ptg.get_table_entry_hints();
        REQUIRE(hints.size() == 4);

        REQUIRE(hints[0] == teh(0, {st::terminal, a_term_idx}, pte::shift(1)));
        REQUIRE(hints[1] == teh(0, {st::non_terminal, a_nterm_idx}, pte::shift(2)));
        REQUIRE(hints[2] == teh(1, {st::terminal, eof_idx}, pte::reduce(a_nterm_idx, 1)));
        REQUIRE(hints[3] == teh(2, {st::terminal, eof_idx}, pte::reduce(root_idx, 1)));
        
        const auto& rr_hints = ptg.get_rr_conflict_hints();
        REQUIRE(rr_hints.size() == 0);
        
        SECTION("tables")
        {
            auto pt = ptg.create_parse_table();
            REQUIRE(pt.get_state_count() == 3);
            REQUIRE(pt.get_term_entry(0, a_term_idx).get_shift_state_idx() == 1);
            REQUIRE(pt.get_nterm_entry(0, a_nterm_idx).get_shift_state_idx() == 2);
            REQUIRE(pt.get_term_entry(1, eof_idx).get_reduce_nterm_idx() == a_nterm_idx);
            REQUIRE(pt.get_term_entry(1, eof_idx).get_reduce_length() == 1);
            REQUIRE(pt.get_term_entry(2, eof_idx).get_reduce_nterm_idx() == root_idx);
            REQUIRE(pt.get_term_entry(2, eof_idx).get_reduce_length() == 1);
        }
    }
}

TEST_CASE("parse_table_generator rr conflict unresolved", "[parse_table_generator]")
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

    const auto& states = ptg.get_states();
    REQUIRE(states.size() == 5);

    lr1_set_builder builder(rs);

    SECTION("states")
    {
        // State 0: kernel {$root -> . S / $eof}, items: {A -> . a / $eof, B -> . a / $eof, $root -> . S / $eof, S -> . A / $eof, S -> . B / $eof}
        auto exp_kernel0 = builder.reset()(root_idx, 0, 0, eof_idx).build();
        REQUIRE(states[0].kernel_matches(exp_kernel0));

        auto exp_items0 = builder.reset()
            (a_nterm_idx, a_r0, 0, eof_idx)
            (b_nterm_idx, b_r0, 0, eof_idx)
            (root_idx, 0, 0, eof_idx)
            (s_idx, s_r0, 0, eof_idx)
            (s_idx, s_r1, 0, eof_idx).build();
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

        REQUIRE(hints[0] == teh(0, {st::terminal, a_term_idx}, pte::shift(1)));
        REQUIRE(hints[1] == teh(0, {st::non_terminal, s_idx}, pte::shift(2)));
        REQUIRE(hints[2] == teh(0, {st::non_terminal, a_nterm_idx}, pte::shift(3)));
        REQUIRE(hints[3] == teh(0, {st::non_terminal, b_nterm_idx}, pte::shift(4)));
        REQUIRE(hints[4] == teh(1, {st::terminal, eof_idx}, pte::rr_conflict(0, 2)));
        REQUIRE(hints[5] == teh(2, {st::terminal, eof_idx}, pte::reduce(root_idx, 1)));
        REQUIRE(hints[6] == teh(3, {st::terminal, eof_idx}, pte::reduce(s_idx, 1)));
        REQUIRE(hints[7] == teh(4, {st::terminal, eof_idx}, pte::reduce(s_idx, 1)));
        
        const auto& rr_hints = ptg.get_rr_conflict_hints();
        REQUIRE(rr_hints.size() == 2);
        REQUIRE(rr_hints[0] == action::reduction{a_nterm_idx, a_r0});
        REQUIRE(rr_hints[1] == action::reduction{b_nterm_idx, b_r0});
        
        SECTION("tables")
        {
            auto pt = ptg.create_parse_table();
            REQUIRE(pt.get_state_count() == 5);
            REQUIRE(pt.get_term_entry(0, a_term_idx).get_shift_state_idx() == 1);
            REQUIRE(pt.get_nterm_entry(0, s_idx).get_shift_state_idx() == 2);
            REQUIRE(pt.get_nterm_entry(0, a_nterm_idx).get_shift_state_idx() == 3);
            REQUIRE(pt.get_nterm_entry(0, b_nterm_idx).get_shift_state_idx() == 4);
            REQUIRE(pt.get_term_entry(1, eof_idx).get_rr_conflict_start_idx() == 0);
            REQUIRE(pt.get_term_entry(1, eof_idx).get_rr_conflict_count() == 2);
            REQUIRE(pt.get_term_entry(2, eof_idx).get_reduce_nterm_idx() == root_idx);
            REQUIRE(pt.get_term_entry(2, eof_idx).get_reduce_length() == 1);
            REQUIRE(pt.get_term_entry(3, eof_idx).get_reduce_nterm_idx() == s_idx);
            REQUIRE(pt.get_term_entry(3, eof_idx).get_reduce_length() == 1);
            REQUIRE(pt.get_term_entry(4, eof_idx).get_reduce_nterm_idx() == s_idx);
            REQUIRE(pt.get_term_entry(4, eof_idx).get_reduce_length() == 1);

            auto rr = ptg.create_rr_table();
            REQUIRE(rr.size() == 2);
            REQUIRE(rr[0].nterm_idx_ == static_cast<uint16_t>(a_nterm_idx));
            REQUIRE(rr[0].length_ == 1);
            REQUIRE(rr[1].nterm_idx_ == static_cast<uint16_t>(b_nterm_idx));
            REQUIRE(rr[1].length_ == 1);
        }
    }

    SECTION("warnings")
    {
        const auto& warnings = ptg.get_warnings();
        REQUIRE(warnings.size() == 4);
        REQUIRE(warnings[0] == "Conflict in state 1 on lookahead '$eof' :");
        REQUIRE(warnings[1] == "\n    A -> a . / $eof");
        REQUIRE(warnings[2] == "\n    B -> a . / $eof");
        REQUIRE(warnings[3] == "\nConflict in state 1 on lookahead '$eof' unresolved. Will resort to GLR parsing");
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
        REQUIRE(warnings[3] == "\nConflict in state 1 on lookahead '$eof' resolved");
    }

    SECTION("states")
    {
        const auto& states = ptg.get_states();
        REQUIRE(states.size() == 5);

        lr1_set_builder builder(rs);

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

        REQUIRE(hints[0] == teh(0, {st::terminal, a_term_idx}, pte::shift(1)));
        REQUIRE(hints[1] == teh(0, {st::non_terminal, s_idx}, pte::shift(2)));
        REQUIRE(hints[2] == teh(0, {st::non_terminal, a_nterm_idx}, pte::shift(3)));
        REQUIRE(hints[3] == teh(0, {st::non_terminal, b_nterm_idx}, pte::shift(4)));
        REQUIRE(hints[4] == teh(1, {st::terminal, eof_idx}, pte::reduce(a_nterm_idx, 1)));
        REQUIRE(hints[5] == teh(2, {st::terminal, eof_idx}, pte::reduce(root_idx, 1)));
        REQUIRE(hints[6] == teh(3, {st::terminal, eof_idx}, pte::reduce(s_idx, 1)));
        REQUIRE(hints[7] == teh(4, {st::terminal, eof_idx}, pte::reduce(s_idx, 1)));
        
        SECTION("tables")
        {
            auto pt = ptg.create_parse_table();
            REQUIRE(pt.get_state_count() == 5);
            REQUIRE(pt.get_term_entry(0, a_term_idx).get_shift_state_idx() == 1);
            REQUIRE(pt.get_nterm_entry(0, s_idx).get_shift_state_idx() == 2);
            REQUIRE(pt.get_nterm_entry(0, a_nterm_idx).get_shift_state_idx() == 3);
            REQUIRE(pt.get_nterm_entry(0, b_nterm_idx).get_shift_state_idx() == 4);
            REQUIRE(pt.get_term_entry(1, eof_idx).get_reduce_nterm_idx() == a_nterm_idx);
            REQUIRE(pt.get_term_entry(1, eof_idx).get_reduce_length() == 1);
            REQUIRE(pt.get_term_entry(2, eof_idx).get_reduce_nterm_idx() == root_idx);
            REQUIRE(pt.get_term_entry(2, eof_idx).get_reduce_length() == 1);
            REQUIRE(pt.get_term_entry(3, eof_idx).get_reduce_nterm_idx() == s_idx);
            REQUIRE(pt.get_term_entry(3, eof_idx).get_reduce_length() == 1);
            REQUIRE(pt.get_term_entry(4, eof_idx).get_reduce_nterm_idx() == s_idx);
            REQUIRE(pt.get_term_entry(4, eof_idx).get_reduce_length() == 1);
            
            auto rr = ptg.create_rr_table();
            REQUIRE(rr.size() == 0);
        }
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

    const auto& states = ptg.get_states();
    REQUIRE(states.size() == 10);

    lr1_set_builder builder(rs);

    SECTION("states")
    {
        // State 0: kernel {$root -> . S / $eof}, items: {A -> . a / b, B -> . a / b, C -> . a b / b, $root -> . S / $eof, S -> . A b / $eof, S -> . B b / $eof, S -> . C b / $eof}
        auto exp_kernel0 = builder.reset()(root_idx, 0, 0, eof_idx).build();
        REQUIRE(states[0].kernel_matches(exp_kernel0));

        auto exp_items0 = builder.reset()
            (a_nterm_idx, a_r0, 0, b_term_idx)
            (b_nterm_idx, b_r0, 0, b_term_idx)
            (c_nterm_idx, c_r0, 0, b_term_idx)
            (root_idx, 0, 0, eof_idx)
            (s_idx, s_r0, 0, eof_idx)
            (s_idx, s_r1, 0, eof_idx)
            (s_idx, s_r2, 0, eof_idx).build();
        REQUIRE(states[0].matches(exp_items0));

        // State 1: kernel {A -> a . / b, B -> a . / b, C -> a . b / b}
        auto exp_kernel1 = builder.reset()
            (a_nterm_idx, a_r0, 1, b_term_idx)
            (b_nterm_idx, b_r0, 1, b_term_idx)
            (c_nterm_idx, c_r0, 1, b_term_idx).build();
        REQUIRE(states[1].kernel_matches(exp_kernel1));

        REQUIRE(states[1].matches(exp_kernel1));

        // State 2: kernel {$root -> S . / $eof}
        auto exp_kernel2 = builder.reset()(root_idx, 0, 1, eof_idx).build();
        REQUIRE(states[2].kernel_matches(exp_kernel2));
        REQUIRE(states[2].matches(exp_kernel2));

        // State 3: kernel {S -> A . b / $eof}
        auto exp_kernel3 = builder.reset()(s_idx, s_r0, 1, eof_idx).build();
        REQUIRE(states[3].kernel_matches(exp_kernel3));
        REQUIRE(states[3].matches(exp_kernel3));

        // State 4: kernel {S -> B . b / $eof}
        auto exp_kernel4 = builder.reset()(s_idx, s_r1, 1, eof_idx).build();
        REQUIRE(states[4].kernel_matches(exp_kernel4));
        REQUIRE(states[4].matches(exp_kernel4));

        // State 5: kernel {S -> C . b / $eof}
        auto exp_kernel5 = builder.reset()(s_idx, s_r2, 1, eof_idx).build();
        REQUIRE(states[5].kernel_matches(exp_kernel5));
        REQUIRE(states[5].matches(exp_kernel5));

        // State 6: kernel {C -> a b . / b}
        auto exp_kernel6 = builder.reset()(c_nterm_idx, c_r0, 2, b_term_idx).build();
        REQUIRE(states[6].kernel_matches(exp_kernel6));
        REQUIRE(states[6].matches(exp_kernel6));

        // State 7: kernel {S -> A b . / $eof}
        auto exp_kernel7 = builder.reset()(s_idx, s_r0, 2, eof_idx).build();
        REQUIRE(states[7].kernel_matches(exp_kernel7));
        REQUIRE(states[7].matches(exp_kernel7));

        // State 8: kernel {S -> B b . / $eof}
        auto exp_kernel8 = builder.reset()(s_idx, s_r1, 2, eof_idx).build();
        REQUIRE(states[8].kernel_matches(exp_kernel8));
        REQUIRE(states[8].matches(exp_kernel8));

        // State 9: kernel {S -> C b . / $eof}
        auto exp_kernel9 = builder.reset()(s_idx, s_r2, 2, eof_idx).build();
        REQUIRE(states[9].kernel_matches(exp_kernel9));
        REQUIRE(states[9].matches(exp_kernel9));
    }

    SECTION("hints")
    {
        const auto& hints = ptg.get_table_entry_hints();
        REQUIRE(hints.size() == 14);

        REQUIRE(hints[0] == teh(0, {st::terminal, a_term_idx}, pte::shift(1)));
        REQUIRE(hints[1] == teh(0, {st::non_terminal, s_idx}, pte::shift(2)));
        REQUIRE(hints[2] == teh(0, {st::non_terminal, a_nterm_idx}, pte::shift(3)));
        REQUIRE(hints[3] == teh(0, {st::non_terminal, b_nterm_idx}, pte::shift(4)));
        REQUIRE(hints[4] == teh(0, {st::non_terminal, c_nterm_idx}, pte::shift(5)));
        REQUIRE(hints[5] == teh(1, {st::terminal, b_term_idx}, pte::shift(6)));
        REQUIRE(hints[6] == teh(2, {st::terminal, eof_idx}, pte::reduce(root_idx, 1)));
        REQUIRE(hints[7] == teh(3, {st::terminal, b_term_idx}, pte::shift(7)));
        REQUIRE(hints[8] == teh(4, {st::terminal, b_term_idx}, pte::shift(8)));
        REQUIRE(hints[9] == teh(5, {st::terminal, b_term_idx}, pte::shift(9)));
        REQUIRE(hints[10] == teh(6, {st::terminal, b_term_idx}, pte::reduce(c_nterm_idx, 2)));
        REQUIRE(hints[11] == teh(7, {st::terminal, eof_idx}, pte::reduce(s_idx, 2)));
        REQUIRE(hints[12] == teh(8, {st::terminal, eof_idx}, pte::reduce(s_idx, 2)));
        REQUIRE(hints[13] == teh(9, {st::terminal, eof_idx}, pte::reduce(s_idx, 2)));
        
        const auto& rr_hints = ptg.get_rr_conflict_hints();
        REQUIRE(rr_hints.size() == 0);
        
        SECTION("tables")
        {
            auto pt = ptg.create_parse_table();
            REQUIRE(pt.get_state_count() == 10);
            REQUIRE(pt.get_term_entry(0, a_term_idx).get_shift_state_idx() == 1);
            REQUIRE(pt.get_nterm_entry(0, s_idx).get_shift_state_idx() == 2);
            REQUIRE(pt.get_nterm_entry(0, a_nterm_idx).get_shift_state_idx() == 3);
            REQUIRE(pt.get_nterm_entry(0, b_nterm_idx).get_shift_state_idx() == 4);
            REQUIRE(pt.get_nterm_entry(0, c_nterm_idx).get_shift_state_idx() == 5);
            REQUIRE(pt.get_term_entry(1, b_term_idx).get_shift_state_idx() == 6);
            REQUIRE(pt.get_term_entry(2, eof_idx).get_reduce_nterm_idx() == root_idx);
            REQUIRE(pt.get_term_entry(2, eof_idx).get_reduce_length() == 1);
            REQUIRE(pt.get_term_entry(3, b_term_idx).get_shift_state_idx() == 7);
            REQUIRE(pt.get_term_entry(4, b_term_idx).get_shift_state_idx() == 8);
            REQUIRE(pt.get_term_entry(5, b_term_idx).get_shift_state_idx() == 9);
            REQUIRE(pt.get_term_entry(6, b_term_idx).get_reduce_nterm_idx() == c_nterm_idx);
            REQUIRE(pt.get_term_entry(6, b_term_idx).get_reduce_length() == 2);
            REQUIRE(pt.get_term_entry(7, eof_idx).get_reduce_nterm_idx() == s_idx);
            REQUIRE(pt.get_term_entry(7, eof_idx).get_reduce_length() == 2);
            REQUIRE(pt.get_term_entry(8, eof_idx).get_reduce_nterm_idx() == s_idx);
            REQUIRE(pt.get_term_entry(8, eof_idx).get_reduce_length() == 2);
            REQUIRE(pt.get_term_entry(9, eof_idx).get_reduce_nterm_idx() == s_idx);
            REQUIRE(pt.get_term_entry(9, eof_idx).get_reduce_length() == 2);
            
            auto rr = ptg.create_rr_table();
            REQUIRE(rr.size() == 0);
        }
    }

    SECTION("warnings")
    {
        const auto& warnings = ptg.get_warnings();
        std::string s;
        for (auto warning : warnings)
            s.append(warning);
        INFO(s);
        REQUIRE(warnings.size() == 5);
        REQUIRE(warnings[0] == "Conflict in state 1 on lookahead 'b' :");
        REQUIRE(warnings[1] == "\n    A -> a . / b");
        REQUIRE(warnings[2] == "\n    B -> a . / b");
        REQUIRE(warnings[3] == "\n    shift on 'b' to state 6 has the highest precedence");
        REQUIRE(warnings[4] == "\nConflict in state 1 on lookahead 'b' resolved");
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

    const auto& states = ptg.get_states();
    REQUIRE(states.size() == 9);

    lr1_set_builder builder(rs);

    SECTION("states")
    {
        // State 0: kernel {$root -> . S / $eof}, items: {A -> . a / b, B -> . a / b, C -> . a b / b, $root -> . S / $eof, S -> . A b / $eof, S -> . B b / $eof, S -> . C b / $eof}
        auto exp_kernel0 = builder.reset()(root_idx, 0, 0, eof_idx).build();
        REQUIRE(states[0].kernel_matches(exp_kernel0));

        auto exp_items0 = builder.reset()
            (a_nterm_idx, a_r0, 0, b_term_idx)
            (b_nterm_idx, b_r0, 0, b_term_idx)
            (c_nterm_idx, c_r0, 0, b_term_idx)
            (root_idx, 0, 0, eof_idx)
            (s_idx, s_r0, 0, eof_idx)
            (s_idx, s_r1, 0, eof_idx)
            (s_idx, s_r2, 0, eof_idx).build();
        REQUIRE(states[0].matches(exp_items0));

        // State 1: kernel {A -> a . / b, B -> a . / b, C -> a . b / b}
        auto exp_kernel1 = builder.reset()
            (a_nterm_idx, a_r0, 1, b_term_idx)
            (b_nterm_idx, b_r0, 1, b_term_idx)
            (c_nterm_idx, c_r0, 1, b_term_idx).build();
        REQUIRE(states[1].kernel_matches(exp_kernel1));

        REQUIRE(states[1].matches(exp_kernel1));

        // State 2: kernel {$root -> S . / $eof}
        auto exp_kernel2 = builder.reset()(root_idx, 0, 1, eof_idx).build();
        REQUIRE(states[2].kernel_matches(exp_kernel2));
        REQUIRE(states[2].matches(exp_kernel2));

        // State 3: kernel {S -> A . b / $eof}
        auto exp_kernel3 = builder.reset()(s_idx, s_r0, 1, eof_idx).build();
        REQUIRE(states[3].kernel_matches(exp_kernel3));
        REQUIRE(states[3].matches(exp_kernel3));

        // State 4: kernel {S -> B . b / $eof}
        auto exp_kernel4 = builder.reset()(s_idx, s_r1, 1, eof_idx).build();
        REQUIRE(states[4].kernel_matches(exp_kernel4));
        REQUIRE(states[4].matches(exp_kernel4));

        // State 5: kernel {S -> C . b / $eof}
        auto exp_kernel5 = builder.reset()(s_idx, s_r2, 1, eof_idx).build();
        REQUIRE(states[5].kernel_matches(exp_kernel5));
        REQUIRE(states[5].matches(exp_kernel5));

        // State 6: kernel {S -> A b . / $eof}
        auto exp_kernel6 = builder.reset()(s_idx, s_r0, 2, eof_idx).build();
        REQUIRE(states[6].kernel_matches(exp_kernel6));
        REQUIRE(states[6].matches(exp_kernel6));

        // State 7: kernel {S -> B b . / $eof}
        auto exp_kernel7 = builder.reset()(s_idx, s_r1, 2, eof_idx).build();
        REQUIRE(states[7].kernel_matches(exp_kernel7));
        REQUIRE(states[7].matches(exp_kernel7));

        // State 8: kernel {S -> C b . / $eof}
        auto exp_kernel8 = builder.reset()(s_idx, s_r2, 2, eof_idx).build();
        REQUIRE(states[8].kernel_matches(exp_kernel8));
        REQUIRE(states[8].matches(exp_kernel8));
    }

    SECTION("hints")
    {
        const auto& hints = ptg.get_table_entry_hints();
        REQUIRE(hints.size() == 13);

        REQUIRE(hints[0] == teh(0, {st::terminal, a_term_idx}, pte::shift(1)));
        REQUIRE(hints[1] == teh(0, {st::non_terminal, s_idx}, pte::shift(2)));
        REQUIRE(hints[2] == teh(0, {st::non_terminal, a_nterm_idx}, pte::shift(3)));
        REQUIRE(hints[3] == teh(0, {st::non_terminal, b_nterm_idx}, pte::shift(4)));
        REQUIRE(hints[4] == teh(0, {st::non_terminal, c_nterm_idx}, pte::shift(5)));
        REQUIRE(hints[5] == teh(1, {st::terminal, b_term_idx}, pte::reduce(a_nterm_idx, 1)));
        REQUIRE(hints[6] == teh(2, {st::terminal, eof_idx}, pte::reduce(root_idx, 1)));
        REQUIRE(hints[7] == teh(3, {st::terminal, b_term_idx}, pte::shift(6)));
        REQUIRE(hints[8] == teh(4, {st::terminal, b_term_idx}, pte::shift(7)));
        REQUIRE(hints[9] == teh(5, {st::terminal, b_term_idx}, pte::shift(8)));
        REQUIRE(hints[10] == teh(6, {st::terminal, eof_idx}, pte::reduce(s_idx, 2)));
        REQUIRE(hints[11] == teh(7, {st::terminal, eof_idx}, pte::reduce(s_idx, 2)));
        REQUIRE(hints[12] == teh(8, {st::terminal, eof_idx}, pte::reduce(s_idx, 2)));
        
        const auto& rr_hints = ptg.get_rr_conflict_hints();
        REQUIRE(rr_hints.size() == 0);
        
        SECTION("tables")
        {
            auto pt = ptg.create_parse_table();
            REQUIRE(pt.get_state_count() == 9);
            REQUIRE(pt.get_term_entry(0, a_term_idx).get_shift_state_idx() == 1);
            REQUIRE(pt.get_nterm_entry(0, s_idx).get_shift_state_idx() == 2);
            REQUIRE(pt.get_nterm_entry(0, a_nterm_idx).get_shift_state_idx() == 3);
            REQUIRE(pt.get_nterm_entry(0, b_nterm_idx).get_shift_state_idx() == 4);
            REQUIRE(pt.get_nterm_entry(0, c_nterm_idx).get_shift_state_idx() == 5);
            REQUIRE(pt.get_term_entry(1, b_term_idx).get_reduce_nterm_idx() == a_nterm_idx);
            REQUIRE(pt.get_term_entry(1, b_term_idx).get_reduce_length() == 1);
            REQUIRE(pt.get_term_entry(2, eof_idx).get_reduce_nterm_idx() == root_idx);
            REQUIRE(pt.get_term_entry(2, eof_idx).get_reduce_length() == 1);
            REQUIRE(pt.get_term_entry(3, b_term_idx).get_shift_state_idx() == 6);
            REQUIRE(pt.get_term_entry(4, b_term_idx).get_shift_state_idx() == 7);
            REQUIRE(pt.get_term_entry(5, b_term_idx).get_shift_state_idx() == 8);
            REQUIRE(pt.get_term_entry(6, eof_idx).get_reduce_nterm_idx() == s_idx);
            REQUIRE(pt.get_term_entry(6, eof_idx).get_reduce_length() == 2);
            REQUIRE(pt.get_term_entry(7, eof_idx).get_reduce_nterm_idx() == s_idx);
            REQUIRE(pt.get_term_entry(7, eof_idx).get_reduce_length() == 2);
            REQUIRE(pt.get_term_entry(8, eof_idx).get_reduce_nterm_idx() == s_idx);
            REQUIRE(pt.get_term_entry(8, eof_idx).get_reduce_length() == 2);

            auto rr = ptg.create_rr_table();
            REQUIRE(rr.size() == 0);
        }
    }

    SECTION("warnings")
    {
        const auto& warnings = ptg.get_warnings();
        REQUIRE(warnings.size() == 5);
        REQUIRE(warnings[0] == "Conflict in state 1 on lookahead 'b' :");
        REQUIRE(warnings[1] == "\n    A -> a . / b (highest precedence)");
        REQUIRE(warnings[2] == "\n    B -> a . / b");
        REQUIRE(warnings[3] == "\n    shift on 'b'");
        REQUIRE(warnings[4] == "\nConflict in state 1 on lookahead 'b' resolved");
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

    const auto& states = ptg.get_states();
    REQUIRE(states.size() == 9);

    lr1_set_builder builder(rs);

    SECTION("states")
    {
        // State 0: kernel {$root -> . S / $eof}, items: {A -> . a / b, B -> . a / b, C -> . a b / b, $root -> . S / $eof, S -> . A b / $eof, S -> . B b / $eof, S -> . C b / $eof}
        auto exp_kernel0 = builder.reset()(root_idx, 0, 0, eof_idx).build();
        REQUIRE(states[0].kernel_matches(exp_kernel0));

        auto exp_items0 = builder.reset()
            (a_nterm_idx, a_r0, 0, b_term_idx)
            (b_nterm_idx, b_r0, 0, b_term_idx)
            (c_nterm_idx, c_r0, 0, b_term_idx)
            (root_idx, 0, 0, eof_idx)
            (s_idx, s_r0, 0, eof_idx)
            (s_idx, s_r1, 0, eof_idx)
            (s_idx, s_r2, 0, eof_idx).build();
        REQUIRE(states[0].matches(exp_items0));

        // State 1: kernel {A -> a . / b, B -> a . / b, C -> a . b / b}
        auto exp_kernel1 = builder.reset()
            (a_nterm_idx, a_r0, 1, b_term_idx)
            (b_nterm_idx, b_r0, 1, b_term_idx)
            (c_nterm_idx, c_r0, 1, b_term_idx).build();
        REQUIRE(states[1].kernel_matches(exp_kernel1));

        REQUIRE(states[1].matches(exp_kernel1));

        // State 2: kernel {$root -> S . / $eof}
        auto exp_kernel2 = builder.reset()(root_idx, 0, 1, eof_idx).build();
        REQUIRE(states[2].kernel_matches(exp_kernel2));
        REQUIRE(states[2].matches(exp_kernel2));

        // State 3: kernel {S -> A . b / $eof}
        auto exp_kernel3 = builder.reset()(s_idx, s_r0, 1, eof_idx).build();
        REQUIRE(states[3].kernel_matches(exp_kernel3));
        REQUIRE(states[3].matches(exp_kernel3));

        // State 4: kernel {S -> B . b / $eof}
        auto exp_kernel4 = builder.reset()(s_idx, s_r1, 1, eof_idx).build();
        REQUIRE(states[4].kernel_matches(exp_kernel4));
        REQUIRE(states[4].matches(exp_kernel4));

        // State 5: kernel {S -> C . b / $eof}
        auto exp_kernel5 = builder.reset()(s_idx, s_r2, 1, eof_idx).build();
        REQUIRE(states[5].kernel_matches(exp_kernel5));
        REQUIRE(states[5].matches(exp_kernel5));

        // State 6: kernel {S -> A b . / $eof}
        auto exp_kernel6 = builder.reset()(s_idx, s_r0, 2, eof_idx).build();
        REQUIRE(states[6].kernel_matches(exp_kernel6));
        REQUIRE(states[6].matches(exp_kernel6));

        // State 7: kernel {S -> B b . / $eof}
        auto exp_kernel7 = builder.reset()(s_idx, s_r1, 2, eof_idx).build();
        REQUIRE(states[7].kernel_matches(exp_kernel7));
        REQUIRE(states[7].matches(exp_kernel7));

        // State 8: kernel {S -> C b . / $eof}
        auto exp_kernel8 = builder.reset()(s_idx, s_r2, 2, eof_idx).build();
        REQUIRE(states[8].kernel_matches(exp_kernel8));
        REQUIRE(states[8].matches(exp_kernel8));
    }

    SECTION("hints")
    {
        const auto& hints = ptg.get_table_entry_hints();
        REQUIRE(hints.size() == 13);

        REQUIRE(hints[0] == teh(0, {st::terminal, a_term_idx}, pte::shift(1)));
        REQUIRE(hints[1] == teh(0, {st::non_terminal, s_idx}, pte::shift(2)));
        REQUIRE(hints[2] == teh(0, {st::non_terminal, a_nterm_idx}, pte::shift(3)));
        REQUIRE(hints[3] == teh(0, {st::non_terminal, b_nterm_idx}, pte::shift(4)));
        REQUIRE(hints[4] == teh(0, {st::non_terminal, c_nterm_idx}, pte::shift(5)));
        REQUIRE(hints[5] == teh(1, {st::terminal, b_term_idx}, pte::rr_conflict(0, 2)));
        REQUIRE(hints[6] == teh(2, {st::terminal, eof_idx}, pte::reduce(root_idx, 1)));
        REQUIRE(hints[7] == teh(3, {st::terminal, b_term_idx}, pte::shift(6)));
        REQUIRE(hints[8] == teh(4, {st::terminal, b_term_idx}, pte::shift(7)));
        REQUIRE(hints[9] == teh(5, {st::terminal, b_term_idx}, pte::shift(8)));
        REQUIRE(hints[10] == teh(6, {st::terminal, eof_idx}, pte::reduce(s_idx, 2)));
        REQUIRE(hints[11] == teh(7, {st::terminal, eof_idx}, pte::reduce(s_idx, 2)));
        REQUIRE(hints[12] == teh(8, {st::terminal, eof_idx}, pte::reduce(s_idx, 2)));
        
        const auto& rr_hints = ptg.get_rr_conflict_hints();
        REQUIRE(rr_hints.size() == 2);
        REQUIRE(rr_hints[0] == action::reduction{a_nterm_idx, a_r0});
        REQUIRE(rr_hints[1] == action::reduction{b_nterm_idx, b_r0});
        
        SECTION("tables")
        {
            auto pt = ptg.create_parse_table();
            REQUIRE(pt.get_state_count() == 9);
            REQUIRE(pt.get_term_entry(0, a_term_idx).get_shift_state_idx() == 1);
            REQUIRE(pt.get_nterm_entry(0, s_idx).get_shift_state_idx() == 2);
            REQUIRE(pt.get_nterm_entry(0, a_nterm_idx).get_shift_state_idx() == 3);
            REQUIRE(pt.get_nterm_entry(0, b_nterm_idx).get_shift_state_idx() == 4);
            REQUIRE(pt.get_nterm_entry(0, c_nterm_idx).get_shift_state_idx() == 5);
            REQUIRE(pt.get_term_entry(1, b_term_idx).get_rr_conflict_start_idx() == 0);
            REQUIRE(pt.get_term_entry(1, b_term_idx).get_rr_conflict_count() == 2);
            REQUIRE(pt.get_term_entry(2, eof_idx).get_reduce_nterm_idx() == root_idx);
            REQUIRE(pt.get_term_entry(2, eof_idx).get_reduce_length() == 1);
            REQUIRE(pt.get_term_entry(3, b_term_idx).get_shift_state_idx() == 6);
            REQUIRE(pt.get_term_entry(4, b_term_idx).get_shift_state_idx() == 7);
            REQUIRE(pt.get_term_entry(5, b_term_idx).get_shift_state_idx() == 8);
            REQUIRE(pt.get_term_entry(6, eof_idx).get_reduce_nterm_idx() == s_idx);
            REQUIRE(pt.get_term_entry(6, eof_idx).get_reduce_length() == 2);
            REQUIRE(pt.get_term_entry(7, eof_idx).get_reduce_nterm_idx() == s_idx);
            REQUIRE(pt.get_term_entry(7, eof_idx).get_reduce_length() == 2);
            REQUIRE(pt.get_term_entry(8, eof_idx).get_reduce_nterm_idx() == s_idx);
            REQUIRE(pt.get_term_entry(8, eof_idx).get_reduce_length() == 2);

            auto rr = ptg.create_rr_table();
            REQUIRE(rr.size() == 2);
            REQUIRE(rr[0].nterm_idx_ == static_cast<uint16_t>(a_nterm_idx));
            REQUIRE(rr[0].length_ == 1);
            REQUIRE(rr[1].nterm_idx_ == static_cast<uint16_t>(b_nterm_idx));
            REQUIRE(rr[1].length_ == 1);
        }
    }

    SECTION("warnings")
    {
        const auto& warnings = ptg.get_warnings();
        REQUIRE(warnings.size() == 5);
        REQUIRE(warnings[0] == "Conflict in state 1 on lookahead 'b' :");
        REQUIRE(warnings[1] == "\n    A -> a . / b");
        REQUIRE(warnings[2] == "\n    B -> a . / b");
        REQUIRE(warnings[3] == "\n    shift on 'b'");
        REQUIRE(warnings[4] == "\nConflict in state 1 on lookahead 'b' unresolved. Will resort to GLR parsing");
    }
}

TEST_CASE("parse_table_generator multiple rr conflict unresolved", "[parse_table_generator]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t root_idx = 0;
    [[maybe_unused]] size_t eof_idx = 0;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_nterm_idx = sc.add_nterm("A");
    [[maybe_unused]] size_t b_nterm_idx = sc.add_nterm("B");
    [[maybe_unused]] size_t c_nterm_idx = sc.add_nterm("C");
    [[maybe_unused]] size_t a_term_idx = sc.add_term("a");

    sc.validate();
    
    ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"A"});
    [[maybe_unused]] size_t s_r1 = rs.add_rule("S", {"B"});
    [[maybe_unused]] size_t s_r2 = rs.add_rule("S", {"C"});
    [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"a"});
    [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"a"});
    [[maybe_unused]] size_t c_r0 = rs.add_rule("C", {"a"});

    rs.validate();
    
    parse_table_generator ptg(rs);

    const auto& states = ptg.get_states();
    REQUIRE(states.size() == 6);

    lr1_set_builder builder(rs);

    SECTION("states")
    {
        // State 0: kernel {$root -> . S / $eof}, items: {A -> . a / $eof, B -> . a / $eof, C -> . a / $eof, $root -> . S / $eof, S -> . A / $eof, S -> . B / $eof, S -> . C / $eof}
        auto exp_kernel0 = builder.reset()(root_idx, 0, 0, eof_idx).build();
        REQUIRE(states[0].kernel_matches(exp_kernel0));

        auto exp_items0 = builder.reset()
            (a_nterm_idx, a_r0, 0, eof_idx)
            (b_nterm_idx, b_r0, 0, eof_idx)
            (c_nterm_idx, c_r0, 0, eof_idx)
            (root_idx, 0, 0, eof_idx)
            (s_idx, s_r0, 0, eof_idx)
            (s_idx, s_r1, 0, eof_idx)
            (s_idx, s_r2, 0, eof_idx).build();
        REQUIRE(states[0].matches(exp_items0));

        // State 1: kernel {A -> a . / $eof, B -> a . / $eof, C -> a . / $eof}
        auto exp_kernel1 = builder.reset()
            (a_nterm_idx, a_r0, 1, eof_idx)
            (b_nterm_idx, b_r0, 1, eof_idx)
            (c_nterm_idx, c_r0, 1, eof_idx).build();
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

        // State 5: kernel {S -> C . / $eof}
        auto exp_kernel5 = builder.reset()(s_idx, s_r2, 1, eof_idx).build();
        REQUIRE(states[5].kernel_matches(exp_kernel5));
        REQUIRE(states[5].matches(exp_kernel5));
    }

    SECTION("hints")
    {
        const auto& hints = ptg.get_table_entry_hints();
        REQUIRE(hints.size() == 10);

        CHECK(hints[0] == teh(0, {st::terminal, a_term_idx}, pte::shift(1)));
        CHECK(hints[1] == teh(0, {st::non_terminal, s_idx}, pte::shift(2)));
        CHECK(hints[2] == teh(0, {st::non_terminal, a_nterm_idx}, pte::shift(3)));
        CHECK(hints[3] == teh(0, {st::non_terminal, b_nterm_idx}, pte::shift(4)));
        CHECK(hints[4] == teh(0, {st::non_terminal, c_nterm_idx}, pte::shift(5)));
        CHECK(hints[5] == teh(1, {st::terminal, eof_idx}, pte::rr_conflict(0, 3)));
        CHECK(hints[6] == teh(2, {st::terminal, eof_idx}, pte::reduce(root_idx, 1)));
        CHECK(hints[7] == teh(3, {st::terminal, eof_idx}, pte::reduce(s_idx, 1)));
        CHECK(hints[8] == teh(4, {st::terminal, eof_idx}, pte::reduce(s_idx, 1)));
        CHECK(hints[9] == teh(5, {st::terminal, eof_idx}, pte::reduce(s_idx, 1)));
        
        const auto& rr_hints = ptg.get_rr_conflict_hints();
        CHECK(rr_hints.size() == 3);
        CHECK(rr_hints[0] == action::reduction{a_nterm_idx, a_r0});
        CHECK(rr_hints[1] == action::reduction{b_nterm_idx, b_r0});
        CHECK(rr_hints[2] == action::reduction{c_nterm_idx, c_r0});
        
        SECTION("tables")
        {
            auto pt = ptg.create_parse_table();
            REQUIRE(pt.get_state_count() == 6);
            REQUIRE(pt.get_term_entry(0, a_term_idx).get_shift_state_idx() == 1);
            REQUIRE(pt.get_nterm_entry(0, s_idx).get_shift_state_idx() == 2);
            REQUIRE(pt.get_nterm_entry(0, a_nterm_idx).get_shift_state_idx() == 3);
            REQUIRE(pt.get_nterm_entry(0, b_nterm_idx).get_shift_state_idx() == 4);
            REQUIRE(pt.get_nterm_entry(0, c_nterm_idx).get_shift_state_idx() == 5);
            REQUIRE(pt.get_term_entry(1, eof_idx).get_rr_conflict_start_idx() == 0);
            REQUIRE(pt.get_term_entry(1, eof_idx).get_rr_conflict_count() == 3);
            REQUIRE(pt.get_term_entry(2, eof_idx).get_reduce_nterm_idx() == root_idx);
            REQUIRE(pt.get_term_entry(2, eof_idx).get_reduce_length() == 1);
            REQUIRE(pt.get_term_entry(3, eof_idx).get_reduce_nterm_idx() == s_idx);
            REQUIRE(pt.get_term_entry(3, eof_idx).get_reduce_length() == 1);
            REQUIRE(pt.get_term_entry(4, eof_idx).get_reduce_nterm_idx() == s_idx);
            REQUIRE(pt.get_term_entry(4, eof_idx).get_reduce_length() == 1);
            REQUIRE(pt.get_term_entry(5, eof_idx).get_reduce_nterm_idx() == s_idx);
            REQUIRE(pt.get_term_entry(5, eof_idx).get_reduce_length() == 1);

            auto rr = ptg.create_rr_table();
            REQUIRE(rr.size() == 3);
            REQUIRE(rr[0].nterm_idx_ == static_cast<uint16_t>(a_nterm_idx));
            REQUIRE(rr[0].length_ == 1);
            REQUIRE(rr[1].nterm_idx_ == static_cast<uint16_t>(b_nterm_idx));
            REQUIRE(rr[1].length_ == 1);
            REQUIRE(rr[2].nterm_idx_ == static_cast<uint16_t>(c_nterm_idx));
            REQUIRE(rr[2].length_ == 1);
        }
    }

    SECTION("warnings")
    {
        const auto& warnings = ptg.get_warnings();
        REQUIRE(warnings.size() == 5);
        REQUIRE(warnings[0] == "Conflict in state 1 on lookahead '$eof' :");
        REQUIRE(warnings[1] == "\n    A -> a . / $eof");
        REQUIRE(warnings[2] == "\n    B -> a . / $eof");
        REQUIRE(warnings[3] == "\n    C -> a . / $eof");
        REQUIRE(warnings[4] == "\nConflict in state 1 on lookahead '$eof' unresolved. Will resort to GLR parsing");
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

    lr1_set_builder builder(rs);

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

        REQUIRE(hints[0] == teh(0, {st::terminal, a_term_idx}, pte::shift(1)));
        REQUIRE(hints[1] == teh(0, {st::terminal, c_term_idx}, pte::shift(2)));
        REQUIRE(hints[2] == teh(0, {st::non_terminal, s_idx}, pte::shift(3)));
        REQUIRE(hints[3] == teh(1, {st::terminal, e_term_idx}, pte::shift(4)));
        REQUIRE(hints[4] == teh(1, {st::non_terminal, a_nterm_idx}, pte::shift(5)));
        REQUIRE(hints[5] == teh(2, {st::terminal, e_term_idx}, pte::shift(6)));
        REQUIRE(hints[6] == teh(2, {st::non_terminal, c_nterm_idx}, pte::shift(7)));
        REQUIRE(hints[7] == teh(3, {st::terminal, eof_idx}, pte::reduce(root_idx, 1)));
        REQUIRE(hints[8] == teh(4, {st::terminal, b_term_idx}, pte::reduce(a_nterm_idx, 1)));
        REQUIRE(hints[9] == teh(5, {st::terminal, b_term_idx}, pte::shift(8)));
        REQUIRE(hints[10] == teh(6, {st::terminal, d_term_idx}, pte::reduce(c_nterm_idx, 1)));
        REQUIRE(hints[11] == teh(7, {st::terminal, d_term_idx}, pte::shift(9)));
        REQUIRE(hints[12] == teh(8, {st::terminal, eof_idx}, pte::reduce(s_idx, 3)));
        REQUIRE(hints[13] == teh(9, {st::terminal, eof_idx}, pte::reduce(s_idx, 3)));
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

    lr1_set_builder builder(rs);

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
        }
        for (auto la : all_la)
        {
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
        }
        for (auto la : all_la)
        {
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
        }
        for (auto la : all_la)
        {
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
        }
        for (auto la : all_la)
        {
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
        REQUIRE(warnings[ 3] == "\nConflict in state 4 on lookahead '+' resolved");
        
        REQUIRE(warnings[ 4] == "Conflict in state 4 on lookahead '-' :");
        REQUIRE(warnings[ 5] == "\n    Expr -> - Expr . / - (highest precedence)");
        REQUIRE(warnings[ 6] == "\n    shift on '-'");
        REQUIRE(warnings[ 7] == "\nConflict in state 4 on lookahead '-' resolved");
        
        REQUIRE(warnings[ 8] == "Conflict in state 4 on lookahead '*' :");
        REQUIRE(warnings[ 9] == "\n    Expr -> - Expr . / * (highest precedence)");
        REQUIRE(warnings[10] == "\n    shift on '*'");
        REQUIRE(warnings[11] == "\nConflict in state 4 on lookahead '*' resolved");
        
        REQUIRE(warnings[12] == "Conflict in state 8 on lookahead '+' :");
        REQUIRE(warnings[13] == "\n    Expr -> Expr + Expr . / + (highest precedence)");
        REQUIRE(warnings[14] == "\n    shift on '+'");
        REQUIRE(warnings[15] == "\nConflict in state 8 on lookahead '+' resolved");
        
        REQUIRE(warnings[16] == "Conflict in state 8 on lookahead '-' :");
        REQUIRE(warnings[17] == "\n    Expr -> Expr + Expr . / - (highest precedence)");
        REQUIRE(warnings[18] == "\n    shift on '-'");
        REQUIRE(warnings[19] == "\nConflict in state 8 on lookahead '-' resolved");
        
        REQUIRE(warnings[20] == "Conflict in state 8 on lookahead '*' :");
        REQUIRE(warnings[21] == "\n    Expr -> Expr + Expr . / *");
        REQUIRE(warnings[22] == "\n    shift on '*' to state 7 has the highest precedence");
        REQUIRE(warnings[23] == "\nConflict in state 8 on lookahead '*' resolved");
        
        REQUIRE(warnings[24] == "Conflict in state 9 on lookahead '+' :");
        REQUIRE(warnings[25] == "\n    Expr -> Expr - Expr . / + (highest precedence)");
        REQUIRE(warnings[26] == "\n    shift on '+'");
        REQUIRE(warnings[27] == "\nConflict in state 9 on lookahead '+' resolved");
        
        REQUIRE(warnings[28] == "Conflict in state 9 on lookahead '-' :");
        REQUIRE(warnings[29] == "\n    Expr -> Expr - Expr . / - (highest precedence)");
        REQUIRE(warnings[30] == "\n    shift on '-'");
        REQUIRE(warnings[31] == "\nConflict in state 9 on lookahead '-' resolved");
        
        REQUIRE(warnings[32] == "Conflict in state 9 on lookahead '*' :");
        REQUIRE(warnings[33] == "\n    Expr -> Expr - Expr . / *");
        REQUIRE(warnings[34] == "\n    shift on '*' to state 7 has the highest precedence");
        REQUIRE(warnings[35] == "\nConflict in state 9 on lookahead '*' resolved");
        
        REQUIRE(warnings[36] == "Conflict in state 10 on lookahead '+' :");
        REQUIRE(warnings[37] == "\n    Expr -> Expr * Expr . / + (highest precedence)");
        REQUIRE(warnings[38] == "\n    shift on '+'");
        REQUIRE(warnings[39] == "\nConflict in state 10 on lookahead '+' resolved");
        
        REQUIRE(warnings[40] == "Conflict in state 10 on lookahead '-' :");
        REQUIRE(warnings[41] == "\n    Expr -> Expr * Expr . / - (highest precedence)");
        REQUIRE(warnings[42] == "\n    shift on '-'");
        REQUIRE(warnings[43] == "\nConflict in state 10 on lookahead '-' resolved");
        
        REQUIRE(warnings[44] == "Conflict in state 10 on lookahead '*' :");
        REQUIRE(warnings[45] == "\n    Expr -> Expr * Expr . / * (highest precedence)");
        REQUIRE(warnings[46] == "\n    shift on '*'");
        REQUIRE(warnings[47] == "\nConflict in state 10 on lookahead '*' resolved");
    }

    SECTION("hints")
    {
        const auto& hints = ptg.get_table_entry_hints();
        REQUIRE(hints.size() == 39);  // As per previous completion

        // State 0 shifts
        REQUIRE(hints[0] == teh(0, {st::terminal, id_idx}, pte::shift(1)));
        REQUIRE(hints[1] == teh(0, {st::terminal, minus_idx}, pte::shift(2)));
        REQUIRE(hints[2] == teh(0, {st::non_terminal, expr_idx}, pte::shift(3)));

        // Reduces from state 1 for all_la
        REQUIRE(hints[3] == teh(1, {st::terminal, all_la[0]}, pte::reduce(expr_idx, 1)));
        REQUIRE(hints[4] == teh(1, {st::terminal, all_la[1]}, pte::reduce(expr_idx, 1)));
        REQUIRE(hints[5] == teh(1, {st::terminal, all_la[2]}, pte::reduce(expr_idx, 1)));
        REQUIRE(hints[6] == teh(1, {st::terminal, all_la[3]}, pte::reduce(expr_idx, 1)));

        // State 2 shifts
        REQUIRE(hints[7] == teh(2, {st::terminal, id_idx}, pte::shift(1)));
        REQUIRE(hints[8] == teh(2, {st::terminal, minus_idx}, pte::shift(2)));
        REQUIRE(hints[9] == teh(2, {st::non_terminal, expr_idx}, pte::shift(4)));

        // State 3 reduce
        REQUIRE(hints[10] == teh(3, {st::terminal, eof_idx}, pte::reduce(root_idx, 1)));

        // State 3 shifts
        REQUIRE(hints[11] == teh(3, {st::terminal, plus_idx}, pte::shift(5)));
        REQUIRE(hints[12] == teh(3, {st::terminal, minus_idx}, pte::shift(6)));
        REQUIRE(hints[13] == teh(3, {st::terminal, mul_idx}, pte::shift(7)));

        // State 4 resolved reduces
        REQUIRE(hints[14] == teh(4, {st::terminal, eof_idx}, pte::reduce(expr_idx, 2)));        
        REQUIRE(hints[15] == teh(4, {st::terminal, plus_idx}, pte::reduce(expr_idx, 2)));
        REQUIRE(hints[16] == teh(4, {st::terminal, minus_idx}, pte::reduce(expr_idx, 2)));
        REQUIRE(hints[17] == teh(4, {st::terminal, mul_idx}, pte::reduce(expr_idx, 2)));

        // State 5 shifts
        REQUIRE(hints[18] == teh(5, {st::terminal, id_idx}, pte::shift(1)));
        REQUIRE(hints[19] == teh(5, {st::terminal, minus_idx}, pte::shift(2)));
        REQUIRE(hints[20] == teh(5, {st::non_terminal, expr_idx}, pte::shift(8)));

        // State 6 shifts
        REQUIRE(hints[21] == teh(6, {st::terminal, id_idx}, pte::shift(1)));
        REQUIRE(hints[22] == teh(6, {st::terminal, minus_idx}, pte::shift(2)));
        REQUIRE(hints[23] == teh(6, {st::non_terminal, expr_idx}, pte::shift(9)));

        // State 7 shifts
        REQUIRE(hints[24] == teh(7, {st::terminal, id_idx}, pte::shift(1)));
        REQUIRE(hints[25] == teh(7, {st::terminal, minus_idx}, pte::shift(2)));
        REQUIRE(hints[26] == teh(7, {st::non_terminal, expr_idx}, pte::shift(10)));

        // State 8 resolved
        REQUIRE(hints[27] == teh(8, {st::terminal, eof_idx}, pte::reduce(expr_idx, 3)));
        REQUIRE(hints[28] == teh(8, {st::terminal, plus_idx}, pte::reduce(expr_idx, 3)));
        REQUIRE(hints[29] == teh(8, {st::terminal, minus_idx}, pte::reduce(expr_idx, 3)));

        REQUIRE(hints[30] == teh(8, {st::terminal, mul_idx}, pte::shift(7)));

        // State 9 resolved
        REQUIRE(hints[31] == teh(9, {st::terminal, eof_idx}, pte::reduce(expr_idx, 3)));
        REQUIRE(hints[32] == teh(9, {st::terminal, plus_idx}, pte::reduce(expr_idx, 3)));
        REQUIRE(hints[33] == teh(9, {st::terminal, minus_idx}, pte::reduce(expr_idx, 3)));

        REQUIRE(hints[34] == teh(9, {st::terminal, mul_idx}, pte::shift(7)));

        // State 10 resolved
        REQUIRE(hints[35] == teh(10, {st::terminal, eof_idx}, pte::reduce(expr_idx, 3)));
        REQUIRE(hints[36] == teh(10, {st::terminal, plus_idx}, pte::reduce(expr_idx, 3)));
        REQUIRE(hints[37] == teh(10, {st::terminal, minus_idx}, pte::reduce(expr_idx, 3)));
        REQUIRE(hints[38] == teh(10, {st::terminal, mul_idx}, pte::reduce(expr_idx, 3)));
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

    lr1_set_builder builder(rs);

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
        }
        for (auto la : all_la)
        {
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
        }
        for (auto la : all_la)
        {
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
        REQUIRE(warnings[ 3] == "\nConflict in state 5 on lookahead '+' resolved");
        
        REQUIRE(warnings[ 4] == "Conflict in state 5 on lookahead '^' :");
        REQUIRE(warnings[ 5] == "\n    Expr -> Expr + Expr . / ^");
        REQUIRE(warnings[ 6] == "\n    shift on '^' to state 4 has the highest precedence");
        REQUIRE(warnings[ 7] == "\nConflict in state 5 on lookahead '^' resolved");
        
        REQUIRE(warnings[ 8] == "Conflict in state 6 on lookahead '+' :");
        REQUIRE(warnings[ 9] == "\n    Expr -> Expr ^ Expr . / + (highest precedence)");
        REQUIRE(warnings[10] == "\n    shift on '+'");
        REQUIRE(warnings[11] == "\nConflict in state 6 on lookahead '+' resolved");
        
        REQUIRE(warnings[12] == "Conflict in state 6 on lookahead '^' :");
        REQUIRE(warnings[13] == "\n    Expr -> Expr ^ Expr . / ^");
        REQUIRE(warnings[14] == "\n    shift on '^' to state 4 has the highest precedence");
        REQUIRE(warnings[15] == "\nConflict in state 6 on lookahead '^' resolved");
    }

    SECTION("table")
    {
        const auto& hints = ptg.get_table_entry_hints();
        REQUIRE(hints.size() == 18);

        REQUIRE(hints[0] == teh(0, {st::terminal, id_idx}, pte::shift(1)));
        REQUIRE(hints[1] == teh(0, {st::non_terminal, expr_idx}, pte::shift(2)));

        // Reduces from state 1 for all_la
        for (size_t i = 0; i < 3; ++i)
        {
            REQUIRE(hints[2 + i] == teh(1, {st::terminal, all_la[i]}, pte::reduce(expr_idx, 1)));
        }

        REQUIRE(hints[5] == teh(2, {st::terminal, eof_idx}, pte::reduce(root_idx, 1)));        
        REQUIRE(hints[6] == teh(2, {st::terminal, plus_idx}, pte::shift(3)));
        REQUIRE(hints[7] == teh(2, {st::terminal, pow_idx}, pte::shift(4)));

        REQUIRE(hints[8] == teh(3, {st::terminal, id_idx}, pte::shift(1)));
        REQUIRE(hints[9] == teh(3, {st::non_terminal, expr_idx}, pte::shift(5)));

        REQUIRE(hints[10] == teh(4, {st::terminal, id_idx}, pte::shift(1)));
        REQUIRE(hints[11] == teh(4, {st::non_terminal, expr_idx}, pte::shift(6)));

        REQUIRE(hints[12] == teh(5, {st::terminal, eof_idx}, pte::reduce(expr_idx, 3)));        
        REQUIRE(hints[13] == teh(5, {st::terminal, plus_idx}, pte::reduce(expr_idx, 3)));
        REQUIRE(hints[14] == teh(5, {st::terminal, pow_idx}, pte::shift(4)));

        REQUIRE(hints[15] == teh(6, {st::terminal, eof_idx}, pte::reduce(expr_idx, 3)));        
        REQUIRE(hints[16] == teh(6, {st::terminal, plus_idx}, pte::reduce(expr_idx, 3)));
        REQUIRE(hints[17] == teh(6, {st::terminal, pow_idx}, pte::shift(4)));
    }
}
