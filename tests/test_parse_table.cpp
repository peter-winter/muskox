#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <parse_table_generator.h>
#include <parse_table.h>
#include <ruleset.h>
#include <symbol_collection.h>

using Catch::Matchers::Message;

using namespace muskox;

using pte = parse_table_entry;
using st = symbol_type;

TEST_CASE("parse_table basics", "[parse_table]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
    sc.validate();
    
    ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a"});
    [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"a"});
    rs.validate();

    size_t state_count = 5;
    parse_table pt(rs, state_count, 5);

    REQUIRE(pt.get_state_count() == state_count);
    REQUIRE(pt.get_symbol_count() == rs.get_term_plus_nterm_count());

    SECTION("default error")
    {
        symbol_ref s_ref{st::non_terminal, s_idx};
        const auto& entry = pt.get(0, s_ref);
        REQUIRE(entry.is_error());
        REQUIRE(entry.get_shift_state_idx() == 0); // arbitrary, but no throw
        REQUIRE(entry.get_reduce_nterm_idx() == 0);
        REQUIRE(entry.get_reduce_rside_idx() == 0);
    }

    SECTION("set and get shift")
    {
        symbol_ref a_ref{st::terminal, a_idx};
        auto& entry = pt.get(2, a_ref);
        entry.set_shift(3);

        const auto& const_entry = pt.get(2, a_ref);
        REQUIRE(const_entry.is_shift());
        REQUIRE(const_entry.get_shift_state_idx() == 3);
    }

    SECTION("set and get reduce")
    {
        symbol_ref a_ref{st::terminal, a_idx};
        auto& entry = pt.get(4, a_ref);
        entry.set_reduce(b_idx, b_r0);

        const auto& const_entry = pt.get(4, a_ref);
        REQUIRE(const_entry.is_reduce());
        REQUIRE(const_entry.get_reduce_nterm_idx() == b_idx);
        REQUIRE(const_entry.get_reduce_rside_idx() == b_r0);
    }
    
    SECTION("set and get rr-conflict")
    {
        symbol_ref a_ref{st::terminal, a_idx};
        auto& entry = pt.get(4, a_ref);
        entry.set_rr_conflict(3, 2);

        const auto& const_entry = pt.get(4, a_ref);
        REQUIRE(const_entry.is_rr_conflict());
        REQUIRE(const_entry.get_rr_conflict_start_idx() == 3);
        REQUIRE(const_entry.get_rr_conflict_count() == 2);
    }

    SECTION("out of range")
    {
        symbol_ref s_ref{st::non_terminal, s_idx};
        REQUIRE_THROWS_AS(pt.get(state_count, s_ref), std::out_of_range);
    }
    
    SECTION("equality")
    {
        pte e1, e2;
        REQUIRE(e1 == e2);  // Both error

        e1.set_shift(42);
        e2.set_shift(42);
        REQUIRE(e1 == e2);

        e1.set_reduce(1, 2);
        e2.set_reduce(1, 2);
        REQUIRE(e1 == e2);
        
        e1.set_rr_conflict(4, 5);
        e2.set_rr_conflict(4, 5);
        REQUIRE(e1 == e2);
        
        e1.set_shift(42);
        e2.set_reduce(1, 2);
        REQUIRE_FALSE(e1 == e2);
    }
}

