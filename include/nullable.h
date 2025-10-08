#pragma once

#include <base_index_subset.h>
#include <ruleset.h>

namespace ptg
{

class nullable
{
private:
    const ruleset& rs_;
    base_index_subset nterms_;
    base_index_subset rside_parts_;

    size_t compute_rside_parts_size(const ruleset& rs);

public:
    nullable(const ruleset& rs);
    ~nullable() = default;

    bool is_nullable_nterm(size_t idx) const;
    bool is_nullable_rside_part(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const;
};

} // namespace ptg
