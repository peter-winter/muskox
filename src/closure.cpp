#include <closure.h>
#include <grammar_error.h>

namespace muskox
{

closure::closure(const ruleset& rs)
    : firsts_(rs), 
      rs_(rs), 
      lr1_item_closures_(rs.get_lr1_set_item_space_dims(), std::nullopt),
      lr1_item_full_closures_(rs.get_lr1_set_item_space_dims(), std::nullopt)
{
}

const closure::opt_subset& closure::calculate(const lr1_set_item& item)
{
    auto& result = lr1_item_closures_.get(item.nterm_idx_, item.rside_idx_, item.symbol_idx_, item.lookahead_idx_);
    
    if (result.has_value())
    {
        return result;
    }
    
    result.emplace(rs_.get_lr1_set_item_space_dims());
    result.value().add(item.nterm_idx_, item.rside_idx_, item.symbol_idx_, item.lookahead_idx_);
    
    size_t rside_symbol_count = rs_.get_symbol_count(item.nterm_idx_, item.rside_idx_);
    if (item.symbol_idx_ == rside_symbol_count)
    {
        return result;
    }
    
    const auto& ref = rs_.get_symbol(item.nterm_idx_, item.rside_idx_, item.symbol_idx_);
    if (ref.type_ == symbol_type::terminal)
    {
        return result;
    }
    
    index_subset<1> terms(rs_.get_term_count());
    
    if (item.symbol_idx_ + 1 < rside_symbol_count)
    {
        const auto& rside_part_first = firsts_.calculate_rside_part(item.nterm_idx_, item.rside_idx_, item.symbol_idx_ + 1);
        terms.add(rside_part_first);
        
        bool rside_part_nullable = rs_.is_rside_part_nullable(item.nterm_idx_, item.rside_idx_, item.symbol_idx_ + 1);
        if (rside_part_nullable)
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
    auto& result = lr1_item_full_closures_.get(item.nterm_idx_, item.rside_idx_, item.symbol_idx_, item.lookahead_idx_);
    
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

} // namespace muskox
