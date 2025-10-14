#include <lr1_state.h>

namespace ptg
{
    void lr1_state::add_item(const std::array<size_t, 4>& indices)
    {
        lr1_set_item item(indices);
        
        if (item.symbol_idx_ == rs_.get_symbol_count(item.nterm_idx_, item.rside_idx_))
        {
            end_items_per_term_[item.lookahead_idx_].push_back(indices);
        }
        else
        {
            auto ref = rs_.get_symbol(item.nterm_idx_, item.rside_idx_, item.symbol_idx_);
        
            if (ref.type_ == symbol_type::terminal)
            {
                items_per_term_[ref.index_].push_back(indices);
            }
            else
            {
                items_per_nterm_[ref.index_].push_back(indices);
            }
        }
        items_.add(indices);
    }

    void lr1_state::add_items(const index_subset<4>& c)
    {
        for (const auto& indices : c.get_indices())
        {
            add_item(indices);
        }
    }
    
    lr1_state::lr1_state(const ruleset& rs, index_subset<4>&& kernel)
        : rs_(rs),
          items_(rs.get_lr1_set_item_space_dims()),
          kernel_(std::move(kernel)),
          items_per_term_(rs.get_term_count()),
          items_per_nterm_(rs.get_nterm_count()),
          end_items_per_term_(rs.get_term_count())
    {
        add_items(kernel_);
    }
    
    const lr1_state::symbol_items& lr1_state::get_symbol_items(symbol_ref ref) const
    {
        if (ref.type_ == symbol_type::terminal)
        {
            rs_.validate_term_idx(ref.index_);
            return items_per_term_[ref.index_];
        }
        else
        {
            rs_.validate_nterm_idx(ref.index_);
            return items_per_nterm_[ref.index_];
        }
    }
    
    const lr1_state::symbol_items& lr1_state::get_end_items(size_t term_idx) const
    {
        rs_.validate_term_idx(term_idx);
        return end_items_per_term_[term_idx];
    }
    
    bool lr1_state::contains_all_items(const index_subset<4>& items) const
    {
        return items_.contains_all(items);
    }
} // namespace ptg

