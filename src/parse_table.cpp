/**
 * @file parse_table.cpp
 * @brief Implementation of the parse_table class.
 *
 * Provides methods for accessing and validating entries in the LR(1) parsing table.
 *
 * Part of the larger MuskOx project.
 */

#include "parse_table.h"

#include <stdexcept>

namespace muskox
{

parse_table::parse_table(size_t term_count, size_t nterm_count, size_t state_count)
    : term_count_(term_count),
      nterm_count_(nterm_count),
      state_count_(state_count),
      table_({state_count, term_count_ + nterm_count_}, parse_table_entry{})
{
}

parse_table_entry& parse_table::get_term_entry(size_t state, size_t term_idx)
{
    validate_state_idx(state);
    validate_term_idx(term_idx);
    return table_.get(state, term_idx);
}

const parse_table_entry& parse_table::get_term_entry(size_t state, size_t term_idx) const
{
    validate_state_idx(state);
    validate_term_idx(term_idx);
    return table_.get(state, term_idx);
}

parse_table_entry& parse_table::get_nterm_entry(size_t state, size_t nterm_idx)
{
    validate_state_idx(state);
    validate_nterm_idx(nterm_idx);
    return table_.get(state, term_count_ + nterm_idx);
}

const parse_table_entry& parse_table::get_nterm_entry(size_t state, size_t nterm_idx) const
{
    validate_state_idx(state);
    validate_nterm_idx(nterm_idx);
    return table_.get(state, term_count_ + nterm_idx);
}

size_t parse_table::get_state_count() const
{
    return state_count_;
}

size_t parse_table::get_symbol_count() const
{
    return term_count_ + nterm_count_;
}

void parse_table::validate_state_idx(size_t idx) const
{
    if (idx >= state_count_)
    {
        throw std::out_of_range("State index out of range");
    }
}

void parse_table::validate_term_idx(size_t idx) const
{
    if (idx >= term_count_)
    {
        throw std::out_of_range("Term index out of range");
    }
}

void parse_table::validate_nterm_idx(size_t idx) const
{
    if (idx >= nterm_count_)
    {
        throw std::out_of_range("Nterm index out of range");
    }
}

} // namespace muskox
