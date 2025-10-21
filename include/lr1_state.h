#pragma once

#include <ruleset.h>
#include <index_subset.h>

#include <variant>
#include <map>

namespace ptg
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
    index_subset<4> items_;
    index_subset<4> kernel_;
        
public:
    lr1_state(const ruleset& rs, index_subset<4>&& kernel);

    void add_items(const index_subset<4>& items);
        
    bool contains_all_items(const index_subset<4>& items) const;
    const index_subset<4>& get_all_items() const;
    bool kernel_contains_all_items(const index_subset<4>& items) const;
    const index_subset<4>& get_kernel() const;
    
    action_map get_actions() const;
    
    std::string to_string() const;
    
    bool kernel_matches(const index_subset<4>& other) const;
    bool matches(const index_subset<4>& other) const;
};

} // namespace ptg
