#pragma once

#include <cstddef>

namespace ptg
{

struct lr1_set_item
{
    std::size_t nterm_idx_;
    std::size_t rside_idx_;
    std::size_t symbol_idx_;
    std::size_t lookahead_idx_;

    lr1_set_item(std::size_t nterm_idx, std::size_t rside_idx, std::size_t symbol_idx, std::size_t lookahead_idx)
        : nterm_idx_(nterm_idx), rside_idx_(rside_idx), symbol_idx_(symbol_idx), lookahead_idx_(lookahead_idx)
    {
    }
};

} // namespace ptg
