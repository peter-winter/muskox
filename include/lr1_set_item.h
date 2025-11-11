/**
 * @file lr1_set_item.h
 * @brief Defines the LR(1) set item structure and related types.
 *
 * This header provides a struct to represent an LR(1) item, which consists of a production
 * (non-terminal, right-hand side), a position (suffix index or "dot"), and a lookahead terminal.
 * It includes constructors, comparison operators, and a method to create a shifted item.
 * Additionally, it defines a type alias for a vector of such items, representing an LR(1) set.
 *
 * LR(1) items are fundamental in LR parsing for state representation and closure computations.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include <cstddef>
#include <array>

namespace muskox
{

/**
 * @struct lr1_set_item
 * @brief Represents an LR(1) item in parsing.
 *
 * Encapsulates the components of an LR(1) item: non-terminal index, right-hand side index,
 * suffix index (position of the dot), and lookahead terminal index.
 */
struct lr1_set_item
{
    /**
     * @brief Type alias for an array representation of the item's components.
     */
    using array_type = std::array<size_t, 4>;

    std::size_t nterm_idx_; /// Index of the non-terminal (left-hand side).
    std::size_t rside_idx_; /// Index of the right-hand side (production alternative).
    std::size_t suffix_idx_; /// Position in the right-hand side (dot position).
    std::size_t lookahead_idx_; /// Index of the lookahead terminal.

    /**
     * @brief Constructs an LR(1) item with individual indices.
     *
     * @param nterm_idx Non-terminal index.
     * @param rside_idx Right-hand side index.
     * @param suffix_idx Suffix (dot) position.
     * @param lookahead_idx Lookahead terminal index.
     */
    lr1_set_item(std::size_t nterm_idx, std::size_t rside_idx, std::size_t suffix_idx, std::size_t lookahead_idx)
        : nterm_idx_(nterm_idx), rside_idx_(rside_idx), suffix_idx_(suffix_idx), lookahead_idx_(lookahead_idx)
    {
    }
    
    /**
     * @brief Constructs an LR(1) item from an array of indices.
     *
     * @param arr Array containing {nterm_idx, rside_idx, suffix_idx, lookahead_idx}.
     */
    lr1_set_item(const array_type& arr)
        : nterm_idx_(arr[0]), rside_idx_(arr[1]), suffix_idx_(arr[2]), lookahead_idx_(arr[3])
    {
    }
    
    /**
     * @brief Returns an array representation of the item.
     *
     * @return Array with the item's indices.
     */
    array_type get_array() const
    {
        return {nterm_idx_, rside_idx_, suffix_idx_, lookahead_idx_};
    }
    
    /**
     * @brief Equality operator for comparing two LR(1) items.
     *
     * @param other The other item to compare.
     * @return True if all indices match, false otherwise.
     */
    bool operator == (const lr1_set_item& other) const
    {
        return nterm_idx_ == other.nterm_idx_ &&
            rside_idx_ == other.rside_idx_ &&
            suffix_idx_ == other.suffix_idx_ &&
            lookahead_idx_ == other.lookahead_idx_;
    }
    
    /**
     * @brief Equality operator for comparing with an array.
     *
     * @param arr The array to compare against.
     * @return True if the item matches the array, false otherwise.
     */
    bool operator == (const array_type& arr) const
    {
        return (*this == lr1_set_item(arr));
    }
    
    /**
     * @brief Creates a new item with the dot shifted forward by one position.
     *
     * @return The shifted LR(1) item.
     */
    lr1_set_item make_shifted() const
    {
        return lr1_set_item(nterm_idx_, rside_idx_, suffix_idx_ + 1, lookahead_idx_);
    }
};

/**
 * @brief Type alias for a vector of LR(1) items, representing an LR(1) set.
 */
using lr1_set = std::vector<lr1_set_item>;

} // namespace muskox
