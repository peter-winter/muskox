#pragma once

#include <firsts.h>
#include <ruleset.h>
#include <index_subset.h>
#include <lr1_set_item.h>
#include <vector_n.h>

namespace ptg
{

class closure
{
private:
    firsts firsts_;
    const ruleset& rs_;
    using opt_subset = std::optional<index_subset<4>>;
    vector_n<opt_subset, 4> lr1_item_closures_;
    
public:
    closure(const ruleset& rs);
    ~closure() = default;

    const opt_subset& calculate(const lr1_set_item& item);
};

} // namespace ptg
