#include "closure.h"
#include "grammar_error.h"
#include "defs.h"

namespace muskox
{

closure::closure(const ruleset& rs)
    : rs_(test_ruleset_validated(rs)), 
      lr1_item_closures_(rs.get_lr1_set_item_space_dims(), std::nullopt),
      lr1_item_full_closures_(rs.get_lr1_set_item_space_dims(), std::nullopt)
{
}

const closure::opt_subset& closure::calculate(const lr1_set_item& item)
{
    auto& result = lr1_item_closures_.get(item.nterm_idx_, item.rside_idx_, item.suffix_idx_, item.lookahead_idx_);
    
    if (result.has_value())
    {
        return result;
    }
    
    result.emplace(rs_.get_lr1_set_item_space_dims());
    result.value().add(item.nterm_idx_, item.rside_idx_, item.suffix_idx_, item.lookahead_idx_);
    
    size_t rside_symbol_count = rs_.get_symbol_count(item.nterm_idx_, item.rside_idx_);
    if (item.suffix_idx_ == rside_symbol_count)
    {
        return result;
    }
    
    const auto& ref = rs_.get_symbol(item.nterm_idx_, item.rside_idx_, item.suffix_idx_);
    if (ref.type_ == symbol_type::terminal)
    {
        return result;
    }
    
    first_set terms(rs_.get_term_count());
    
    if (item.suffix_idx_ + 1 < rside_symbol_count)
    {
        const auto& suffix_first = rs_.get_suffix_first(item.nterm_idx_, item.rside_idx_, item.suffix_idx_ + 1);
        if (suffix_first.has_value())
        {
            terms.add(suffix_first.value());
        }
        
        if (rs_.is_suffix_nullable(item.nterm_idx_, item.rside_idx_, item.suffix_idx_ + 1))
        {
            terms.add(item.lookahead_idx_);
        }
    }
    else
    {
        terms.add(item.lookahead_idx_);
    }
    
    for (auto t : terms.get_indices())
    {
        for (size_t rside_idx = 0; rside_idx < rs_.get_nterm_rside_count(ref.index_); ++rside_idx)
        {
            result.value().add(ref.index_, rside_idx, 0, t);
        }
    }
    
    return result;
}

const closure::opt_subset& closure::calculate_full(const lr1_set_item& item)
{
    auto& result = lr1_item_full_closures_.get(item.nterm_idx_, item.rside_idx_, item.suffix_idx_, item.lookahead_idx_);
    
    if (result.has_value())
    {
        return result;
    }
    
    result.emplace(rs_.get_lr1_set_item_space_dims());
    result.value().add(calculate(item).value());
    
    for (size_t i = 1; i < result.value().get_count(); ++i)
    {
        lr1_set_item it(result.value().get_indices()[i]);
        result.value().add(calculate_full(it).value());
    }
    
    return result;
}

const ruleset& closure::test_ruleset_validated(const ruleset& rs) const
{
    if (!rs.is_validated())
    {
        throw std::runtime_error("Ruleset not validated");
    }
    
    if (!rs.get_errors().empty())
    {
        throw std::runtime_error("Ruleset has issues");
    }
    return rs;
}

} // namespace muskox
