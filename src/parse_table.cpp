#include <parse_table.h>

#include <stdexcept>

namespace muskox
{
    
parse_table_entry parse_table_entry::shift(size_t state_idx)
{
    parse_table_entry entry;
    entry.set_shift(state_idx);
    return entry;
}

parse_table_entry parse_table_entry::reduce(size_t nterm_idx, size_t rside_idx)
{
    parse_table_entry entry;
    entry.set_reduce(nterm_idx, rside_idx);
    return entry;
}

void parse_table_entry::set_shift(size_t state_idx)
{
    if (state_idx > std::numeric_limits<uint32_t>::max())
    {
        throw std::overflow_error("Shift state index exceeds 32-bit limit");
    }
    type = entry_type::shift;
    data.shift_state_idx = static_cast<uint32_t>(state_idx);
}

void parse_table_entry::set_reduce(size_t nterm_idx, size_t rside_idx)
{
    if (nterm_idx > std::numeric_limits<uint16_t>::max())
    {
        throw std::overflow_error("Reduce nterm index exceeds 16-bit limit");
    }
    if (rside_idx > std::numeric_limits<uint16_t>::max())
    {
        throw std::overflow_error("Reduce rside index exceeds 16-bit limit");
    }
    type = entry_type::reduce;
    data.reduce.nterm_idx = static_cast<uint16_t>(nterm_idx);
    data.reduce.rside_idx = static_cast<uint16_t>(rside_idx);
}

bool parse_table_entry::is_error() const
{
    return type == entry_type::error;
}

bool parse_table_entry::is_shift() const
{
    return type == entry_type::shift;
}

bool parse_table_entry::is_reduce() const
{
    return type == entry_type::reduce;
}

size_t parse_table_entry::get_shift_state_idx() const
{
    return data.shift_state_idx;
}

size_t parse_table_entry::get_reduce_nterm_idx() const
{
    return data.reduce.nterm_idx;
}

size_t parse_table_entry::get_reduce_rside_idx() const
{
    return data.reduce.rside_idx;
}

bool parse_table_entry::operator==(const parse_table_entry& other) const
{
    if (type != other.type) return false;
    switch (type)
    {
    case entry_type::error:
        return true;
    case entry_type::shift:
        return data.shift_state_idx == other.data.shift_state_idx;
    case entry_type::reduce:
        return data.reduce.nterm_idx == other.data.reduce.nterm_idx &&
               data.reduce.rside_idx == other.data.reduce.rside_idx;
    }
    return false;  // Unreachable, but for completeness
}

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
