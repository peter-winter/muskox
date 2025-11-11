/**
 * @file lr1_state.cpp
 * @brief Implementation of the LR(1) state class for parser construction.
 *
 * Part of the larger MuskOx project.
 */

#include "lr1_state.h"
#include "list_printer.h"

namespace muskox
{
    
lr1_state::lr1_state(const ruleset& rs, lr1_sorted_set&& kernel)
    : rs_(rs),
      items_(rs.get_lr1_set_item_space_dims()),
      kernel_(std::move(kernel)),
      sorted_items_(lr1_set_item_comp(rs_))
{
    // Initialize the state by adding the kernel items
    add_items(kernel_);
}

void lr1_state::add_item(const lr1_set_item& indices)
{
    // Attempt to add to the bitset; if new, also insert into sorted structure
    bool added = items_.add(indices.get_array());
    if (added)
    {
        sorted_items_.insert(indices);
    }
}

void lr1_state::add_items(const lr1_sorted_set& items)
{
    // Add each item from the sorted set individually
    for (const auto& it : items.get_all())
    {
        add_item(it);
    }
}

void lr1_state::add_items(const ordered_bitset_nd<4>& items)
{
    // Convert bitset indices to items and add each one
    for (const auto& indices : items.get_indices())
    {
        add_item(lr1_set_item(indices));
    }
}

const lr1_sorted_set& lr1_state::get_sorted_items() const
{
    return sorted_items_;
}

std::string lr1_state::to_string() const
{
    return rs_.lr1_set_to_string(sorted_items_.get_all());
}

bool lr1_state::kernel_matches(const lr1_sorted_set& items) const
{
    return kernel_.get_all() == items.get_all();
}

bool lr1_state::kernel_matches(const lr1_set& items) const
{
    // Compare kernel's vector with the provided items, checks order in items as well
    return kernel_.get_all() == items;
}

bool lr1_state::matches(const lr1_set& items) const
{
    return sorted_items_.get_all() == items;
}

} // namespace muskox
