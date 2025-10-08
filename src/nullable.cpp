#include <nullable.h>

namespace ptg
{

size_t nullable::compute_rside_parts_size(const ruleset& rs)
{
    size_t n = rs.get_nterm_count();
    size_t max_rsides = rs.get_max_rside_count();
    size_t max_symbols = rs.get_max_symbol_count();
    return n * max_rsides * max_symbols;
}

nullable::nullable(const ruleset& rs)
    : nterms_(rs.get_nterm_count(), true), rside_parts_(compute_rside_parts_size(rs), true)
{
}

const base_index_subset& nullable::get_nterms() const
{
    return nterms_;
}

const base_index_subset& nullable::get_rside_parts() const
{
    return rside_parts_;
}

} // namespace ptg
