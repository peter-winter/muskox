#pragma once

#include "ruleset.h"
#include "ordered_bitset_nd.h"

#include <variant>
#include <map>

namespace muskox
{

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
        std::vector<std::array<size_t, 4>> items_;
    };
    
    struct conflict
    {
        std::optional<shift> s_;
        std::vector<reduction> r_;
    };
    
    using action = std::variant<shift, reduction, conflict>;
    using action_map = std::map<symbol_ref, lr1_state::action>;
    
private:
    const ruleset& rs_;
    ordered_bitset_nd<4> items_;
    ordered_bitset_nd<4> kernel_;
        
public:
    lr1_state(const ruleset& rs, ordered_bitset_nd<4>&& kernel);

    void add_items(const ordered_bitset_nd<4>& items);
        
    bool contains_all_items(const ordered_bitset_nd<4>& items) const;
    const ordered_bitset_nd<4>& get_all_items() const;
    bool kernel_contains_all_items(const ordered_bitset_nd<4>& items) const;
    const ordered_bitset_nd<4>& get_kernel() const;
    
    action_map get_actions() const;
    
    std::string to_string() const;
    
    bool kernel_matches(const ordered_bitset_nd<4>& other) const;
    bool matches(const ordered_bitset_nd<4>& other) const;
};

} // namespace muskox
