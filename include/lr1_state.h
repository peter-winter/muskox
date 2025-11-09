#pragma once

#include "ruleset.h"
#include "bitset_nd.h"
#include "sorted_grouped_vector.h"
#include "lr1_set_item.h"

#include <variant>
#include <map>

namespace muskox
{

struct lr1_set_item_comp
{
    lr1_set_item_comp(const ruleset& rs)
        : rs_(rs)
    {}
    
    auto get_part(const lr1_set_item& it) const 
    {
        if (it.suffix_idx_ < rs_.get_symbol_count(it.nterm_idx_, it.rside_idx_))
        {
            return rs_.get_symbol(it.nterm_idx_, it.rside_idx_, it.suffix_idx_);
        }
        else
        {
            return symbol_ref{symbol_type::terminal, it.lookahead_idx_};
        }
    }
    
    const ruleset& rs_;
};

class lr1_state
{
public:
    struct reduction
    {
        size_t nterm_idx_;
        size_t rside_idx_;
    };
    
    struct shift
    {
        lr1_set items_;
    };
    
    struct conflict
    {
        std::optional<shift> s_;
        std::vector<reduction> r_;
    };
    
    using action = std::variant<shift, reduction, conflict>;
    using action_map = std::map<symbol_ref, lr1_state::action>;
    
    lr1_state(const ruleset& rs, lr1_set&& kernel);

    void add_items(const lr1_set& items);
    void add_items(const ordered_bitset_nd<4>& items);
        
    action_map get_actions() const;
    const sorted_grouped_vector<lr1_set_item, lr1_set_item_comp>& get_sorted_items() const;
    
    std::string to_string() const;
    
    bool kernel_matches(const lr1_set& items) const;
    bool matches(const lr1_set& items) const;
    
    const lr1_set& get_kernel() const { return kernel_; }
    
private:
    void add_item(const lr1_set_item::array_type& indices);
    
    const ruleset& rs_;
    bitset_nd<4> items_;
    lr1_set kernel_;
    lr1_set_item_comp comparer_;
    sorted_grouped_vector<lr1_set_item, lr1_set_item_comp> sorted_items_;
};

} // namespace muskox
