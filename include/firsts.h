#pragma once

#include <nullable.h>
#include <ruleset.h>
#include <index_subset.h>
#include <vector_n.h>

#include <optional>

namespace ptg
{

class firsts
{
private:
    const ruleset& rs_;
    nullable null_;
    using opt_subset = std::optional<index_subset<1>>;
    vector_n<opt_subset, 1> nterms_;
    vector_n<opt_subset, 3> rside_parts_;
    
    const opt_subset& calculate_nterm_impl(
        size_t nterm_idx, 
        base_index_subset<1>& calculating_nterms, 
        base_index_subset<3>& calculating_rside_parts);
        
    const opt_subset& calculate_rside_part_impl(
        size_t nterm_idx, 
        size_t rside_idx, 
        size_t symbol_start_idx, 
        base_index_subset<1>& calculating_nterms, 
        base_index_subset<3>& calculating_rside_parts);
        
public:
    const opt_subset& calculate_nterm(size_t nterm_idx);
    const opt_subset& calculate_rside_part(size_t nterm_idx, size_t rside_idx, size_t symbol_start_idx);
 
    const opt_subset& get_nterm_firsts(size_t nterm_idx) const;
    const opt_subset& get_rside_part_firsts(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const;

    bool calculate_nullable_rside_part(size_t nterm_idx, size_t rside_idx, size_t symbol_idx);
    
    void calculate_all();
    
    firsts(const ruleset& rs);
    ~firsts() = default;
};

} // namespace ptg
