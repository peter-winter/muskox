/**
 * @file closure.cpp
 * @brief Implementation of the closure class for computing LR(1) item closures.
 *
 * Part of the larger MuskOx project.
 */

#include "closure.h"
#include "grammar_error.h"
#include "defs.h"

#include <deque>

namespace muskox
{

closure::closure(const ruleset& rs)
    : rs_(test_ruleset_validated(rs)), 
      lr1_item_full_closures_(rs.get_lr1_set_item_space_dims(), std::nullopt)
{
}

const closure::opt_subset& closure::calculate_full(const lr1_set_item& item)
{
    auto& result = lr1_item_full_closures_.get(item.nterm_idx_, item.rside_idx_, item.suffix_idx_, item.lookahead_idx_);
    
    // Memoization check: Return cached result if already computed
    if (result.has_value())
    {
        return result;
    }
    
    // Initialize the full closure subset
    result.emplace(rs_.get_lr1_set_item_space_dims());
    
    // Add the seed item to the result
    result.value().add(item.nterm_idx_, item.rside_idx_, item.suffix_idx_, item.lookahead_idx_);
    
    // Initialize worklist with the seed item, use std::deque for breadth-first search
    std::deque<lr1_set_item> worklist;
    worklist.emplace_back(item.nterm_idx_, item.rside_idx_, item.suffix_idx_, item.lookahead_idx_);
    
    // Process the worklist iteratively until no more expansions
    while (!worklist.empty())
    {
        // Pop the next item to expand
        lr1_set_item current = std::move(worklist.front());
        worklist.pop_front();
        
        // Skip if dot is at the end (no expansion possible)
        size_t rside_symbol_count = rs_.get_symbol_count(current.nterm_idx_, current.rside_idx_);
        if (current.suffix_idx_ == rside_symbol_count)
        {
            continue;
        }
        
        // Skip if the next symbol is a terminal (no non-terminal expansion)
        const auto& ref = rs_.get_symbol(current.nterm_idx_, current.rside_idx_, current.suffix_idx_);
        if (ref.type_ == symbol_type::terminal)
        {
            continue;
        }
        
        // Compute the FIRST set for lookaheads
        first_set terms(rs_.get_term_count());
        if (current.suffix_idx_ + 1 < rside_symbol_count)
        {
            const auto& suffix_first = rs_.get_suffix_first(current.nterm_idx_, current.rside_idx_, current.suffix_idx_ + 1);
            if (suffix_first.has_value())
            {
                terms.add(suffix_first.value());
            }
            
            if (rs_.is_suffix_nullable(current.nterm_idx_, current.rside_idx_, current.suffix_idx_ + 1))
            {
                terms.add(current.lookahead_idx_);
            }
        }
        else
        {
            terms.add(current.lookahead_idx_);
        }
        
        // For each possible lookahead and production of the non-terminal,
        // attempt to add the new kernel item
        for (auto t : terms.get_indices())
        {
            for (size_t rside_idx = 0; rside_idx < rs_.get_nterm_rside_count(ref.index_); ++rside_idx)
            {
                // If newly added (not duplicate), push to worklist for further expansion
                if (result.value().add(ref.index_, rside_idx, 0, t))
                {
                    worklist.emplace_back(ref.index_, rside_idx, 0, t);
                }
            }
        }
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
