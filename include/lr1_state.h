#pragma once

#include "ruleset.h"
#include "bitset_nd.h"
#include "lr1_set_item.h"
#include "lr1_sorted_set.h"

namespace muskox
{

class lr1_state
{
public:
    lr1_state(const ruleset& rs, lr1_sorted_set&& kernel);

    void add_items(const lr1_sorted_set& items);
    void add_items(const ordered_bitset_nd<4>& items);
        
    const lr1_sorted_set& get_sorted_items() const;
    
    std::string to_string() const;
    
    bool kernel_matches(const lr1_sorted_set& items) const;
    bool kernel_matches(const lr1_set& items) const;
    bool matches(const lr1_set& items) const;
    
private:
    void add_item(const lr1_set_item& indices);
    
    const ruleset& rs_;
    bitset_nd<4> items_;
    lr1_sorted_set kernel_;
    lr1_sorted_set sorted_items_;
};

} // namespace muskox
