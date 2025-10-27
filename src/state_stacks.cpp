// state_stacks.cpp
#include "state_stacks.h"

#include <stdexcept>

namespace ptg
{

state_stacks::state_stacks()
{
    stacks_.resize(1);
}

void state_stacks::push_state_on_stack(size_t idx, size_t state_idx)
{
    if (idx >= stacks_.size())
    {
        throw std::out_of_range("Stack index out of range");
    }
    stacks_[idx].push_back(state_idx);
}

bool state_stacks::stack_empty(size_t idx) const
{
    if (idx >= stacks_.size())
    {
        throw std::out_of_range("Stack index out of range");
    }
    return stacks_[idx].empty();
}

size_t state_stacks::get_back_state(size_t idx) const
{
    if (idx >= stacks_.size())
    {
        throw std::out_of_range("Stack index out of range");
    }
    if (stacks_[idx].empty())
    {
        throw std::out_of_range("Stack empty");
    }
    return stacks_[idx].back();
}

bool state_stacks::pop_n_from_stack(size_t idx, size_t n)
{
    if (idx >= stacks_.size())
    {
        throw std::out_of_range("Stack index out of range");
    }
    if (stacks_[idx].size() < n)
    {
        return false;
    }
    stacks_[idx].erase(stacks_[idx].end() - static_cast<int>(n), stacks_[idx].end());
    return true;
}

} // namespace ptg
