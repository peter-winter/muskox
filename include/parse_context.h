#pragma once

#include "state_stacks.h"
#include "refs.h"

#include <string>
#include <vector>

namespace muskox
{

class parse_context
{
public:
    void on_shift(size_t /*active_stack_idx*/, size_t /*from_state*/, size_t /*to_state*/, size_t /*term_idx*/) {}
    void on_reduce(size_t /*active_stack_idx*/, size_t /*nterm_idx*/, size_t /*reduce_length*/) {}
    void on_error(size_t /*active_stack_idx*/, size_t /*state*/, symbol_ref /*lookahead*/) {}
    void on_accept(size_t /*active_stack_idx*/) {}

    void push_state_on_stack(size_t active_stack_idx, size_t state_idx);
    bool stack_empty(size_t active_stack_idx) const;
    size_t get_back_state(size_t active_stack_idx) const;
    bool pop_n_from_stack(size_t active_stack_idx, size_t n);

    void add_error(std::string error);
    std::vector<std::string> take_errors();

private:
    state_stacks state_stacks_;
    std::vector<std::string> errors_;
};

} // namespace muskox
