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

parse_table_entry parse_table_entry::rr_conflict(size_t rr_conflict_start_idx, size_t rr_conflict_count)
{
    parse_table_entry entry;
    entry.set_rr_conflict(rr_conflict_start_idx, rr_conflict_count);
    return entry;
}

void parse_table_entry::set_shift(size_t state_idx)
{
    if (state_idx > std::numeric_limits<uint32_t>::max())
    {
        throw std::overflow_error("Shift state index exceeds 32-bit limit");
    }
    type_ = entry_type::shift;
    data_.shift_state_idx_ = static_cast<uint32_t>(state_idx);
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
    type_ = entry_type::reduce;
    data_.reduce_.nterm_idx_ = static_cast<uint16_t>(nterm_idx);
    data_.reduce_.rside_idx_ = static_cast<uint16_t>(rside_idx);
}

void parse_table_entry::set_rr_conflict(size_t rr_conflict_start_idx, size_t rr_conflict_count)
{
    if (rr_conflict_start_idx > std::numeric_limits<uint16_t>::max())
    {
        throw std::overflow_error("RR conflict table start index exceeds 16-bit limit");
    }
    if (rr_conflict_count > std::numeric_limits<uint16_t>::max())
    {
        throw std::overflow_error("RR conflict table element count exceeds 16-bit limit");
    }
    type_ = entry_type::rr_conflict;
    data_.rr_conflict_.rr_conflict_start_idx_ = static_cast<uint32_t>(rr_conflict_start_idx);
    data_.rr_conflict_.rr_conflict_count_ = static_cast<uint32_t>(rr_conflict_count);
}

bool parse_table_entry::is_error() const
{
    return type_ == entry_type::error;
}

bool parse_table_entry::is_shift() const
{
    return type_ == entry_type::shift;
}

bool parse_table_entry::is_reduce() const
{
    return type_ == entry_type::reduce;
}

bool parse_table_entry::is_rr_conflict() const
{
    return type_ == entry_type::rr_conflict;
}

size_t parse_table_entry::get_shift_state_idx() const
{
    return data_.shift_state_idx_;
}

size_t parse_table_entry::get_reduce_nterm_idx() const
{
    return data_.reduce_.nterm_idx_;
}

size_t parse_table_entry::get_reduce_rside_idx() const
{
    return data_.reduce_.rside_idx_;
}

size_t parse_table_entry::get_rr_conflict_start_idx() const
{
    return data_.rr_conflict_.rr_conflict_start_idx_;
}

size_t parse_table_entry::get_rr_conflict_count() const
{
    return data_.rr_conflict_.rr_conflict_count_;
}

bool parse_table_entry::operator==(const parse_table_entry& other) const
{
    if (type_ != other.type_) return false;
    switch (type_)
    {
    case entry_type::error:
        return true;
    case entry_type::shift:
        return data_.shift_state_idx_ == other.data_.shift_state_idx_;
    case entry_type::reduce:
        return data_.reduce_.nterm_idx_ == other.data_.reduce_.nterm_idx_ &&
               data_.reduce_.rside_idx_ == other.data_.reduce_.rside_idx_;
    case entry_type::rr_conflict:
        return data_.rr_conflict_.rr_conflict_start_idx_ == other.data_.rr_conflict_.rr_conflict_start_idx_ &&
               data_.rr_conflict_.rr_conflict_count_ == other.data_.rr_conflict_.rr_conflict_count_;
    }
    return false;  // Unreachable, but for completeness
}

}