TEST_CASE("parse_table validate", "[parse_table]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
    sc.validate();

    ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a"});
    [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"a"});
    rs.validate();

    size_t state_count = 3;
    parse_table pt(rs, state_count, 6);

    SECTION("valid empty")
    {
        REQUIRE_NOTHROW(pt.validate());
    }

    SECTION("valid shift")
    {
        symbol_ref a_ref{st::terminal, a_idx};
        auto& entry = pt.get(0, a_ref);
        entry.set_shift(2);
        REQUIRE_NOTHROW(pt.validate());
    }

    SECTION("invalid shift")
    {
        symbol_ref a_ref{st::terminal, a_idx};
        auto& entry = pt.get(0, a_ref);
        entry.set_shift(state_count);
        REQUIRE_THROWS_MATCHES(pt.validate(), std::runtime_error, Message("Invalid shift state index"));
    }

    SECTION("valid reduce")
    {
        symbol_ref b_ref{st::non_terminal, b_idx};
        auto& entry = pt.get(1, b_ref);
        entry.set_reduce(b_idx, b_r0);
        REQUIRE_NOTHROW(pt.validate());
    }

    SECTION("invalid reduce nterm")
    {
        symbol_ref b_ref{st::non_terminal, b_idx};
        pt.get(1, b_ref) = pte::reduce(rs.get_nterm_count(), 0);
        REQUIRE_THROWS_MATCHES(pt.validate(), std::runtime_error, Message("Invalid reduce nterm index"));
    }

    SECTION("invalid reduce rside")
    {
        symbol_ref b_ref{st::non_terminal, b_idx};
        pt.get(1, b_ref) = pte::reduce(b_idx, rs.get_nterm_rside_count(b_idx));
        REQUIRE_THROWS_MATCHES(pt.validate(), std::runtime_error, Message("Invalid reduce rside index"));
    }
    
    SECTION("valid rr-conflict")
    {
        symbol_ref b_ref{st::non_terminal, b_idx};
        auto& entry = pt.get(1, b_ref);
        entry.set_rr_conflict(4, 2);
        REQUIRE_NOTHROW(pt.validate());
    }

    SECTION("invalid rr-conflict")
    {
        symbol_ref b_ref{st::non_terminal, b_idx};
        SECTION("start overflow")
        {
            pt.get(1, b_ref) = pte::rr_conflict(pt.get_rr_table().size(), 1);
            REQUIRE_THROWS_MATCHES(pt.validate(), std::runtime_error, Message("Invalid rr-conflict start index"));
        }
        SECTION("start + count overflow")
        {
            pt.get(1, b_ref) = pte::rr_conflict(pt.get_rr_table().size() - 1, 2);
            REQUIRE_THROWS_MATCHES(pt.validate(), std::runtime_error, Message("Invalid rr-conflict count"));
        }
        SECTION("count zero")
        {
            pt.get(1, b_ref) = pte::rr_conflict(pt.get_rr_table().size() - 1, 0);
            REQUIRE_THROWS_MATCHES(pt.validate(), std::runtime_error, Message("Zero rr-conflict count"));
        }
    }
}

TEST_CASE("parse_table_entry limits", "[parse_table]")
{
    pte entry;

    SECTION("set_shift overflow")
    {
        REQUIRE_THROWS_MATCHES(entry.set_shift(4294967296ULL), std::overflow_error, Message("Shift state index exceeds 32-bit limit"));
    }

    SECTION("set_reduce nterm overflow")
    {
        REQUIRE_THROWS_MATCHES(entry.set_reduce(65536, 0), std::overflow_error, Message("Reduce nterm index exceeds 16-bit limit"));
    }

    SECTION("set_reduce rside overflow")
    {
        REQUIRE_THROWS_MATCHES(entry.set_reduce(0, 65536), std::overflow_error, Message("Reduce rside index exceeds 16-bit limit"));
    }
    
    SECTION("set_rr_conflict start index overflow")
    {
        REQUIRE_THROWS_MATCHES(entry.set_rr_conflict(65536, 1), std::overflow_error, Message("RR conflict table start index exceeds 16-bit limit"));
    }
    
    SECTION("set_rr_conflict count overflow")
    {
        REQUIRE_THROWS_MATCHES(entry.set_rr_conflict(1, 65536), std::overflow_error, Message("RR conflict table element count exceeds 16-bit limit"));
    }
}

TEST_CASE("parse_table_generator create_parse_table simple", "[parse_table_generator]")
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

    parse_table pt = ptg.create_parse_table();

    REQUIRE(pt.get_state_count() == 5);
    REQUIRE(pt.get_symbol_count() == rs.get_term_plus_nterm_count());

    // State 0: kernel {$root -> . S / $eof}, items: that + {S -> . a B / $eof}
    REQUIRE(pt.get(0, {st::terminal, a_idx}) == pte::shift(1));
    REQUIRE(pt.get(0, {st::non_terminal, s_idx}) == pte::shift(2));

    // State 1: kernel {S -> a . B / $eof}, items: that + {B -> . c / $eof}
    REQUIRE(pt.get(1, {st::terminal, c_idx}) == pte::shift(3));
    REQUIRE(pt.get(1, {st::non_terminal, b_idx}) == pte::shift(4));

    // State 2: kernel {$root -> S . / $eof}
    REQUIRE(pt.get(2, {st::terminal, eof_idx}) == pte::reduce(root_idx, 0));

    // State 3: kernel {B -> c . / $eof}
    REQUIRE(pt.get(3, {st::terminal, eof_idx}) == pte::reduce(b_idx, 0));

    // State 4: kernel {S -> a B . / $eof}
    REQUIRE(pt.get(4, {st::terminal, eof_idx}) == pte::reduce(s_idx, 0));
}

