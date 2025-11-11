#include "parse_table_generator.h"
#include "grammar_error.h"
#include "list_printer.h"

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
    lr1_set_item_comp comp(rs_);
    lr1_sorted_set root_kernel(std::move(comp));
    root_kernel.insert({0, 0, 0, 0});
    
    states_.emplace_back(rs_, std::move(root_kernel));
    
    for (size_t i = 0; i < states_.size(); ++i)
    {
        auto& current_state = states_[i];
        
        for (const auto& item : current_state.get_sorted_items().get_all())
        {
            const auto& cl = cl_.calculate_full(item).value();
            current_state.add_items(cl);
        }
        
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
                process_reduce(i, ref.index_, a.get_only_reduction());
            }
            else
            {
                process_shift(i, ref, a);
            }
        }
        
        for (auto&& new_kernel : std::move(new_kernels_))
        {
            states_.emplace_back(rs_, std::move(new_kernel));
        }
        new_kernels_.clear();
    }
}

std::size_t parse_table_generator::process_shift(size_t state_idx, symbol_ref ref, action& a)
{
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
    
    table_entry_hints_.emplace_back(state_idx, ref, parse_table_entry::shift(new_state_idx));
    return new_state_idx;
}

void parse_table_generator::process_reduce(size_t state_idx, size_t lookahead_idx, const action::reduction& r)
{
    table_entry_hints_.emplace_back(state_idx, symbol_ref{symbol_type::terminal, lookahead_idx}, parse_table_entry::reduce(r.nterm_idx_, r.rside_idx_));
}

std::optional<std::size_t> parse_table_generator::process_conflict(size_t state_idx, size_t term_idx, action& a)
{
    size_t r_prec_max = 0;
    size_t i_max = 0;
    std::optional<size_t> single_max_prec_reduction_idx;
    const auto& reductions = a.get_reductions();
    
    for (size_t i = 0; i < reductions.size(); ++i)
    {
        const auto& red = reductions[i];
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
    
    const auto& red = reductions[i_max];
    
    std::optional<size_t> shift_over_reduce_state_idx;
    
    bool has_shift = a.has_shift(); // store the value, process_shift changes the action object calling 'action::take_new_kernel'
    
    if (a.has_shift())
    {
        if (shift_over_reduce(term_idx, red.nterm_idx_, red.rside_idx_))
        {
            shift_over_reduce_state_idx = process_shift(state_idx, symbol_ref{symbol_type::terminal, term_idx}, a);
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
    
    collect_conflict_warnings(state_idx, term_idx, a.get_reductions(), has_shift, single_max_prec_reduction_idx, shift_over_reduce_state_idx);
    
    return shift_over_reduce_state_idx;
}

bool parse_table_generator::shift_over_reduce(size_t term_idx, size_t lhs_nterm_idx, size_t rside_idx) const
{
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
    std::optional<size_t> prefered_idx_reduce, 
    std::optional<size_t> shift_over_reduce_state_idx)
{
    bool resolved = false;
    auto name = rs_.get_term_name(lookahead_idx);
    warnings_.push_back(grammar_message(grammar_error_templates::code::conflict_intro, state_idx, name));
    std::string prods;
    
    for (size_t i = 0; i < reds.size(); ++i)
    {
        const auto& r = reds[i];
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

    if (has_shift)
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

std::optional<size_t> parse_table_generator::find_state(const lr1_sorted_set& kernel) const
{
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
    parse_table pt(rs_, state_count);

    for (const auto& hint : table_entry_hints_)
    {
        pt.get(hint.get_state_idx(), hint.get_ref()) = hint.get_entry();
    }

    return pt;
}

} // namespace muskox
