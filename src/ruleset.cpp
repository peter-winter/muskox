/**
 * @file ruleset.cpp
 * @brief Implementation of the ruleset class.
 *
 * This file contains the implementation of methods for managing grammar rules,
 * including addition, validation, querying, and string conversion.
 * Rule addition incrementally updates nullable non-terminals.
 *
 * Part of the larger MuskOx project.
 */

#include "ruleset.h"
#include "list_printer.h"
#include "grammar_error.h"

#include <stdexcept>
#include <sstream>
#include <queue>

namespace muskox
{

ruleset::ruleset(const symbol_collection& symbols)
    : symbols_(symbols), 
      nterms_data_(symbols.get_nterm_count()),
      validated_(false),
      nullable_nterms_({symbols.get_nterm_count()})
{
    if (!symbols_.is_validated())
    {
        throw std::runtime_error("Symbol collection not validated");
    }
    
    set_root_impl(symbol_ref{symbol_type::non_terminal, 1}); // First user nterm
}

void ruleset::validate()
{
    validated_ = true;
    
    // Add implicit rule for $root, throw if rules for $root already exist
    if (!nterms_data_[0].rsides_.empty())
    {
        throw std::runtime_error("Unexpected $root rside");
    }
    
    add_rside_impl(0, symbol_list{root_});
    
    // Check for nonterminals with no rsides
    for (size_t i = 0; i < get_nterm_count(); ++i)
    {
        if (get_nterm_rside_count(i) == 0)
        {
            const std::string_view name = get_nterm_name(i);
            throw grammar_error(grammar_error::code::nterm_no_rsides, name);
        }
    }
    
    // calculate all effective rside precedences
    for (size_t i = 0; i < get_nterm_count(); ++i)
    {
        for (size_t j = 0; j < get_nterm_rside_count(i); ++j)
        {
            calculate_effective_rside_precedence(i, j);
        }
    }
}

bool ruleset::is_validated() const
{
    return validated_;
}

symbol_ref ruleset::set_root(std::string_view name)
{
    if (validated_)
    {
        throw std::runtime_error("Cannot set root after validation");
    }
    
    if (name[0] == '$')
    {
        throw grammar_error(grammar_error::code::cannot_refer_special, name);
    }
    
    symbol_ref ref;
    try
    {
        ref = symbols_.get_symbol_ref(name);
    }
    catch (const std::out_of_range&)
    {
        throw grammar_error(grammar_error::code::root_not_exists, name);
    }
    
    if (ref.type_ != symbol_type::non_terminal)
    {
        throw grammar_error(grammar_error::code::root_term, name);
    }
    
    return set_root_impl(ref);
}

size_t ruleset::add_rule(std::string_view left, const std::vector<std::string_view>& rights, std::optional<size_t> precedence)
{
    if (validated_)
    {
        throw std::runtime_error("Cannot add rules after validation");
    }
    
    auto [lref, rrefs] = validate_add_rule_inputs(left, rights);
    return add_rside_impl(lref.index_, std::move(rrefs), precedence);
}

symbol_ref ruleset::get_root() const
{
    return root_;
}

size_t ruleset::get_nterm_count() const
{
    return symbols_.get_nterm_count();
}

size_t ruleset::get_term_count() const
{
    return symbols_.get_term_count();
}

size_t ruleset::get_term_plus_nterm_count() const
{
    return get_term_count() + get_nterm_count();
}

size_t ruleset::get_nterm_rside_count(size_t nterm_idx) const
{
    validate_nterm_idx(nterm_idx);
    return nterms_data_[nterm_idx].rsides_.size();
}

size_t ruleset::get_max_rside_count() const
{
    size_t max = 0;
    for (const auto& nd : nterms_data_)
    {
        max = std::max(max, nd.rsides_.size());
    }
    return max;
}

size_t ruleset::get_symbol_count(size_t nterm_idx, size_t rside_idx) const
{
    validate_rside_idx(nterm_idx, rside_idx);
    return nterms_data_[nterm_idx].rsides_[rside_idx].symbols_.size();
}

size_t ruleset::get_max_symbol_count() const
{
    size_t max = 0;
    for (size_t i = 0; i < get_nterm_count(); ++i)
    {
        for (size_t j = 0; j < get_nterm_rside_count(i); ++j)
        {
            max = std::max(max, get_symbol_count(i, j));
        }
    }
    return max;
}

symbol_ref ruleset::get_symbol(size_t nterm_idx, size_t rside_idx, size_t suffix_idx) const
{
    validate_suffix_idx(nterm_idx, rside_idx, suffix_idx);
    return nterms_data_[nterm_idx].rsides_[rside_idx].symbols_[suffix_idx];
}

symbol_type ruleset::get_symbol_type(size_t nterm_idx, size_t rside_idx, size_t suffix_idx) const
{
    return get_symbol(nterm_idx, rside_idx, suffix_idx).type_;
}

size_t ruleset::get_symbol_index(size_t nterm_idx, size_t rside_idx, size_t suffix_idx) const
{
    return get_symbol(nterm_idx, rside_idx, suffix_idx).index_;
}

std::string_view ruleset::get_nterm_name(size_t nterm_idx) const
{
    validate_nterm_idx(nterm_idx);
    return symbols_.get_nterm_name(nterm_idx);
}

std::string_view ruleset::get_term_name(size_t term_idx) const
{
    validate_term_idx(term_idx);
    return symbols_.get_term_name(term_idx);
}

size_t ruleset::get_term_prec(size_t term_idx) const
{
    validate_term_idx(term_idx);
    auto prec = symbols_.get_term_prec(term_idx);
    return prec.value_or(0);
}

associativity::type ruleset::get_term_assoc(size_t term_idx) const
{
    validate_term_idx(term_idx);
    return symbols_.get_term_assoc(term_idx).get();
}

size_t ruleset::get_effective_rside_precedence(size_t nterm_idx, size_t rside_idx) const
{
    if (!validated_)
    {
        throw std::runtime_error("Cannot query effective rside precedence before validation");
    }
    
    validate_rside_idx(nterm_idx, rside_idx);
    
    return nterms_data_[nterm_idx].rsides_[rside_idx].effective_precedence_.value();
}

std::optional<size_t> ruleset::get_explicit_rside_precedence(size_t nterm_idx, size_t rside_idx) const
{
    validate_rside_idx(nterm_idx, rside_idx);
    
    return nterms_data_[nterm_idx].rsides_[rside_idx].precedence_;
}

std::array<size_t, 3> ruleset::get_suffix_space_dims() const
{
    if (!validated_)
    {
        throw std::runtime_error("Cannot query suffix space dims before validation");
    }
    
    return {get_nterm_count(), get_max_rside_count(), get_max_symbol_count()};
}

std::array<size_t, 4> ruleset::get_lr1_set_item_space_dims() const
{
    if (!validated_)
    {
        throw std::runtime_error("Cannot lr1 set item space dims before validation");
    }
    
    return {get_nterm_count(), get_max_rside_count(), get_max_symbol_count() + 1, get_term_count()};
}

bool ruleset::is_suffix_nullable(size_t nterm_idx, size_t rside_idx, size_t suffix_idx) const
{
    if (!validated_)
    {
        throw std::runtime_error("Cannot query suffix nullability before validation");
    }
    
    validate_suffix_idx(nterm_idx, rside_idx, suffix_idx);
    
    const auto& count = nterms_data_[nterm_idx].rsides_[rside_idx].potentially_nullable_suffixes_[suffix_idx];
    return count == 0;
}

bool ruleset::is_nterm_nullable(size_t idx) const
{
    if (!validated_)
    {
        throw std::runtime_error("Cannot query nterm nullability before validation");
    }
    
    validate_nterm_idx(idx);
    return nullable_nterms_.contains(idx);
}

std::string ruleset::to_string() const
{
    size_t i = 0;
    auto print_nterm = [&](const auto& nterm_rsides) -> std::string
    {
        if (nterm_rsides.empty())
        {
            ++i;
            return "";
        }
        
        std::string left(symbols_.get_nterm_name(i));
        std::string indent(left.size() + 1, ' ');
        
        auto print_rside = [&](const auto& rs)
        {
            std::string symbols_s = symbols_.print_symbol_list(rs.symbols_);
                
            std::string precedence_s;
            if (rs.precedence_.has_value())
            {
                list_printer precedence_printer("[", "", "]");
                precedence_s = precedence_printer.print_single(rs.precedence_.value());
            }
            
            list_printer rside_printer;
            return rside_printer.print_list(symbols_s, precedence_s);
        };
        
        list_printer nterm_printer(left + " : ", "\n" + indent + "| ", "\n" + indent + ";", true);
        ++i;
        return nterm_printer.print_container(nterm_rsides, print_rside);
    };
    
    list_printer ruleset_printer("", "\n\n", "");
    std::vector<std::vector<rside>> all_rsides;
    for (const auto& nd : nterms_data_)
    {
        all_rsides.push_back(nd.rsides_);
    }
    return ruleset_printer.print_container(all_rsides, print_nterm);
}

std::string ruleset::lr1_set_item_to_string(const lr1_set_item& item) const
{
    std::string_view left = symbols_.get_nterm_name(item.nterm_idx_);
    const auto& rs = nterms_data_[item.nterm_idx_].rsides_[item.rside_idx_];
    
    list_printer lp;
    
    std::string before_dot = symbols_.print_symbol_list_from_to(rs.symbols_, 0, item.symbol_idx_);
    
    std::string after_dot = symbols_.print_symbol_list_from_to(rs.symbols_, item.symbol_idx_, rs.symbols_.size());
    
    return lp.print_list(left, "->", before_dot, ".", after_dot, "/", symbols_.get_term_name(item.lookahead_idx_));
}

void ruleset::validate_term_idx(size_t term_idx) const
{
    if (term_idx >= get_term_count())
    {
        throw std::out_of_range("Term index out of range");
    }
}

void ruleset::validate_nterm_idx(size_t nterm_idx) const
{
    if (nterm_idx >= get_nterm_count())
    {
        throw std::out_of_range("Nterm index out of range");
    }
}

void ruleset::validate_rside_idx(size_t nterm_idx, size_t rside_idx) const
{
    validate_nterm_idx(nterm_idx);
    if (rside_idx >= get_nterm_rside_count(nterm_idx))
    {
        throw std::out_of_range("Rside index out of range");
    }
}

void ruleset::validate_suffix_idx(size_t nterm_idx, size_t rside_idx, size_t suffix_idx) const
{
    validate_nterm_idx(nterm_idx);
    validate_rside_idx(nterm_idx, rside_idx);
    if (suffix_idx >= get_symbol_count(nterm_idx, rside_idx))
    {
        throw std::out_of_range("Symbol index out of range");
    }
}

void ruleset::propagate_nullable(size_t nt_idx)
{
    std::queue<size_t> q;
    q.push(nt_idx);
    while (!q.empty())
    {
        size_t nt = q.front();
        q.pop();
        for (const auto& appearance : nterms_data_[nt].appearances_in_potentially_nullable_suffixes_)
        {
            auto& rside = nterms_data_[appearance.nterm_idx].rsides_[appearance.rside_idx];
            auto& remaining = rside.potentially_nullable_suffixes_[appearance.suffix_idx];
            if (remaining > 0)
            {
                --remaining;
                if (remaining == 0)
                {
                    size_t lhs = appearance.nterm_idx;
                    if (appearance.suffix_idx == 0 && nullable_nterms_.add(lhs))
                    {
                        q.push(lhs);
                    }
                }
            }
        }
    }
}

std::pair<symbol_ref, symbol_list> ruleset::validate_add_rule_inputs(std::string_view left, const std::vector<std::string_view>& rights) const
{
    if (left[0] == '$')
    {
        throw grammar_error(grammar_error::code::cannot_refer_special, left);
    }
    
    symbol_ref lref;
    try
    {
        lref = symbols_.get_symbol_ref(left);
    }
    catch (const std::out_of_range&)
    {
        throw grammar_error(grammar_error::code::lside_not_exists, left);
    }

    if (lref.type_ != symbol_type::non_terminal)
    {
        throw grammar_error(grammar_error::code::lside_term, left);
    }

    symbol_list rrefs;
    for (auto r : rights)
    {
        if (r[0] == '$')
        {
            throw grammar_error(grammar_error::code::cannot_refer_special, r);
        }
        
        try
        {
            rrefs.push_back(symbols_.get_symbol_ref(r));
        }
        catch (const std::out_of_range&)
        {
            throw grammar_error(grammar_error::code::rside_not_exist, r);
        }
    }

    return {lref, std::move(rrefs)};
}

size_t ruleset::add_rside_impl(size_t lhs_idx, symbol_list symbols, std::optional<size_t> precedence)
{
    auto& nd = nterms_data_[lhs_idx];
    nd.rsides_.emplace_back(std::move(symbols), precedence);
    size_t new_rside_idx = nd.rsides_.size() - 1;
    auto& new_rside = nd.rsides_.back();
    const auto& new_symbols = new_rside.symbols_;
    new_rside.potentially_nullable_suffixes_.resize(new_symbols.size(), std::numeric_limits<size_t>::max());
    
    // Immediately propagate, nothing to track
    if (new_symbols.empty())
    {
        if (nullable_nterms_.add(lhs_idx))
        {
            propagate_nullable(lhs_idx);
        }
        return new_rside_idx;
    }
    
    // Find the start of the trailing all-non-terminals segment
    size_t trail_start = new_symbols.size();
    while (trail_start != 0 && new_symbols[trail_start - 1].type_ != symbol_type::terminal)
    {
        --trail_start;
    }

    if (trail_start < new_symbols.size())
    {
        // Compute cumulative remaining from the end
        size_t cumulative_remaining = 0;
        for (std::size_t i = new_symbols.size(); i-- != trail_start; )
        {
            size_t suffix_idx = i;
            size_t nt_idx = new_symbols[suffix_idx].index_;
            size_t add_to_remaining = nullable_nterms_.contains(nt_idx) ? 0 : 1;
            cumulative_remaining += add_to_remaining;
            new_rside.potentially_nullable_suffixes_[suffix_idx] = cumulative_remaining;
            if (add_to_remaining > 0)
            {
                nterms_data_[nt_idx].appearances_in_potentially_nullable_suffixes_.emplace_back(suffix_ref{lhs_idx, new_rside_idx, suffix_idx});
            }
            if (cumulative_remaining == 0 && suffix_idx == 0)
            {
                if (nullable_nterms_.add(lhs_idx))
                {
                    propagate_nullable(lhs_idx);
                }
            }
        }
    }
    return new_rside_idx;
}

symbol_ref ruleset::set_root_impl(symbol_ref root)
{
    root_ = root;
    return root_;
}

size_t ruleset::calculate_effective_rside_precedence(size_t nterm_idx, size_t rside_idx)
{
    size_t ret = 0;
    
    auto& stored_effective_prec = nterms_data_[nterm_idx].rsides_[rside_idx].effective_precedence_;
    
    auto opt_explicit_prec = nterms_data_[nterm_idx].rsides_[rside_idx].precedence_;
    if (opt_explicit_prec.has_value())
    {
        ret = opt_explicit_prec.value();
    }
    else
    {
        for (size_t i = get_symbol_count(nterm_idx, rside_idx); i-- > 0;)
        {
            auto ref = get_symbol(nterm_idx, rside_idx, i);
            if (ref.type_ == symbol_type::terminal)
            {
                auto s_prec = symbols_.get_term_prec(ref.index_);
                if (s_prec.has_value())
                {
                    ret = s_prec.value();
                    break;
                }
            }
        }
    }
    
    stored_effective_prec = ret;
    return ret;
}

} // namespace muskox
