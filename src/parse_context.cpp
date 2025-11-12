#include "parse_context.h"

#include <utility>

namespace muskox
{

void parse_context::push_state_on_stack(size_t active_stack_idx, size_t state_idx)
{
    state_stacks_.push_state_on_stack(active_stack_idx, state_idx);
}

bool parse_context::stack_empty(size_t active_stack_idx) const
{
    return state_stacks_.stack_empty(active_stack_idx);
}

size_t parse_context::get_back_state(size_t active_stack_idx) const
{
    return state_stacks_.get_back_state(active_stack_idx);
}

bool parse_context::pop_n_from_stack(size_t active_stack_idx, size_t n)
{
    return state_stacks_.pop_n_from_stack(active_stack_idx, n);
}

void parse_context::add_error(std::string error)
{
    errors_.push_back(std::move(error));
}

std::vector<std::string> parse_context::take_errors()
{
    return std::exchange(errors_, {});
}

} // namespace muskox
