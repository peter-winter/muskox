/**
 * @file parse_table_generator.cpp
 * @brief Implementation of the LR(1) parse table generator class.
 *
 * Part of the larger MuskOx project.
 */

#include "parse_table_generator.h"
#include "grammar_error.h"
#include "list_printer.h"

namespace muskox
{

parse_table_generator::parse_table_generator(const ruleset& rs)
    : rs_(rs), cl_(rs)
{
    // Generate the LR(1) states upon construction
    generate_states();
}

const std::vector<std::string>& parse_table_generator::get_warnings() const
{
    return warnings_;
}

void parse_table_generator::generate_states()
{
    lr1_set_item_comp comp(rs_);
    lr1_sorted_set root_kernel(std::move(comp));
    // Add the initial item for the root production
    root_kernel.insert({0, 0, 0, 0});
    
    // Start with the initial state
    states_.emplace_back(rs_, std::move(root_kernel));
    
    // Process each state
    for (size_t i = 0; i < states_.size(); ++i)
    {
        auto& current_state = states_[i];
        
        // Compute closures for all items in the state
        for (const auto& item : current_state.get_sorted_items().get_all())
        {
            const auto& cl = cl_.calculate_full(item).value();
            current_state.add_items(cl);
        }
        
        // Process each group of items by symbol
        for (const auto& group : current_state.get_sorted_items().grouped_view())
        {
            symbol_ref ref = rs_.get_symbol_of_interest(group.front());
            
            auto a = get_action(rs_, group);
            
            if (a.has_conflict())
            {
                process_conflict(i, ref.index_, a);
            }
            else if (a.is_one_reduction_only())
            {
                process_single_reduce(i, ref.index_, a.get_only_reduction());
            }
            else
            {
                process_shift(i, ref, a);
            }
        }
        
        // Add pending kernels as new states
        for (auto&& new_kernel : std::move(new_kernels_))
        {
            states_.emplace_back(rs_, std::move(new_kernel));
        }
        new_kernels_.clear();
    }
}

std::size_t parse_table_generator::process_shift(size_t state_idx, symbol_ref ref, action& a)
{
    // Find or create new state for shift
    auto found = find_state(a.get_new_kernel());
    size_t new_state_idx;
    if (found.has_value())
    {
        new_state_idx = found.value();
    }
    else
    {
        new_state_idx = states_.size() + new_kernels_.size();
        new_kernels_.emplace_back(a.take_new_kernel());
    }
    
    // Add shift entry hint
    table_entry_hints_.emplace_back(state_idx, ref, parse_table_entry::shift(new_state_idx));
    return new_state_idx;
}

void parse_table_generator::process_reduce(size_t state_idx, size_t lookahead_idx, const std::vector<size_t>& all_max_precedence_reductions, const action& a)
{
    if (all_max_precedence_reductions.size() == 1)
    {
        // single unique max precedence reduction
        process_single_reduce(state_idx, lookahead_idx, a.get_reductions()[all_max_precedence_reductions[0]]);
    }
    else
    {
        // Handle reduce-reduce conflict
        process_rr_conflict(state_idx, lookahead_idx, all_max_precedence_reductions, a);
    }
}

void parse_table_generator::process_single_reduce(size_t state_idx, size_t lookahead_idx, const action::reduction& r)
{
    // Add reduce entry hint
    table_entry_hints_.emplace_back(state_idx, symbol_ref{symbol_type::terminal, lookahead_idx}, parse_table_entry::reduce(r.nterm_idx_, r.rside_idx_));
}

std::optional<std::size_t> parse_table_generator::process_conflict(size_t state_idx, size_t term_idx, action& a)
{
    const auto& reductions = a.get_reductions();
    
    // Determine highest precedence reduction
    // Put all max precedence reductions into a collection of indices into 'reductions'
    // If no single unique highest precedence reduction, then i_max is first in order of declarations
    size_t r_prec_max = 0;
    size_t i_max = 0;
    
    std::vector<std::size_t> all_max_precedence_reductions;
    all_max_precedence_reductions.reserve(reductions.size());
    
    for (size_t i = 0; i < reductions.size(); ++i)
    {
        const auto& red = reductions[i];
        size_t r_prec = rs_.get_effective_rside_precedence(red.nterm_idx_, red.rside_idx_);
        if (r_prec > r_prec_max)
        {
            r_prec_max = r_prec;
            i_max = i;
            all_max_precedence_reductions.clear();
            all_max_precedence_reductions.push_back(i);
        }
        else if (r_prec == r_prec_max)
        {
            all_max_precedence_reductions.push_back(i);
        }
    }
    
    // one of the max precedence reductions (first in declaration order)
    const auto& first_in_order_max_precedence_reduction = reductions[i_max];
    
    std::optional<size_t> shift_over_reduce_state_idx;
    
    bool has_shift = a.has_shift(); // store the value, process_shift changes the action object calling 'action::take_new_kernel'
    
    if (a.has_shift())
    {
        // Handle shift-reduce conflict
        if (shift_over_reduce(term_idx, first_in_order_max_precedence_reduction.nterm_idx_, first_in_order_max_precedence_reduction.rside_idx_))
        {
            // Prefer shift based on precedence/associativity
            // If shift has higher precedence over all of the reductions, prefer it
            shift_over_reduce_state_idx = process_shift(state_idx, symbol_ref{symbol_type::terminal, term_idx}, a);
        }
        else
        {
            // Prefer reduction, may be single or reduce-reduce conflict
            process_reduce(state_idx, term_idx, all_max_precedence_reductions, a);
        }
    }
    else
    {
        // No shift, so single reduce or reduce-reduce conflict
        process_reduce(state_idx, term_idx, all_max_precedence_reductions, a);
    }
    
    // Collect warnings for the conflict
    collect_conflict_warnings(state_idx, term_idx, a.get_reductions(), has_shift, all_max_precedence_reductions, shift_over_reduce_state_idx);
    
    return shift_over_reduce_state_idx;
}

void parse_table_generator::process_rr_conflict(size_t state_idx, size_t lookahead_idx, const std::vector<size_t>& all_max_precedence_reductions, const action& a)
{
    const auto& reds = a.get_reductions();
    size_t current_rr_conflict_hints_size = rr_conflict_hints_.size();
    
    for (size_t i : all_max_precedence_reductions)
    {
        const auto& r = reds[i];
        rr_conflict_hints_.push_back(r);
    }
 
    table_entry_hints_.emplace_back(
        state_idx, 
        symbol_ref{symbol_type::terminal, lookahead_idx}, 
        parse_table_entry::rr_conflict(current_rr_conflict_hints_size, all_max_precedence_reductions.size())
    );   
}

bool parse_table_generator::shift_over_reduce(size_t term_idx, size_t lhs_nterm_idx, size_t rside_idx) const
{
    // Decide if shift should be preferred over reduce based on precedence and associativity
    size_t s_prec = rs_.get_term_prec(term_idx);
    auto s_ass = rs_.get_term_assoc(term_idx);
    size_t r_prec = rs_.get_effective_rside_precedence(lhs_nterm_idx, rside_idx);
    
    return (s_prec > r_prec) || ((s_prec == r_prec) && s_ass == associativity::type::right);
}

void parse_table_generator::collect_conflict_warnings(
    size_t state_idx, 
    size_t lookahead_idx, 
    const action::reductions& reds, 
    bool has_shift,
    const std::vector<size_t>& all_max_precedence_reductions,
    std::optional<size_t> shift_over_reduce_state_idx)
{
    bool resolved = false;
    auto name = rs_.get_term_name(lookahead_idx);
    
    // Intro warning for conflict
    warnings_.push_back(grammar_message(grammar_error_templates::code::conflict_intro, state_idx, name));
    std::string prods;
    
    std::optional<size_t> prefered_idx_reduce = std::nullopt;
    if (all_max_precedence_reductions.size() == 1)
    {
        prefered_idx_reduce = all_max_precedence_reductions[0];
    }
    
    for (size_t i = 0; i < reds.size(); ++i)
    {
        const auto& r = reds[i];
        lr1_set_item item{r.nterm_idx_, r.rside_idx_, rs_.get_symbol_count(r.nterm_idx_, r.rside_idx_), lookahead_idx};
        std::string prod = rs_.lr1_set_item_to_string(item);
        if (!shift_over_reduce_state_idx.has_value() && prefered_idx_reduce.has_value() && prefered_idx_reduce.value() == i)
        {
            // Note highest precedence reduction
            warnings_.push_back(grammar_message(grammar_error_templates::code::conflict_detail_highest_prec, prod));
            resolved = true;
        }
        else
        {
            // Detail for reduction
            warnings_.push_back(grammar_message(grammar_error_templates::code::conflict_detail, prod));
        }
    }

    if (has_shift)
    {
        if (shift_over_reduce_state_idx.has_value())
        {
            resolved = true;
            // Note shift with highest precedence
            warnings_.push_back(grammar_message(grammar_error_templates::code::conflict_detail_shift_highest_prec, name, shift_over_reduce_state_idx.value()));
        }
        else
        {
            // Detail for shift
            warnings_.push_back(grammar_message(grammar_error_templates::code::conflict_detail_shift, name));
        }
    }
    
    if (resolved)
    {
        // Conflict resolved
        warnings_.push_back(grammar_message(grammar_error_templates::code::conflict_resolved, state_idx, name));
    }
    else
    {
        // Conflict unresolved
        warnings_.push_back(grammar_message(grammar_error_templates::code::conflict_unresolved, state_idx, name));
    }
}

std::optional<size_t> parse_table_generator::find_state(const lr1_sorted_set& kernel) const
{
    // Search for existing state with matching kernel
    for (size_t i = 0; i < states_.size(); ++i)
    {
        if (states_[i].kernel_matches(kernel))
        {
            return i;
        }
    }
    
    return std::nullopt;
}

std::string parse_table_generator::states_to_string() const
{
    auto to_string_f = [&](const auto& st) {
        return st.to_string();
    };

    list_printer lp("", "\n\n", "");
    return lp.print_container(states_, to_string_f);
}

const std::vector<lr1_state>& parse_table_generator::get_states() const
{
    return states_;
}

const std::vector<table_entry_hint>& parse_table_generator::get_table_entry_hints() const
{
    return table_entry_hints_;
}

parse_table parse_table_generator::create_parse_table() const
{
    size_t state_count = states_.size();
    parse_table pt(rs_, state_count, rr_conflict_hints_.size());

    // Populate the parse table from hints
    for (const auto& hint : table_entry_hints_)
    {
        pt.get(hint.get_state_idx(), hint.get_ref()) = hint.get_entry();
    }

    // Populate the reduce-reduce conflicts table from hints
    for (size_t i = 0; i < rr_conflict_hints_.size(); ++i)
    {
        const auto& hint = rr_conflict_hints_[i];
        pt.get_rr_conflict(i) = parse_table_entry::reduction{static_cast<uint16_t>(hint.nterm_idx_), static_cast<uint16_t>(hint.rside_idx_)};
    }
    
    return pt;
}

} // namespace muskox
