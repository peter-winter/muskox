#pragma once

#include "lr1_set_item.h"
#include "lr1_sorted_set.h"
#include "ruleset.h"

namespace muskox
{
    class action
    {
    public:
        struct reduction
        {
            size_t nterm_idx_;
            size_t rside_idx_;
        };
        
        using reductions = std::vector<reduction>;
        
        action(const ruleset& rs)
            : rs_(rs), new_kernel_(lr1_set_item_comp(rs_))
        {}
        
        action& add_shift(const lr1_set_item& item)
        {
            new_kernel_.insert(item.make_shifted());
            return *this;
        }
        
        action& add_reduction(size_t nterm_idx, size_t rside_idx)
        {
            r_.push_back(reduction{nterm_idx, rside_idx});
            return *this;
        }
        
        bool has_conflict() const
        {
            return r_.size() > 1 || (r_.size() == 1 && !new_kernel_.empty());
        }
        
        bool is_one_reduction_only() const
        {
            return r_.size() == 1 && new_kernel_.empty();
        }
        
        const reduction& get_only_reduction() const
        {
            if (!is_one_reduction_only())
            {
                throw std::runtime_error("Action is not one reduction only");
            }
            return r_[0];
        }
        
        bool has_shift() const
        {
            return !new_kernel_.empty();
        }
        
        lr1_sorted_set take_new_kernel()
        {
            return std::exchange(new_kernel_, lr1_sorted_set(rs_));
        }
        
        const lr1_sorted_set& get_new_kernel() const
        {
            return new_kernel_;
        }
        
        const reductions& get_reductions() const
        {
            return r_;
        }
        
    private:
        const ruleset& rs_;
        reductions r_;
        lr1_sorted_set new_kernel_;
    };
        
    template<typename Range>
    action get_action(const ruleset& rs, const Range& group)
    {
        action result(rs);
    
        for (const auto& item : group)
        {
            size_t prod_len = rs.get_symbol_count(item.nterm_idx_, item.rside_idx_);

            if (item.suffix_idx_ == prod_len)
            {
                result.add_reduction(item.nterm_idx_, item.rside_idx_);
            }
            else
            {
                result.add_shift(item);
            }
        }

        return result;
    }
}
