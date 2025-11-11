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
#include <ranges>

namespace muskox
{

ruleset::ruleset(const symbol_collection& symbols)
    : symbols_(test_symbol_collection_validated(symbols)),
      nterms_data_(symbols.get_nterm_count()),
      validated_(false),
      nullable_nterms_({symbols.get_nterm_count()})
{
    set_root_impl(symbol_ref{symbol_type::non_terminal, 1}); // First user nterm
}

size_t ruleset::validate()
{
    if (validated_)
    {
        throw std::runtime_error("Cannot validate twice");
    }
    
    validated_ = true;
    
    check_nterm_no_rsides();
    check_usused_symbols();
    check_unsolvable_nterms();
    
    // Add implicit rule for $root, throw if rules for $root already exist
    if (!nterms_data_[0].rsides_.empty())
    {
        throw std::runtime_error("Unexpected $root rside");
    }
    
    // Don't add $eof as terminal at the end, it is rather a lookahead symbol in LR(1) parsing root item
    add_rside_impl(0, symbol_list{root_});
        
    // Calculate all effective rside precedences
    for (size_t i = 0; i < get_nterm_count(); ++i)
    {
        for (size_t j = 0; j < get_nterm_rside_count(i); ++j)
        {
            calculate_effective_rside_precedence(i, j);
        }
    }
    
    return errors_.size();
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

symbol_ref ruleset::get_symbol_of_interest(const lr1_set_item& it) const
{
    if (it.suffix_idx_ < get_symbol_count(it.nterm_idx_, it.rside_idx_))
    {
        return get_symbol(it.nterm_idx_, it.rside_idx_, it.suffix_idx_);
    }
    else
    {
        return symbol_ref{symbol_type::terminal, it.lookahead_idx_};
    }
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
        throw std::runtime_error("Cannot query lr1 set item space dims before validation");
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

bool ruleset::is_nterm_nullable(size_t nterm_idx) const
{
    if (!validated_)
    {
        throw std::runtime_error("Cannot query nterm nullability before validation");
    }
    
    validate_nterm_idx(nterm_idx);
    return nullable_nterms_.contains(nterm_idx);
}

const std::optional<first_set>& ruleset::get_suffix_first(size_t nterm_idx, size_t rside_idx, size_t suffix_idx) const
{
    if (!validated_)
    {
        throw std::runtime_error("Cannot query suffix FIRST sets before validation");
    }
    
    validate_suffix_idx(nterm_idx, rside_idx, suffix_idx);
    
    return nterms_data_[nterm_idx].rsides_[rside_idx].first_[suffix_idx];
}
    
const first_set& ruleset::get_nterm_first(size_t nterm_idx) const
{
    if (!validated_)
    {
        throw std::runtime_error("Cannot query nterm FIRST sets before validation");
    }
    
    validate_nterm_idx(nterm_idx);
    const auto& first = nterms_data_[nterm_idx].first_;
    
    if (!first.has_value())
    {
        throw std::runtime_error("No FIRST set for non-terminal");
    }
    
    return first.value();
}
    
std::string ruleset::to_string() const
{
    auto nterm_view = 
        std::views::iota(size_t(0), get_nterm_count()) |
        std::views::transform([this](size_t i){ return std::pair{get_nterm_name(i), nterms_data_[i].rsides_}; });

    auto print_nterm = [&](const auto& p) -> std::string
    {
        const auto& [left, nterm_rsides] = p;
        if (nterm_rsides.empty())
        {
            return "";
        }
        
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
        
        std::string lhs_string{left};
        lhs_string += " : ";
        list_printer nterm_printer(lhs_string, "\n" + indent + "| ", "\n" + indent + ";", true);
        return nterm_printer.print_container(nterm_rsides, print_rside);
    };
    
    list_printer ruleset_printer("", "\n\n", "");
    return ruleset_printer.print_range(nterm_view, print_nterm);
}

std::string ruleset::lr1_set_item_to_string(const lr1_set_item& item) const
{
    std::string_view left = symbols_.get_nterm_name(item.nterm_idx_);
    const auto& rs = nterms_data_[item.nterm_idx_].rsides_[item.rside_idx_];
    
    list_printer lp;
    
    std::string before_dot = symbols_.print_symbol_list_from_to(rs.symbols_, 0, item.suffix_idx_);
    
    std::string after_dot = symbols_.print_symbol_list_from_to(rs.symbols_, item.suffix_idx_, rs.symbols_.size());
    
    return lp.print_list(left, "->", before_dot, ".", after_dot, "/", symbols_.get_term_name(item.lookahead_idx_));
}

std::string ruleset::lr1_set_to_string(const lr1_set& s) const
{
    auto to_string_f = [&](const lr1_set_item& it) {
        return lr1_set_item_to_string(it);
    };

    list_printer lp("", "\n", "");
    return lp.print_container(s, to_string_f);
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

const std::vector<std::string>& ruleset::get_errors() const
{
    return errors_;
}

const std::vector<std::string>& ruleset::get_warnings() const
{
    return warnings_;
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
            auto& rs = nterms_data_[appearance.nterm_idx_].rsides_[appearance.rside_idx_];
            auto& remaining = rs.potentially_nullable_suffixes_[appearance.suffix_idx_];
            if (remaining > 0)
            {
                --remaining;
                if (remaining == 0)
                {
                    size_t lhs = appearance.nterm_idx_;
                    if (appearance.suffix_idx_ == 0 && nullable_nterms_.add(lhs))
                    {
                        q.push(lhs);
                    }
                }
            }
        }
        
        for (const auto& appearance : nterms_data_[nt].appearances_at_start_of_suffixes_)
        {
            auto& rs = nterms_data_[appearance.nterm_idx_].rsides_[appearance.rside_idx_];
            std::optional<first_set>& suffix_first = rs.first_[appearance.suffix_idx_];
            size_t suffix = appearance.suffix_idx_ + 1;
            while (suffix < rs.symbols_.size())
            {
                symbol_ref next = rs.symbols_[suffix];
                if (next.type_ == symbol_type::non_terminal)
                {
                    // Now the next non-terminal after nullable 'appears' at the start of suffix
                    nterms_data_[next.index_].appearances_at_start_of_suffixes_.emplace_back(appearance);
                    
                    // Union FIRST set from next non-terminal after nullable into this suffix FIRST set
                    first_set_add_with_lazy_init(suffix_first, nterms_data_[next.index_].first_);
                                    
                    if (!nullable_nterms_.contains(next.index_))
                    {
                        break;
                    }
                }
                else
                {
                    // Add terminal after nullable into suffix FIRST
                    first_set_add_with_lazy_init(suffix_first, next.index_);
                    
                    if (appearance.suffix_idx_ == 0 && first_set_add_with_lazy_init(nterms_data_[appearance.nterm_idx_].first_, next.index_))
                    {
                        // Started from suffix == 0, so we are analyzing full rside, if newly added then propagate
                        propagate_added_to_first_set(appearance.nterm_idx_, next.index_);
                    }
                    
                    break;
                }
                suffix++;
            }
        }
    }
}

void ruleset::propagate_added_to_first_set(size_t nt_idx, size_t t_idx)
{
    std::queue<std::pair<size_t, size_t>> q;
    q.push({nt_idx, t_idx});
    while (!q.empty())
    {
        auto [nt, t] = q.front();
        q.pop();
        
        for (const auto& appearance : nterms_data_[nt].appearances_at_start_of_suffixes_)
        {
            auto& rs = nterms_data_[appearance.nterm_idx_].rsides_[appearance.rside_idx_];
            first_set_add_with_lazy_init(rs.first_[appearance.suffix_idx_], t);
            
            if (appearance.suffix_idx_ == 0 && first_set_add_with_lazy_init(nterms_data_[appearance.nterm_idx_].first_, t))
            {
                // Suffix == 0, so we are analyzing full rside, if newly added then propagate
                q.push({appearance.nterm_idx_, t});
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
    
    // Initialize with sentinel value to indicate non-nullable, 0 value means nullable, >0 means undetermined yet
    new_rside.potentially_nullable_suffixes_.resize(new_symbols.size(), std::numeric_limits<size_t>::max());
    
    // Initialize with nullopt for lazy init
    new_rside.first_.resize(new_symbols.size(), std::nullopt);
    
    // Immediately propagate nullable non-terminal, nothing to track
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
        for (std::size_t suffix_idx = new_symbols.size(); suffix_idx-- != trail_start; )
        {
            size_t nt_idx = new_symbols[suffix_idx].index_;
            size_t add_to_remaining = nullable_nterms_.contains(nt_idx) ? 0 : 1;
            cumulative_remaining += add_to_remaining;
            new_rside.potentially_nullable_suffixes_[suffix_idx] = cumulative_remaining;
            if (add_to_remaining > 0)
            {
                // Add appearance only if remaining undetermined non-terminals in suffix
                nterms_data_[nt_idx].appearances_in_potentially_nullable_suffixes_.emplace_back(suffix_ref{lhs_idx, new_rside_idx, suffix_idx});
            }
            // Propagate if suffix==0 (whole rside) and no undetermined remaining
            if (cumulative_remaining == 0 && suffix_idx == 0)
            {
                if (nullable_nterms_.add(lhs_idx))
                {
                    propagate_nullable(lhs_idx);
                }
            }
        }
    }
    
    size_t nullable_streak = 0;
    for (size_t suffix_idx = 0; suffix_idx < new_symbols.size(); ++suffix_idx)
    {
        const symbol_ref& ref = new_symbols[suffix_idx];
        std::optional<first_set>& lhs_first = nterms_data_[lhs_idx].first_;
        
        if (ref.type_ == symbol_type::terminal)
        {
            for (size_t i = suffix_idx - nullable_streak; i <= suffix_idx; ++i)
            {
                first_set_add_with_lazy_init(new_rside.first_[i], ref.index_);
            }
            
            // For terminal at the start of whole rside, considering nullable streak at the start of suffix, 
            // immediately add to lhs non-terminal FIRST, propagate if newly added
            if (suffix_idx == nullable_streak && first_set_add_with_lazy_init(lhs_first, ref.index_))
            {
                propagate_added_to_first_set(lhs_idx, ref.index_);
            }
        }
        else
        {
            // Union FIRST from non-terminal at the start of suffix, considering nullable streak at the start of suffix, 
            // then also union into lhs non-terminal if streak is from the very start
            
            for (size_t i = suffix_idx - nullable_streak; i <= suffix_idx; ++i)
            {
                first_set_add_with_lazy_init(new_rside.first_[i], nterms_data_[ref.index_].first_);
            }
            
            if (suffix_idx == nullable_streak)
            {
                size_t old_size = lhs_first.has_value() ? lhs_first.value().get_count() : 0;
                size_t new_size = first_set_add_with_lazy_init(lhs_first, nterms_data_[ref.index_].first_);
                
                // Propagate all newly added during union
                for (size_t i = old_size; i < new_size; ++i)
                {
                    propagate_added_to_first_set(lhs_idx, lhs_first.value().get_indices()[i]);
                }
            }
            
            if (nullable_nterms_.contains(ref.index_))
            {
                nullable_streak++;
            }
            else
            {
                nullable_streak = 0;
            }
            
            // Only for immediate start of suffix, if non-terminals conclude to nullable then add appearances for >0 posittions in suffix
            nterms_data_[ref.index_].appearances_at_start_of_suffixes_.emplace_back(suffix_ref{lhs_idx, new_rside_idx, suffix_idx});
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

bool ruleset::first_set_add_with_lazy_init(std::optional<first_set>& opt, size_t t_idx)
{
    if (!opt.has_value())
    {
        opt.emplace(get_term_count());
    }
    return opt.value().add(t_idx);
}

size_t ruleset::first_set_add_with_lazy_init(std::optional<first_set>& opt, const std::optional<first_set>& other)
{
    if (other.has_value())
    {
        if (!opt.has_value())
        {
            opt.emplace(get_term_count());
        }
        opt.value().add(other.value());
    }
    
    return opt.has_value() ? opt.value().get_count() : 0;
}

const symbol_collection& ruleset::test_symbol_collection_validated(const symbol_collection& sc) const
{
    if (!sc.is_validated())
    {
        throw std::runtime_error("Symbol collection not validated");
    }
    
    if (sc.get_errors().size() != 0)
    {
        throw std::runtime_error("Symbol collection has issues");
    }
    return sc;
}

void ruleset::check_nterm_no_rsides()
{
    // Start from 1, don't check $root
    for (size_t i = 1; i < get_nterm_count(); ++i)
    {
        if (get_nterm_rside_count(i) == 0)
        {
            const std::string_view name = get_nterm_name(i);
            errors_.push_back(grammar_message(grammar_error::code::nterm_no_rsides, name));
        }
    }
}

void ruleset::check_usused_symbols()
{
    bitset_nd<1> reachable_nterms({get_nterm_count()}, false);
    std::queue<size_t> to_visit;
    to_visit.push(root_.index_);
    reachable_nterms.add(root_.index_);

    bitset_nd<1> used_terms({get_term_count()}, false);

    while (!to_visit.empty())
    {
        size_t curr = to_visit.front();
        to_visit.pop();

        for (size_t r = 0; r < get_nterm_rside_count(curr); ++r)
        {
            for (size_t s = 0; s < get_symbol_count(curr, r); ++s)
            {
                auto ref = get_symbol(curr, r, s);
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
    for (size_t i = 1; i < get_nterm_count(); ++i)
    {
        if (!reachable_nterms.contains(i))
        {
            std::string_view name = get_nterm_name(i);
            warnings_.push_back(grammar_message(grammar_error_templates::code::unused_nterm, name));
        }
    }

    // Skip $eof (index 0)
    for (size_t i = 1; i < get_term_count(); ++i)
    {
        if (!used_terms.contains(i))
        {
            std::string_view name = get_term_name(i);
            warnings_.push_back(grammar_message(grammar_error_templates::code::unused_term, name));
        }
    }
}

void ruleset::check_unsolvable_nterms()
{
    for (size_t i = 1; i < get_nterm_count(); ++i)
    {
        // If doesn't have FIRST set and is not pure epsilon non-terminal
        if (!nterms_data_[i].first_.has_value() && 
            !nterms_data_[i].rsides_.empty() &&     // No productions, this is reported as such, not unsolvable
            !(nterms_data_[i].rsides_.size() == 1 && nterms_data_[i].rsides_[0].symbols_.empty()))
        {
            std::string_view name = get_nterm_name(i);
            errors_.push_back(grammar_message(grammar_error_templates::code::nterm_unsolvable, name));
        }
    }
}

} // namespace muskox
