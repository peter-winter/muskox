#pragma once

#include <nullable.h>
#include <ruleset.h>

namespace ptg
{

class lr1_state_set
{
private:
    const ruleset& rs_;
    nullable nullables_;

    const ruleset& validate(const ruleset& rs) const;
    
public:
    lr1_state_set(const ruleset& rs);
    ~lr1_state_set() = default;
};

} // namespace ptg
