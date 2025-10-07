#pragma once

#include <index_subset.h>
#include <ruleset.h>

namespace ptg
{

class nullable
{
private:
    index_subset nterms_;
    index_subset rside_parts_;

    size_t compute_rside_parts_size(const ruleset& rs);

public:
    nullable(const ruleset& rs);
    ~nullable() = default;

    const index_subset& get_nterms() const;
    const index_subset& get_rside_parts() const;
};

} // namespace ptg
