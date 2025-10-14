#pragma once

#include <nullable.h>
#include <ruleset.h>
#include <index_subset.h>
#include <lr1_state.h>

#include <vector>
#include <string>

namespace ptg
{
    
class lr1_state_set
{
private:
    const ruleset& rs_;
    nullable null_;
    std::vector<std::string> warnings_;
    std::vector<lr1_state> states_;

    const ruleset& validate(const ruleset& rs) const;
    void collect_unused_warnings();
    
public:
    lr1_state_set(const ruleset& rs);
    ~lr1_state_set() = default;

    const std::vector<std::string>& get_warnings() const;
};

} // namespace ptg
