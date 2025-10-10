#include <nullable.h>

namespace ptg
{

bool nullable::calculate_nterm_impl(
    size_t nterm_idx,
    base_index_subset<1>& calculating_nterms, 
    base_index_subset<3>& calculating_rside_parts)
{
    if (nterms_.contains(nterm_idx))
    {
        return true;
    }
    
    if (!calculating_nterms.add(nterm_idx))
    {
        return false;
    }
    
    bool ret = false;
    for (size_t rside_idx = 0; rside_idx < rs_.get_nterm_rside_count(nterm_idx); ++rside_idx)
    {
        if (rs_.get_symbol_count(nterm_idx, rside_idx) == 0 ||
            calculate_rside_part_impl(nterm_idx, rside_idx, 0, calculating_nterms, calculating_rside_parts))
        {
            ret = true;
            break;
        }
    }
    
    if (ret)
    {
        nterms_.add(nterm_idx);
    }
    
    calculating_nterms.remove(nterm_idx);
    return ret;
}

bool nullable::calculate_rside_part_impl(
    size_t nterm_idx, 
    size_t rside_idx, 
    size_t symbol_start_idx,
    base_index_subset<1>& calculating_nterms, 
    base_index_subset<3>& calculating_rside_parts)
{
    if (rside_parts_.contains(nterm_idx, rside_idx, symbol_start_idx))
    {
        return true;
    }
    
    if (!calculating_rside_parts.add(nterm_idx, rside_idx, symbol_start_idx))
    {
        return false;
    }
    
    bool ret = true;
    
    for (size_t symbol_idx = symbol_start_idx; symbol_idx < rs_.get_symbol_count(nterm_idx, rside_idx); ++symbol_idx)
    {
        auto ref = rs_.get_symbol(nterm_idx, rside_idx, symbol_idx);
        if (ref.type_ == symbol_type::terminal || !calculate_nterm_impl(ref.index_, calculating_nterms, calculating_rside_parts))
        {
            ret = false;
            break;
        }
    }
    
    if (ret)
    {
        rside_parts_.add(nterm_idx, rside_idx, symbol_start_idx);
    }
    
    calculating_rside_parts.remove(nterm_idx, rside_idx, symbol_start_idx);
    return ret;
}

nullable::nullable(const ruleset& rs)
    : rs_(rs), 
      nterms_({rs.get_nterm_count()}, false),
      rside_parts_({rs.get_nterm_count(), rs.get_max_rside_count(), rs.get_max_symbol_count()}, false)
{
}

void nullable::calculate_all()
{
    for (size_t nterm_idx = 0; nterm_idx < rs_.get_nterm_count(); ++nterm_idx)
    {
        calculate_nterm(nterm_idx);

        for (size_t rside_idx = 0; rside_idx < rs_.get_nterm_rside_count(nterm_idx); ++rside_idx)
        {
            for (size_t symbol_idx = 0; symbol_idx < rs_.get_symbol_count(nterm_idx, rside_idx); ++symbol_idx)
            {
                calculate_rside_part(nterm_idx, rside_idx, symbol_idx);
            }
        }
    }
}

bool nullable::calculate_nterm(size_t nterm_idx)
{    
    base_index_subset<1> calculating_nterms({rs_.get_nterm_count()});
    base_index_subset<3> calculating_rside_parts({rs_.get_nterm_count(), rs_.get_max_rside_count(), rs_.get_max_symbol_count()});
    return calculate_nterm_impl(nterm_idx, calculating_nterms, calculating_rside_parts);
}

bool nullable::calculate_rside_part(size_t nterm_idx, size_t rside_idx, size_t symbol_idx)
{
    base_index_subset<1> calculating_nterms({rs_.get_nterm_count()});
    base_index_subset<3> calculating_rside_parts({rs_.get_nterm_count(), rs_.get_max_rside_count(), rs_.get_max_symbol_count()});
    return calculate_rside_part_impl(nterm_idx, rside_idx, symbol_idx, calculating_nterms, calculating_rside_parts);
}
    
bool nullable::is_nullable_nterm(size_t idx) const
{
    return nterms_.contains(idx);
}

bool nullable::is_nullable_rside_part(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const
{
    return rside_parts_.contains(nterm_idx, rside_idx, symbol_idx);
}

} // namespace ptg
