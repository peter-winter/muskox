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
    vector_n<std::optional<index_subset>, 1> nterms_;
    vector_n<std::optional<index_subset>, 3> rside_parts_;

    const index_subset& calculate_nterm_impl(
        size_t nterm_idx, 
        base_index_subset<1>& calculating_nterms, 
        base_index_subset<3>& calculating_rside_parts);
        
    const index_subset& calculate_rside_part_impl(
        size_t nterm_idx, 
        size_t rside_idx, 
        size_t symbol_start_idx, 
        base_index_subset<1>& calculating_nterms, 
        base_index_subset<3>& calculating_rside_parts);
        
public:
    const index_subset& get_nterm_firsts(size_t nterm_idx) const;
    const index_subset& get_rside_part_firsts(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const;
    
    void calculate_all();
    
    firsts(const ruleset& rs);
    ~firsts() = default;
};

} // namespace ptg
