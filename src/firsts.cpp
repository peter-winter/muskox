#include <firsts.h>

namespace ptg
{

firsts::firsts(const ruleset& rs)
    : rs_(rs),
      null_(rs),
      nterms_({rs.get_nterm_count()}, std::nullopt),
      rside_parts_({rs.get_nterm_count(), rs.get_max_rside_count(), rs.get_max_symbol_count()}, std::nullopt)
{ 
}

void firsts::calculate_all()
{
    base_index_subset<1> calculating_nterms({rs_.get_nterm_count()});
    base_index_subset<3> calculating_rside_parts({rs_.get_nterm_count(), rs_.get_max_rside_count(), rs_.get_max_symbol_count()});

    for (size_t nterm_idx = 0; nterm_idx < rs_.get_nterm_count(); ++nterm_idx)
    {
        calculate_nterm_impl(nterm_idx, calculating_nterms, calculating_rside_parts);
    }

    for (size_t nterm_idx = 0; nterm_idx < rs_.get_nterm_count(); ++nterm_idx)
    {
        for (size_t rside_idx = 0; rside_idx < rs_.get_nterm_rside_count(nterm_idx); ++rside_idx)
        {
            for (size_t symbol_idx = 0; symbol_idx < rs_.get_symbol_count(nterm_idx, rside_idx); ++symbol_idx)
            {
                calculate_rside_part_impl(nterm_idx, rside_idx, symbol_idx, calculating_nterms, calculating_rside_parts);
            }
        }
    }
}

const firsts::opt_subset& firsts::calculate_nterm_impl(
    size_t nterm_idx, 
    base_index_subset<1>& calculating_nterms, 
    base_index_subset<3>& calculating_rside_parts)
{
    auto& result = nterms_.get(nterm_idx);
    
    if (!calculating_nterms.add(nterm_idx))
    {
        return result;
    }
    
    if (result.has_value())
    {
        return result;
    }
    
    index_subset temp(rs_.get_term_count());
            
    for (size_t rside_idx = 0; rside_idx < rs_.get_nterm_rside_count(nterm_idx); ++rside_idx)
    {
        const auto& part_result = calculate_rside_part_impl(nterm_idx, rside_idx, 0, calculating_nterms, calculating_rside_parts);
        if (part_result.has_value())
        {
            temp.add(part_result.value());
        }
    }
    
    if (temp.get_count() > 0)
    {
        result.emplace(std::move(temp));
    }
    
    calculating_nterms.remove(nterm_idx);
    return result;
}

const firsts::opt_subset& firsts::calculate_rside_part_impl(
    size_t nterm_idx, 
    size_t rside_idx, 
    size_t symbol_start_idx,
    base_index_subset<1>& calculating_nterms, 
    base_index_subset<3>& calculating_rside_parts)
{
    auto& result = rside_parts_.get(nterm_idx, rside_idx, symbol_start_idx);
    
    if (!calculating_rside_parts.add(nterm_idx, rside_idx, symbol_start_idx))
    {
        return result;
    }
        
    if (result.has_value())
    {
        return result;
    }
    
    index_subset temp(rs_.get_term_count());
    
    for (size_t symbol_idx = symbol_start_idx; symbol_idx < rs_.get_symbol_count(nterm_idx, rside_idx); ++symbol_idx)
    {
        auto ref = rs_.get_symbol(nterm_idx, rside_idx, symbol_idx);
        if (ref.type_ == symbol_type::terminal)
        {
            temp.add(ref.index_);
            break;
        }
        
        const auto& nterm_res = calculate_nterm_impl(ref.index_, calculating_nterms, calculating_rside_parts);
        if (nterm_res.has_value())
        {
            temp.add(nterm_res.value());
        }
        
        if (!null_.calculate_nterm(ref.index_))
        {
            break;
        }
    }
    
    if (temp.get_count() > 0)
    {
        result.emplace(std::move(temp));
    }
    
    calculating_rside_parts.remove(nterm_idx, rside_idx, symbol_start_idx);
    return result;
}

const firsts::opt_subset& firsts::get_nterm_firsts(size_t nterm_idx) const
{
    rs_.validate_nterm_idx(nterm_idx);
    return nterms_.get(nterm_idx);
}

const firsts::opt_subset& firsts::get_rside_part_firsts(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const
{
    rs_.validate_symbol_idx(nterm_idx, rside_idx, symbol_idx);
    return rside_parts_.get(nterm_idx, rside_idx, symbol_idx);
}


} // namespace ptg
