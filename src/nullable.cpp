#include <nullable.h>

namespace ptg
{

struct nullable_calculator
{
    nullable_calculator(const ruleset& rs, base_index_subset& nterms, base_index_subset& rside_parts)
        : rs_(rs), nterms_(nterms), rside_parts_(rside_parts),
          calculating_nterms_(nterms.get_size()), calculating_rside_parts_(rside_parts.get_size())
    {}
    
    bool calculate_nterm(size_t nterm_idx)
    {
        if (nterms_.contains(nterm_idx))
        {
            return true;
        }
        
        if (!calculating_nterms_.add(nterm_idx))
        {
            return false;
        }
        
        bool ret = false;
        
        for (size_t rside_idx = 0; rside_idx < rs_.get_nterm_rside_count(nterm_idx); ++rside_idx)
        {
            if (rs_.get_symbol_count(nterm_idx, rside_idx) == 0 || calculate_rside_part(nterm_idx, rside_idx, 0))
            {
                ret = true;
                break;
            }
        }
        
        if (ret)
        {
            nterms_.add(nterm_idx);
        }
        
        calculating_nterms_.remove(nterm_idx);
        return ret;
    };
    
    bool calculate_rside_part(size_t nterm_idx, size_t rside_idx, size_t symbol_start_idx)
    {
        auto idx = rs_.get_rside_part_flat_index(nterm_idx, rside_idx, symbol_start_idx);
        
        if (rside_parts_.contains(idx))
        {
            return true;
        }
        
        if (!calculating_rside_parts_.add(idx))
        {
            return false;
        }
        
        bool ret = true;
                
        for (size_t symbol_idx = symbol_start_idx; symbol_idx < rs_.get_symbol_count(nterm_idx, rside_idx); ++symbol_idx)
        {
            auto ref = rs_.get_symbol(nterm_idx, rside_idx, symbol_idx);
            if (ref.type_ == symbol_type::terminal || calculate_nterm(ref.index_) == false)
            {
                ret = false;
                break;
            }
        }
        
        if (ret)
        {
            rside_parts_.add(idx);
        }
        
        calculating_rside_parts_.remove(idx);
        
        return ret;
    };
    
    const ruleset& rs_;
    base_index_subset& nterms_;
    base_index_subset& rside_parts_;
    base_index_subset calculating_nterms_;
    base_index_subset calculating_rside_parts_; 
};

size_t nullable::compute_rside_parts_size(const ruleset& rs)
{
    size_t n = rs.get_nterm_count();
    size_t max_rsides = rs.get_max_rside_count();
    size_t max_symbols = rs.get_max_symbol_count();
    return n * max_rsides * max_symbols;
}

nullable::nullable(const ruleset& rs)
    : rs_(rs), nterms_(rs.get_nterm_count(), false), rside_parts_(compute_rside_parts_size(rs), false)
{    
    nullable_calculator c(rs_, nterms_, rside_parts_);
    
    for (size_t nterm_idx = 0; nterm_idx < nterms_.get_size(); ++nterm_idx)
    {
        c.calculate_nterm(nterm_idx);
    
        for (size_t rside_idx = 0; rside_idx < rs_.get_nterm_rside_count(nterm_idx); ++rside_idx)
        {
            for (size_t symbol_idx = 0; symbol_idx < rs_.get_symbol_count(nterm_idx, rside_idx); ++symbol_idx)
            {
                c.calculate_rside_part(nterm_idx, rside_idx, symbol_idx);
            }
        }
    }
}

bool nullable::is_nullable_nterm(size_t idx) const
{
    return nterms_.contains(idx);
}

bool nullable::is_nullable_rside_part(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const
{
    size_t flat = rs_.get_rside_part_flat_index(nterm_idx, rside_idx, symbol_idx);
    return rside_parts_.contains(flat);
}
    
} // namespace ptg
