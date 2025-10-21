#include <lr1_state.h>
#include <list_printer.h>

namespace ptg
{
    
void lr1_state::add_items(const index_subset<4>& c)
{
    items_.add(c);
}

lr1_state::lr1_state(const ruleset& rs, index_subset<4>&& kernel)
    : rs_(rs),
      items_(rs.get_lr1_set_item_space_dims()),
      kernel_(std::move(kernel))
{
    add_items(kernel_);
}

bool lr1_state::contains_all_items(const index_subset<4>& items) const
{
    return items_.contains_all(items);
}

const index_subset<4>& lr1_state::get_all_items() const
{
    return items_;
}

bool lr1_state::kernel_contains_all_items(const index_subset<4>& items) const
{
    return kernel_.contains_all(items);
}

const index_subset<4>& lr1_state::get_kernel() const
{
    return kernel_;
}

lr1_state::action_map lr1_state::get_actions() const
{
    action_map result;

    for (const auto& arr : items_.get_indices())
    {
        size_t nterm_idx = arr[0];
        size_t rside_idx = arr[1];
        size_t symbol_idx = arr[2];
        size_t lookahead_idx = arr[3];

        size_t prod_len = rs_.get_symbol_count(nterm_idx, rside_idx);

        if (symbol_idx == prod_len)
        {
            // reduce
            symbol_ref key{symbol_type::terminal, lookahead_idx};
            reduction red{nterm_idx, rside_idx};

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
            symbol_ref key = rs_.get_symbol(nterm_idx, rside_idx, symbol_idx);
            std::array<size_t, 4> new_kernel_item{nterm_idx, rside_idx, symbol_idx + 1, lookahead_idx};

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

std::string lr1_state::to_string() const
{
    auto to_string_f = [&](const std::array<size_t, 4>& arr) {
        return rs_.lr1_set_item_to_string(lr1_set_item(arr));
    };

    list_printer lp("", "\n", "");
    return lp.print_container(items_.get_indices(), to_string_f);
}

bool lr1_state::kernel_matches(const index_subset<4>& other) const
{
    return kernel_.contains_only_items(other);
}

bool lr1_state::matches(const index_subset<4>& other) const
{
    return items_.contains_only_items(other);
}

} // namespace ptg

