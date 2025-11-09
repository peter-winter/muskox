#include "lr1_state.h"
#include "list_printer.h"

namespace muskox
{
    
lr1_state::lr1_state(const ruleset& rs, lr1_set&& kernel)
    : rs_(rs),
      items_(rs.get_lr1_set_item_space_dims()),
      kernel_(std::move(kernel)),
      comparer_(rs),
      sorted_items_(comparer_)
{
    add_items(kernel_);
}

void lr1_state::add_item(const lr1_set_item::array_type& indices)
{
    bool added = items_.add(indices);
    if (added)
    {
        sorted_items_.insert(lr1_set_item(indices));
    }
}

void lr1_state::add_items(const lr1_set& items)
{
    for (const auto& it : items)
    {
        add_item(it.get_array());
    }
}

void lr1_state::add_items(const ordered_bitset_nd<4>& items)
{
    for (const auto& indices : items.get_indices())
    {
        add_item(indices);
    }
}

lr1_state::action_map lr1_state::get_actions() const
{
    action_map result;

    for (const auto& it : sorted_items_.get_all())
    {
        size_t prod_len = rs_.get_symbol_count(it.nterm_idx_, it.rside_idx_);

        if (it.suffix_idx_ == prod_len)
        {
            // reduce
            symbol_ref key{symbol_type::terminal, it.lookahead_idx_};
            reduction red{it.nterm_idx_, it.rside_idx_};

            auto [it, inserted] = result.try_emplace(key, red);
            if (!inserted)
            {
                if (std::holds_alternative<shift>(it->second))
                {
                    conflict c{std::get<shift>(it->second), {red}};
                    it->second = std::move(c);
                }
                else if (std::holds_alternative<reduction>(it->second))
                {
                    conflict c{std::nullopt, {std::get<reduction>(it->second), red}};
                    it->second = std::move(c);
                }
                else
                {
                    auto& c = std::get<conflict>(it->second);
                    c.r_.push_back(red);
                }
            }
        }
        else
        {
            // shift
            symbol_ref key = rs_.get_symbol(it.nterm_idx_, it.rside_idx_, it.suffix_idx_);
            lr1_set_item new_kernel_item(it.nterm_idx_, it.rside_idx_, it.suffix_idx_ + 1, it.lookahead_idx_);

            auto [it, inserted] = result.try_emplace(key, shift{{new_kernel_item}});
            if (!inserted)
            {
                if (std::holds_alternative<shift>(it->second))
                {
                    auto& s = std::get<shift>(it->second);
                    s.items_.push_back(new_kernel_item);
                }
                else if (std::holds_alternative<reduction>(it->second))
                {
                    conflict c{shift{{new_kernel_item}}, {std::get<reduction>(it->second)}};
                    it->second = std::move(c);
                }
                else
                {
                    auto& c = std::get<conflict>(it->second);
                    if (!c.s_.has_value())
                    {
                        c.s_ = shift{};
                    }
                    c.s_.value().items_.push_back(new_kernel_item);
                }
            }
        }
    }

    return result;
}

const sorted_grouped_vector<lr1_set_item, lr1_set_item_comp>& lr1_state::get_sorted_items() const
{
    return sorted_items_;
}

std::string lr1_state::to_string() const
{
    return rs_.lr1_set_to_string(sorted_items_.get_all());
}

bool lr1_state::kernel_matches(const lr1_set& items) const
{
    return kernel_ == items;
}

bool lr1_state::matches(const lr1_set& items) const
{
    return sorted_items_.get_all() == items;
}

} // namespace muskox

