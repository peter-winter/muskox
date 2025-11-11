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
    add_items(kernel_);
}

void lr1_state::add_item(const lr1_set_item& indices)
{
    bool added = items_.add(indices.get_array());
    if (added)
    {
        sorted_items_.insert(indices);
    }
}

void lr1_state::add_items(const lr1_sorted_set& items)
{
    for (const auto& it : items.get_all())
    {
        add_item(it);
    }
}

void lr1_state::add_items(const ordered_bitset_nd<4>& items)
{
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
    return kernel_.get_all() == items;
}

bool lr1_state::matches(const lr1_set& items) const
{
    return sorted_items_.get_all() == items;
}

} // namespace muskox

