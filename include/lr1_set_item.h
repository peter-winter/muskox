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
    
    array_type get_array() const
    {
        return {nterm_idx_, rside_idx_, suffix_idx_, lookahead_idx_};
    }
    
    bool operator == (const lr1_set_item& other) const
    {
        return nterm_idx_ == other.nterm_idx_ &&
            rside_idx_ == other.rside_idx_ &&
            suffix_idx_ == other.suffix_idx_ &&
            lookahead_idx_ == other.lookahead_idx_;
    }
    
    bool operator == (const array_type& arr) const
    {
        return (*this == lr1_set_item(arr));
    }
    
    lr1_set_item make_shifted() const
    {
        return lr1_set_item(nterm_idx_, rside_idx_, suffix_idx_ + 1, lookahead_idx_);
    }
};

using lr1_set = std::vector<lr1_set_item>;

} // namespace muskox
