#include <ruleset.h>

#include <grammar_error.h>

#include <stdexcept>
#include <sstream>

namespace ptg
{

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

void ruleset::validate_symbol_idx(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const
{
    validate_nterm_idx(nterm_idx);
    validate_rside_idx(nterm_idx, rside_idx);
    if (symbol_idx >= get_symbol_count(nterm_idx, rside_idx))
    {
        throw std::out_of_range("Symbol index out of range");
    }
}

symbol_ref ruleset::set_root(symbol_ref root)
{
    if (rsides_[0].empty())
    {
        symbol_list rrefs{root};
        rsides_[0].emplace_back(std::move(rrefs), 0);
    }
    else
    {
        rsides_[0][0].symbols_[0] = root;
    }
    root_ = root;
    
    return root_;
}

ruleset::ruleset(const symbol_collection& symbols)
    : symbols_(symbols), rsides_(symbols.get_nterm_count())
{
    if (symbols_.get_nterm_count() <= 1)
    {
        throw grammar_error(grammar_error::code::no_nterm);
    }
    std::string_view root_name = symbols_.get_nterm_name(1);
    root_ = set_root(root_name);
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
    
    return set_root(ref);
}

size_t ruleset::add_rule(std::string_view left, const std::vector<std::string_view>& rights, size_t precedence)
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

    rsides_[lref.index_].emplace_back(std::move(rrefs), precedence);
    return rsides_[lref.index_].size() - 1;
}

size_t ruleset::get_nterm_rside_count(size_t nterm_idx) const
{
    validate_nterm_idx(nterm_idx);
    return rsides_[nterm_idx].size();
}

size_t ruleset::get_max_rside_count() const
{
    size_t max = 0;
    for (const auto& r : rsides_)
    {
        max = std::max(max, r.size());
    }
    return max;
}

size_t ruleset::get_symbol_count(size_t nterm_idx, size_t rside_idx) const
{
    validate_rside_idx(nterm_idx, rside_idx);
    return rsides_[nterm_idx][rside_idx].symbols_.size();
}

symbol_ref ruleset::get_symbol(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const
{
    validate_symbol_idx(nterm_idx, rside_idx, symbol_idx);
    return rsides_[nterm_idx][rside_idx].symbols_[symbol_idx];
}

symbol_type ruleset::get_symbol_type(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const
{
    return get_symbol(nterm_idx, rside_idx, symbol_idx).type_;
}

size_t ruleset::get_symbol_index(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const
{
    return get_symbol(nterm_idx, rside_idx, symbol_idx).index_;
}

size_t ruleset::get_rside_precedence(size_t nterm_idx, size_t rside_idx) const
{
    validate_rside_idx(nterm_idx, rside_idx);
    return rsides_[nterm_idx][rside_idx].precedence_;
}

size_t ruleset::get_max_symbol_count() const
{
    size_t max = 0;
    for (const auto& nterm_rsides : rsides_)
    {
        for (const auto& rs : nterm_rsides)
        {
            max = std::max(max, rs.symbols_.size());
        }
    }
    return max;
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

std::string_view ruleset::get_nterm_name(size_t nterm_idx) const
{
    return symbols_.get_nterm_name(nterm_idx);
}

std::array<size_t, 3> ruleset::get_rside_part_space_dims() const
{
    return {get_nterm_count(), get_max_rside_count(), get_max_symbol_count()};
}

std::array<size_t, 4> ruleset::get_lr1_set_item_space_dims() const
{
    return {get_nterm_count(), get_max_rside_count(), get_max_symbol_count() + 1, get_term_count()};
}

std::string ruleset::to_string() const
{
    std::stringstream ss;
    for (size_t i = 0; i < symbols_.get_nterm_count(); ++i)
    {
        const auto& rside_group = rsides_[i];
        if (rside_group.empty())
        {
            continue;
        }
        
        std::string_view left = symbols_.get_nterm_name(i);

        ss << left << " : ";
        size_t indent = left.size() + 1;
        bool first = true;
        for (const auto& rs : rside_group)
        {
            if (!first)
            {
                ss << "\n" << std::string(indent, ' ') << "| ";
            }
            
            first = false;

            symbols_.print_symbol_list(ss, rs.symbols_);

            if (rs.precedence_ != 0)
            {
                if (!rs.symbols_.empty())
                {
                    ss << " ";
                }
                ss << "[" << rs.precedence_ << "]";
            }
        }
        ss << "\n" << std::string(indent, ' ') << ";\n\n";
    }
    return ss.str();
}

std::string ruleset::lr1_set_item_to_string(const lr1_set_item& item) const
{
    std::stringstream ss;
    
    std::string_view left = symbols_.get_nterm_name(item.nterm_idx_);
    ss << left << " : ";
    const auto& rs = rsides_[item.nterm_idx_][item.rside_idx_];
    symbols_.print_symbol_list_from_to(ss, rs.symbols_, 0, item.symbol_idx_);
    ss << " . ";
    symbols_.print_symbol_list_from_to(ss, rs.symbols_, item.symbol_idx_, rs.symbols_.size());
    ss << " | " << symbols_.get_term_name(item.lookahead_idx_) << "\n";
    
    return ss.str();
}
        
} // namespace ptg
