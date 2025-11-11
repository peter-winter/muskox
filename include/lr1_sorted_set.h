/**
 * @file lr1_sorted_set.h
 * @brief Defines a sorted and grouped container for LR(1) set items.
 *
 * This header provides a specialization of the sorted_grouped_vector template
 * for LR(1) set items, using a partial comparer based on the symbol of interest
 * from the ruleset. It allows efficient insertion while maintaining sorted order
 * and provides a grouped view for items sharing the same partial key (symbol).
 *
 * The container is useful in LR(1) parsing for organizing items by their next
 * symbol, facilitating operations like goto transitions in state construction.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include "sorted_grouped_vector.h"
#include "lr1_set_item.h"

#include <functional>

namespace muskox
{

/**
 * @struct lr1_set_item_comp
 * @brief Partial comparer for LR(1) set items based on the symbol of interest.
 *
 * Uses the ruleset to extract the symbol after the dot in the LR(1) item
 * for comparison purposes. This enables grouping and sorting items by their
 * next symbol in the production.
 */
struct lr1_set_item_comp
{
    /**
     * @brief Constructs the comparer with a reference to the ruleset.
     *
     * @param rs The ruleset used to query symbols.
     */
    lr1_set_item_comp(const ruleset& rs)
        : rs_(std::cref(rs))
    {}
    
    /**
     * @brief Gets the partial key (symbol of interest) for comparison.
     *
     * @param it The LR(1) set item.
     * @return The symbol reference after the dot.
     */
    auto get_part(const lr1_set_item& it) const 
    {
        return rs_.get().get_symbol_of_interest(it);
    }
    
    std::reference_wrapper<const ruleset> rs_; /// Reference to the ruleset for symbol queries.
};
    
/**
 * @brief Type alias for a sorted and grouped vector of LR(1) set items.
 *
 * Specializes sorted_grouped_vector with lr1_set_item as the value type
 * and lr1_set_item_comp as the partial comparer.
 */
using lr1_sorted_set = sorted_grouped_vector<lr1_set_item, lr1_set_item_comp>;

} // namespace muskox
