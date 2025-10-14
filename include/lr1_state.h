#pragma once

#include <ruleset.h>
#include <index_subset.h>

#include <vector>

namespace ptg
{

class lr1_state
{
private:
    const ruleset& rs_;
    index_subset<4> items_;
    index_subset<4> kernel_;
    
    using symbol_items = std::vector<std::array<size_t, 4>>;
    
    std::vector<symbol_items> items_per_term_;
    std::vector<symbol_items> items_per_nterm_;
    std::vector<symbol_items> end_items_per_term_;
    
    void add_item(const std::array<size_t, 4>& indices);
        
public:
    lr1_state(const ruleset& rs, index_subset<4>&& kernel);

    void add_items(const index_subset<4>& items);
    
    const symbol_items& get_symbol_items(symbol_ref ref) const;
    const symbol_items& get_end_items(size_t term_idx) const;
    
    bool contains_all_items(const index_subset<4>& items) const;
};

} // namespace ptg
