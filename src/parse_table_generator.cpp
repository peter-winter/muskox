#include "parse_table_generator.h"
#include "grammar_error.h"
#include "list_printer.h"

#include <queue>

namespace muskox
{

parse_table_generator::parse_table_generator(const ruleset& rs)
    : rs_(rs), cl_(rs)
{
    generate_states();
}

const std::vector<std::string>& parse_table_generator::get_warnings() const
{
    return warnings_;
}

void parse_table_generator::generate_states()
{
    index_subset<4> root_kernel(rs_.get_lr1_set_item_space_dims());
    root_kernel.add(0, 0, 0, 0);
    
    states_.emplace_back(rs_, std::move(root_kernel));
    
    for (size_t i = 0; i < states_.size(); ++i)
    {
        auto& current_state = states_[i];
        
        const auto& items = current_state.get_all_items().get_indices();
        for (size_t j = 0; j < items.size(); ++j)
        {
            const auto& new_items = cl_.calculate_full(lr1_set_item(items[j])).value();
            current_state.add_items(new_items);
        }
        
        lr1_state::action_map actions = current_state.get_actions();
        
        for (const auto& [ref, act] : actions)
        {
            if (std::holds_alternative<lr1_state::shift>(act))
            {
                const auto& s = std::get<lr1_state::shift>(act);
                process_shift(i, ref, s);                
            }
            else if (std::holds_alternative<lr1_state::reduction>(act))
            {
                const auto& r = std::get<lr1_state::reduction>(act);
                process_reduce(i, ref.index_, r);
            }
            else
            {
                const auto& c = std::get<lr1_state::conflict>(act);
                process_conflict(i, ref.index_, c);
            }
        }
    }
}

size_t parse_table_generator::process_shift(size_t state_idx, symbol_ref ref, const lr1_state::shift& s)
{
    index_subset<4> new_kernel(rs_.get_lr1_set_item_space_dims());
    for (const auto& item : s.items_)
    {
        new_kernel.add(item);
    }
    
    auto found = find_state(new_kernel);
    size_t new_state_idx;
    if (found.has_value())
    {
        new_state_idx = found.value();
    }
    else
    {
        new_state_idx = states_.size();
        states_.emplace_back(rs_, std::move(new_kernel));
    }
    
    table_entry_hints_.emplace_back(state_idx, ref, parse_table_entry::shift(new_state_idx));
    return new_state_idx;
}

void parse_table_generator::process_reduce(size_t state_idx, size_t lookahead_idx, const lr1_state::reduction& r)
{
    table_entry_hints_.emplace_back(state_idx, symbol_ref{symbol_type::terminal, lookahead_idx}, parse_table_entry::reduce(r.nterm_idx_, r.rside_idx_));
}

void parse_table_generator::process_conflict(size_t state_idx, size_t term_idx, const lr1_state::conflict& c)
{
    size_t r_prec_max = 0;
    size_t i_max = 0;
    std::optional<size_t> single_max_prec_reduction_idx;
    for (size_t i = 0; i < c.r_.size(); ++i)
    {
        const auto& red = c.r_[i];
        size_t r_prec = rs_.get_effective_rside_precedence(red.nterm_idx_, red.rside_idx_);
        if (r_prec > r_prec_max)
        {
            r_prec_max = r_prec;
            i_max = i;
            single_max_prec_reduction_idx = i_max;
        }
        else if (r_prec == r_prec_max)
        {
            single_max_prec_reduction_idx = std::nullopt;
        }
    }
    
    const auto& red = c.r_[i_max];
    
    std::optional<size_t> shift_over_reduce_state_idx;
    if (c.s_.has_value())
    {
        if (shift_over_reduce(term_idx, red))
        {
            shift_over_reduce_state_idx = process_shift(state_idx, symbol_ref{symbol_type::terminal, term_idx}, c.s_.value());
        }
        else
        {
            process_reduce(state_idx, term_idx, red);
        }
    }
    else
    {
        process_reduce(state_idx, term_idx, red);
    }
    
    collect_conflict_warnings(state_idx, term_idx, c, single_max_prec_reduction_idx, shift_over_reduce_state_idx);
}

bool parse_table_generator::shift_over_reduce(size_t term_idx, const lr1_state::reduction& r) const
{
    size_t s_prec = rs_.get_term_prec(term_idx);
    auto s_ass = rs_.get_term_assoc(term_idx);
    size_t r_prec = rs_.get_effective_rside_precedence(r.nterm_idx_, r.rside_idx_);
    
    return (s_prec > r_prec) || ((s_prec == r_prec) && s_ass == associativity::type::right);
}

void parse_table_generator::collect_conflict_warnings(
    size_t state_idx, 
    size_t lookahead_idx, 
    const lr1_state::conflict& c, 
    std::optional<size_t> prefered_idx_reduce, 
    std::optional<size_t> shift_over_reduce_state_idx)
{
    bool resolved = false;
    auto name = rs_.get_term_name(lookahead_idx);
    warnings_.push_back(grammar_message(grammar_error_templates::code::conflict_intro, state_idx, name));
    std::string prods;
    for (size_t i = 0; i < c.r_.size(); ++i)
    {
        const auto& r = c.r_[i];
        lr1_set_item item{r.nterm_idx_, r.rside_idx_, rs_.get_symbol_count(r.nterm_idx_, r.rside_idx_), lookahead_idx};
        std::string prod = rs_.lr1_set_item_to_string(item);
        if (!shift_over_reduce_state_idx.has_value() && prefered_idx_reduce.has_value() && prefered_idx_reduce.value() == i)
        {
            warnings_.push_back(grammar_message(grammar_error_templates::code::conflict_detail_highest_prec, prod));
            resolved = true;
        }
        else
        {
            warnings_.push_back(grammar_message(grammar_error_templates::code::conflict_detail, prod));
        }
    }
    if (c.s_.has_value())
    {
        if (shift_over_reduce_state_idx.has_value())
        {
            resolved = true;
            warnings_.push_back(grammar_message(grammar_error_templates::code::conflict_detail_shift_highest_prec, name, shift_over_reduce_state_idx.value()));
        }
        else
        {
            warnings_.push_back(grammar_message(grammar_error_templates::code::conflict_detail_shift, name));
        }
    }
    
    if (resolved)
    {
        warnings_.push_back(grammar_message(grammar_error_templates::code::conflict_resolved, state_idx, name));
    }
    else
    {
        warnings_.push_back(grammar_message(grammar_error_templates::code::conflict_unresolved, state_idx, name));
    }
}

std::optional<size_t> parse_table_generator::find_state(const index_subset<4>& kernel) const
{
    for (size_t i = 0; i < states_.size(); ++i)
    {
        if (states_[i].kernel_contains_all_items(kernel))
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
    parse_table pt(rs_, state_count);

    for (const auto& hint : table_entry_hints_)
    {
        pt.get(hint.get_state_idx(), hint.get_ref()) = hint.get_entry();
    }

    return pt;
}

} // namespace muskox
