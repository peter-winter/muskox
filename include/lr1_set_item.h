#pragma once

#include <cstddef>
#include <array>

namespace muskox
{

struct lr1_set_item
{
    using array_type = std::array<size_t, 4>;
    std::size_t nterm_idx_;
    std::size_t rside_idx_;
    std::size_t suffix_idx_;
    std::size_t lookahead_idx_;

    lr1_set_item(std::size_t nterm_idx, std::size_t rside_idx, std::size_t suffix_idx, std::size_t lookahead_idx)
        : nterm_idx_(nterm_idx), rside_idx_(rside_idx), suffix_idx_(suffix_idx), lookahead_idx_(lookahead_idx)
    {
    }
    
    lr1_set_item(const array_type& arr)
        : nterm_idx_(arr[0]), rside_idx_(arr[1]), suffix_idx_(arr[2]), lookahead_idx_(arr[3])
    {
    }
};

} // namespace muskox
