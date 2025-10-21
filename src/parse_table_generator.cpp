#include <parse_table_generator.h>
#include <grammar_error.h>
#include <list_printer.h>

#include <queue>

namespace ptg
{

parse_table_generator::parse_table_generator(const ruleset& rs)
    : rs_(validate(rs)), cl_(rs)
{
    collect_unused_warnings();
    generate_states();
}

const ruleset& parse_table_generator::validate(const ruleset& rs) const
{
    size_t n = rs.get_nterm_count();
    
    for (size_t i = 0; i < n; ++i)
    {
        if (rs.get_nterm_rside_count(i) == 0)
        {
            const std::string_view name = rs.get_nterm_name(i);
            throw grammar_error(grammar_error::code::nterm_no_rules, name);
        }
    }
    
    return rs;
}

void parse_table_generator::collect_unused_warnings()
{
    base_index_subset<1> reachable_nterms({rs_.get_nterm_count()}, false);
    std::queue<size_t> to_visit;
    size_t start_nterm = rs_.get_root().index_;
    to_visit.push(start_nterm);
    reachable_nterms.add(start_nterm);

    base_index_subset<1> used_terms({rs_.get_term_count()}, false);

    while (!to_visit.empty())
    {
        size_t curr = to_visit.front();
        to_visit.pop();

        for (size_t r = 0; r < rs_.get_nterm_rside_count(curr); ++r)
        {
            for (size_t s = 0; s < rs_.get_symbol_count(curr, r); ++s)
            {
                auto ref = rs_.get_symbol(curr, r, s);
                if (ref.type_ == symbol_type::non_terminal)
                {
                    if (reachable_nterms.add(ref.index_))
                    {
                        to_visit.push(ref.index_);
                    }
                }
                else
                {
                    used_terms.add(ref.index_);
                }
            }
        }
    }

    // Skip $root (index 0)
    for (size_t i = 1; i < rs_.get_nterm_count(); ++i)
    {
        if (!reachable_nterms.contains(i))
        {
            std::string_view name = rs_.get_nterm_name(i);
            warnings_.push_back(grammar_warning(grammar_error_templates::code::unused_nterm, name));
        }
    }

    // Skip $eof (index 0)
    for (size_t i = 1; i < rs_.get_term_count(); ++i)
    {
        if (!used_terms.contains(i))
        {
            std::string_view name = rs_.get_term_name(i);
            warnings_.push_back(grammar_warning(grammar_error_templates::code::unused_term, name));
        }
    }
}

const std::vector<std::string>& parse_table_generator::get_warnings() const
{
    return warnings_;
}

const std::vector<std::string>& parse_table_generator::get_infos() const
{
    return infos_;
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
                process_shift(i, ref.index_, s);                
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

size_t parse_table_generator::process_shift(size_t /*state_idx*/, size_t /*term_idx*/, const lr1_state::shift& s)
{
    index_subset<4> new_kernel(rs_.get_lr1_set_item_space_dims());
    for (const auto& item : s.items_)
    {
        new_kernel.add(item);
    }
    
    auto found = find_state(new_kernel);
    if (found.has_value())
    {
        return found.value();
    }
    else
    {
        auto s = states_.size();
        states_.emplace_back(rs_, std::move(new_kernel));
        return s;
    }
}

void parse_table_generator::process_reduce(size_t /*state_idx*/, size_t /*lookahead_idx*/, const lr1_state::reduction& /*r*/)
{
}

void parse_table_generator::process_conflict(size_t state_idx, size_t term_idx, const lr1_state::conflict& c)
{
    const lr1_state::reduction& red = c.r_[0]; // ignore more than one reduction for now
    
    if (c.s_.has_value())
    {
        std::optional<size_t> s_prec = rs_.get_term_prec(term_idx);
        auto s_ass = rs_.get_term_assoc(term_idx);
        size_t r_prec = rs_.calculate_rside_precedence(red.nterm_idx_, red.rside_idx_);
        if (
              s_prec.has_value() && 
              (
                  (s_prec.value() > r_prec) || 
                  ((s_prec.value() == r_prec) && s_ass == associativity::type::right)
              )
           )
        {
            size_t shifted_state_idx = process_shift(state_idx, term_idx, c.s_.value());
            collect_sr_conflict_info_resolution_shift(state_idx, term_idx, shifted_state_idx);
        }
        else
        {
            process_reduce(state_idx, term_idx, red);
            collect_sr_conflict_info_resolution_reduce(state_idx, term_idx, red.nterm_idx_, red.rside_idx_);
        }
        
        return;
    }
    
    // rr conflict
    if (!c.s_.has_value() && c.r_.size() > 1)
    {
        collect_rr_conflict_warning(state_idx, term_idx, c);
    }
    process_reduce(state_idx, term_idx, red);
}

void parse_table_generator::collect_rr_conflict_warning(size_t state_idx, size_t lookahead_idx, const lr1_state::conflict& c)
{
    std::string prods;
    for (const auto& r : c.r_)
    {
        lr1_set_item item{r.nterm_idx_, r.rside_idx_, rs_.get_symbol_count(r.nterm_idx_, r.rside_idx_), lookahead_idx};
        prods += "\n    " + rs_.lr1_set_item_to_string(item);
    }
    
    auto name = rs_.get_term_name(lookahead_idx);
    warnings_.push_back(grammar_warning(grammar_error_templates::code::rr_conflict, state_idx, name, prods));
}

void parse_table_generator::collect_sr_conflict_info_resolution_reduce(size_t state_idx, size_t lookahead_idx, size_t nterm_idx, size_t rside_idx)
{
    auto name = rs_.get_term_name(lookahead_idx);
    lr1_set_item item{nterm_idx, rside_idx, rs_.get_symbol_count(nterm_idx, rside_idx), lookahead_idx};
    auto prod = rs_.lr1_set_item_to_string(item);
    infos_.push_back(grammar_warning(grammar_error_templates::code::sr_conflict_resolved_reduce, state_idx, name, prod));
}

void parse_table_generator::collect_sr_conflict_info_resolution_shift(size_t state_idx, size_t term_idx, size_t shifted_state_idx)
{
    auto name = rs_.get_term_name(term_idx);
    infos_.push_back(grammar_warning(grammar_error_templates::code::sr_conflict_resolved_shift, state_idx, name, shifted_state_idx));
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

} // namespace ptg
