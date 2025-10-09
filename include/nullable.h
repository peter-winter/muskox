#pragma once

#include <base_index_subset.h>
#include <ruleset.h>

namespace ptg
{

class nullable
{
private:
    base_index_subset<1> nterms_;
    base_index_subset<3> rside_parts_;

    // Calculation methods
    bool calculate_nterm(size_t nterm_idx, const ruleset& rs, base_index_subset<1>& calculating_nterms, base_index_subset<3>& calculating_rside_parts);
    bool calculate_rside_part(size_t nterm_idx, size_t rside_idx, size_t symbol_start_idx, const ruleset& rs, base_index_subset<1>& calculating_nterms, base_index_subset<3>& calculating_rside_parts);

public:
    nullable(const ruleset& rs);
    ~nullable() = default;

    bool is_nullable_nterm(size_t idx) const;
    bool is_nullable_rside_part(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const;
};

} // namespace ptg
