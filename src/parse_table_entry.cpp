#include "parse_table_entry.h"

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

}
