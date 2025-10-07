#include <nullable.h>

#include <grammar_error.h>
#include <symbol_collection.h>

#include <stdexcept>

namespace ptg
{

size_t nullable::compute_rside_parts_size(const ruleset& rs)
{
    size_t n = rs.get_nterm_count();
    
    for (size_t i = 0; i < n; ++i)
    {
        if (rs.get_nterm_rside_count(i) == 0)
        {
            const std::string_view name = rs.get_nterm_name(i);
            throw grammar_error(grammar_error::code::nterm_no_rules, name);
        }
    }

    size_t max_rsides = rs.get_max_rside_count();
    size_t max_symbols = rs.get_max_symbol_count();
    return n * max_rsides * max_symbols;
}

nullable::nullable(const ruleset& rs)
    : nterms_(rs.get_nterm_count()), rside_parts_(compute_rside_parts_size(rs))
{
}

const index_subset& nullable::get_nterms() const
{
    return nterms_;
}

const index_subset& nullable::get_rside_parts() const
{
    return rside_parts_;
}

} // namespace ptg
