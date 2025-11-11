#pragma once

#include "sorted_grouped_vector.h"
#include "lr1_set_item.h"

#include <functional>

namespace muskox
{

struct lr1_set_item_comp
{
    lr1_set_item_comp(const ruleset& rs)
        : rs_(std::cref(rs))
    {}
    
    auto get_part(const lr1_set_item& it) const 
    {
        return rs_.get().get_symbol_of_interest(it);
    }
    
    std::reference_wrapper<const ruleset> rs_;
};
    
using lr1_sorted_set = sorted_grouped_vector<lr1_set_item, lr1_set_item_comp>;

}
