#pragma once

#include <vector>

namespace muskox
{

class state_stacks
{
public:
    using stack_type = std::vector<size_t>;

    state_stacks();

    void push_state_on_stack(size_t idx, size_t state_idx);
    bool stack_empty(size_t idx) const;
    size_t get_back_state(size_t idx) const;
    bool pop_n_from_stack(size_t idx, size_t n);

private:
    std::vector<stack_type> stacks_;
};

} // namespace muskox
