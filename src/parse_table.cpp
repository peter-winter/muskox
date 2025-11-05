#include "parse_table.h"

#include <stdexcept>

namespace muskox
{

parse_table::parse_table(const ruleset& rs, size_t state_count)
    : rs_(rs), state_count_(state_count), table_({state_count, rs.get_term_plus_nterm_count()}, parse_table_entry{})
{
}

size_t parse_table::get_column(const symbol_ref& ref) const
{
    if (ref.type_ == symbol_type::terminal)
    {
        return ref.index_;
    }
    else
    {
        return rs_.get_term_count() + ref.index_;
    }
}

parse_table_entry& parse_table::get(size_t state, const symbol_ref& ref)
{
    if (state >= state_count_)
    {
        throw std::out_of_range("State index out of range");
    }
    size_t col = get_column(ref);
    return table_.get(state, col);
}

const parse_table_entry& parse_table::get(size_t state, const symbol_ref& ref) const
{
    if (state >= state_count_)
    {
        throw std::out_of_range("State index out of range");
    }
    size_t col = get_column(ref);
    return table_.get(state, col);
}

size_t parse_table::get_state_count() const
{
    return state_count_;
}

size_t parse_table::get_symbol_count() const
{
    return rs_.get_term_plus_nterm_count();
}

void parse_table::validate() const
{
    size_t symbol_count = get_symbol_count();
    for (size_t state = 0; state < state_count_; ++state)
    {
        for (size_t col = 0; col < symbol_count; ++col)
        {
            const auto& entry = table_.get(state, col);
            if (entry.is_shift())
            {
                size_t shift_state = entry.get_shift_state_idx();
                if (shift_state >= state_count_)
                {
                    throw std::runtime_error("Invalid shift state index");
                }
            }
            else if (entry.is_reduce())
            {
                size_t nterm_idx = entry.get_reduce_nterm_idx();
                size_t rside_idx = entry.get_reduce_rside_idx();
                if (nterm_idx >= rs_.get_nterm_count())
                {
                    throw std::runtime_error("Invalid reduce nterm index");
                }
                if (rside_idx >= rs_.get_nterm_rside_count(nterm_idx))
                {
                    throw std::runtime_error("Invalid reduce rside index");
                }
            }
            // error is always valid
        }
    }
}

} // namespace muskox
