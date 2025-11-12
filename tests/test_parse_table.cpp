#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include "parse_table.h"
#include "parse_table_entry.h"

using Catch::Matchers::Message;
using namespace muskox;

TEST_CASE("parse_table construction and basic access", "[parse_table]")
{
    const size_t term_count = 5;
    const size_t nterm_count = 3;
    const size_t state_count = 10;
    parse_table pt(term_count, nterm_count, state_count);

    REQUIRE(pt.get_state_count() == state_count);
    REQUIRE(pt.get_symbol_count() == term_count + nterm_count);

    // Default entries should be empty/invalid
    const auto& default_entry = pt.get_term_entry(0, 0);
    REQUIRE(default_entry.is_error());

    // Set and get shift
    pt.get_term_entry(1, 2) = parse_table_entry::shift(5);
    const auto& shift_entry = pt.get_term_entry(1, 2);
    REQUIRE(shift_entry.is_shift());
    REQUIRE(shift_entry.get_shift_state_idx() == 5);

    // Set and get goto
    pt.get_nterm_entry(3, 1) = parse_table_entry::shift(7); // Note: shift used for goto in context
    const auto& goto_entry = pt.get_nterm_entry(3, 1);
    REQUIRE(goto_entry.is_shift());
    REQUIRE(goto_entry.get_shift_state_idx() == 7);

    // Set and get reduce
    pt.get_term_entry(4, 3) = parse_table_entry::reduce(2, 4);
    const auto& reduce_entry = pt.get_term_entry(4, 3);
    REQUIRE(reduce_entry.is_reduce());
    REQUIRE(reduce_entry.get_reduce_nterm_idx() == 2);
    REQUIRE(reduce_entry.get_reduce_length() == 4);

    // Set and get rr_conflict
    pt.get_term_entry(5, 0) = parse_table_entry::rr_conflict(1, 3);
    const auto& rr_entry = pt.get_term_entry(5, 0);
    REQUIRE(rr_entry.is_rr_conflict());
    REQUIRE(rr_entry.get_rr_conflict_start_idx() == 1);
    REQUIRE(rr_entry.get_rr_conflict_count() == 3);
}

TEST_CASE("parse_table bounds checking", "[parse_table]")
{
    parse_table pt(2, 2, 3);

    // Valid accesses
    REQUIRE_NOTHROW(pt.get_term_entry(0, 0));
    REQUIRE_NOTHROW(pt.get_term_entry(2, 1));
    REQUIRE_NOTHROW(pt.get_nterm_entry(0, 0));
    REQUIRE_NOTHROW(pt.get_nterm_entry(2, 1));

    // Invalid state
    REQUIRE_THROWS_MATCHES(pt.get_term_entry(3, 0), std::out_of_range, Message("State index out of range"));
    REQUIRE_THROWS_MATCHES(pt.get_nterm_entry(3, 0), std::out_of_range, Message("State index out of range"));

    // Invalid term
    REQUIRE_THROWS_MATCHES(pt.get_term_entry(0, 2), std::out_of_range, Message("Term index out of range"));

    // Invalid nterm
    REQUIRE_THROWS_MATCHES(pt.get_nterm_entry(0, 2), std::out_of_range, Message("Nterm index out of range"));
}
