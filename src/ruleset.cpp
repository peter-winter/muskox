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
      rsides_(symbols.get_nterm_count()),
      validated_(false),
      potentially_empty_rsides_(),
      appearances_in_pot_rsides_(symbols.get_nterm_count()),
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
    // Add implicit rule for $root, throw if rules for $root already exist
    if (!rsides_[0].empty())
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
    
    compute_all_suffixes_nullable();
    
    validated_ = true;
}

bool ruleset::is_validated() const
{
    return validated_;
}

symbol_ref ruleset::set_root(std::string_view name)
{
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
    return rsides_[nterm_idx].size();
}

size_t ruleset::get_max_rside_count() const
{
    size_t max = 0;
    for (const auto& nterm_rsides : rsides_)
    {
        max = std::max(max, nterm_rsides.size());
    }
    return max;
}

size_t ruleset::get_symbol_count(size_t nterm_idx, size_t rside_idx) const
{
    validate_rside_idx(nterm_idx, rside_idx);
    return rsides_[nterm_idx][rside_idx].symbols_.size();
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
    return rsides_[nterm_idx][rside_idx].symbols_[suffix_idx];
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

std::optional<size_t> ruleset::get_rside_precedence(size_t nterm_idx, size_t rside_idx) const
{
    validate_rside_idx(nterm_idx, rside_idx);
    return rsides_[nterm_idx][rside_idx].precedence_;
}

size_t ruleset::calculate_rside_precedence(size_t nterm_idx, size_t rside_idx) const
{
    validate_rside_idx(nterm_idx, rside_idx);
    
    auto r_prec = get_rside_precedence(nterm_idx, rside_idx);
    if (r_prec.has_value())
        return r_prec.value();
    
    size_t ret = 0;
    
    for (size_t i = get_symbol_count(nterm_idx, rside_idx); i-- > 0;)
    {
        auto ref = get_symbol(nterm_idx, rside_idx, i);
        if (ref.type_ == symbol_type::terminal)
        {
            auto s_prec = symbols_.get_term_prec(ref.index_);
            if (s_prec.has_value())
            {
                return s_prec.value();
            }
        }
    }
    
    return ret;
}

std::array<size_t, 3> ruleset::get_suffix_space_dims() const
{
    return {get_nterm_count(), get_max_rside_count(), get_max_symbol_count()};
}

std::array<size_t, 4> ruleset::get_lr1_set_item_space_dims() const
{
    return {get_nterm_count(), get_max_rside_count(), get_max_symbol_count() + 1, get_term_count()};
}

bool ruleset::is_suffix_nullable(size_t nterm_idx, size_t rside_idx, size_t suffix_idx) const
{
    validate_suffix_idx(nterm_idx, rside_idx, suffix_idx);
    return nullable_suffixes_.value().contains(nterm_idx, rside_idx, suffix_idx);
}

bool ruleset::is_nterm_nullable(size_t idx) const
{
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
    return ruleset_printer.print_container(rsides_, print_nterm);
}

std::string ruleset::lr1_set_item_to_string(const lr1_set_item& item) const
{
    std::string_view left = symbols_.get_nterm_name(item.nterm_idx_);
    const auto& rs = rsides_[item.nterm_idx_][item.rside_idx_];
    
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
        for (size_t pot_idx : appearances_in_pot_rsides_[nt])
        {
            auto& per = potentially_empty_rsides_[pot_idx];
            if (per.remaining_ > 0)
            {
                --per.remaining_;
                if (per.remaining_ == 0)
                {
                    size_t lhs = per.nterm_idx_;
                    if (nullable_nterms_.add(lhs))
                    {
                        q.push(lhs);
                    }
                }
            }
        }
    }
}

bool ruleset::compute_suffix_nullable(size_t nterm_idx, size_t rside_idx, size_t suffix_idx)
{
    bool ret = true;
    
    for (size_t symbol_idx = suffix_idx; symbol_idx < get_symbol_count(nterm_idx, rside_idx); ++symbol_idx)
    {
        auto ref = get_symbol(nterm_idx, rside_idx, symbol_idx);
        if (ref.type_ == symbol_type::terminal || !is_nterm_nullable(ref.index_))
        {
            ret = false;
            break;
        }
    }
    
    return ret;
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
    rsides_[lhs_idx].emplace_back(std::move(symbols), precedence);
    size_t new_rside_idx = rsides_[lhs_idx].size() - 1;

    // Incremental nullable update
    const auto& new_symbols = rsides_[lhs_idx][new_rside_idx].symbols_;
    bool all_nterms = true;
    size_t remaining = 0;
    for (const auto& sym : new_symbols)
    {
        if (sym.type_ == symbol_type::terminal)
        {
            all_nterms = false;
            break;
        }
        else
        {
            size_t nt_idx = sym.index_;
            if (!nullable_nterms_.contains(nt_idx))
            {
                ++remaining;
                appearances_in_pot_rsides_[nt_idx].push_back(potentially_empty_rsides_.size());
            }
        }
    }

    if (all_nterms)
    {
        potentially_empty_rsides_.emplace_back(potentially_empty_rside{lhs_idx, new_rside_idx, remaining});
        if (remaining == 0)
        {
            if (nullable_nterms_.add(lhs_idx))
            {
                propagate_nullable(lhs_idx);
            }
        }
    }

    return new_rside_idx;
}

void ruleset::compute_all_suffixes_nullable()
{
    nullable_suffixes_ = base_index_subset<3>(get_suffix_space_dims(), false);
    for (size_t nterm_idx = 0; nterm_idx < get_nterm_count(); ++nterm_idx)
    {
        for (size_t rside_idx = 0; rside_idx < get_nterm_rside_count(nterm_idx); ++rside_idx)
        {
            for (size_t suffix_idx = 0; suffix_idx < get_symbol_count(nterm_idx, rside_idx); ++suffix_idx)
            {
                bool is_nullable = compute_suffix_nullable(nterm_idx, rside_idx, suffix_idx);
                if (is_nullable)
                {
                    nullable_suffixes_.value().add(nterm_idx, rside_idx, suffix_idx);
                }
            }
        }
    }
}

bool ruleset::calculate_suffix_nullable(size_t nterm_idx, size_t rside_idx, size_t suffix_idx)
{
    validate_suffix_idx(nterm_idx, rside_idx, suffix_idx);
    
    if (!nullable_suffixes_.has_value())
    {
        nullable_suffixes_ = base_index_subset<3>(get_suffix_space_dims());
    }
    
    if (nullable_suffixes_.value().contains(nterm_idx, rside_idx, suffix_idx))
    {
        return true;
    }
    bool ret = compute_suffix_nullable(nterm_idx, rside_idx, suffix_idx);
    if (ret)
    {
        nullable_suffixes_.value().add(nterm_idx, rside_idx, suffix_idx);
    }
    return ret;
}

symbol_ref ruleset::set_root_impl(symbol_ref root)
{
    root_ = root;
    return root_;
}
        
} // namespace muskox
