#pragma once

#include "ruleset.h"
#include "index_subset.h"
#include "lr1_set_item.h"
#include "vector_n.h"

namespace muskox
{

class closure
{
public:
    closure(const ruleset& rs);
    ~closure() = default;
    
    using opt_subset = std::optional<index_subset<4>>;

    const opt_subset& calculate(const lr1_set_item& item);
    const opt_subset& calculate_full(const lr1_set_item& item);
    
private:
    const ruleset& test_ruleset_validated(const ruleset& rs) const;
    const ruleset& rs_;
    
    vector_n<opt_subset, 4> lr1_item_closures_;
    vector_n<opt_subset, 4> lr1_item_full_closures_;
};

} // namespace muskox
