#include <ruleset.h>

#include <grammar_error.h>

#include <stdexcept>
#include <sstream>

namespace ptg
{

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

    add_special_rule("$root", {name});

    return ref;
}

size_t ruleset::add_special_rule(std::string_view left, const std::vector<std::string_view>& rights, size_t precedence)
{
    symbol_ref lref = symbols_.get_symbol_ref(left);
    
    symbol_list rrefs;
    for (auto r : rights)
    {
        rrefs.push_back(symbols_.get_symbol_ref(r));
    }

    rsides_[lref.index_].emplace_back(std::move(rrefs), precedence);
    return rsides_[lref.index_].size() - 1;
}

void ruleset::validate_nterm_idx(size_t nterm_idx) const
{
    if (nterm_idx >= get_nterm_count())
    {
        throw std::out_of_range("Nterm_idx out of range");
    }
}

void ruleset::validate_rside_idx(size_t nterm_idx, size_t rside_idx) const
{
    validate_nterm_idx(nterm_idx);
    if (rside_idx >= get_nterm_rside_count(nterm_idx))
    {
        throw std::out_of_range("Rside_idx out of range");
    }
}

void ruleset::validate_symbol_idx(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const
{
    validate_nterm_idx(nterm_idx);
    validate_rside_idx(nterm_idx, rside_idx);
    if (symbol_idx >= get_symbol_count(nterm_idx, rside_idx))
    {
        throw std::out_of_range("Symbol_idx out of range");
    }
}

ruleset::ruleset(const symbol_collection& symbols, std::string_view root_name)
    : symbols_(symbols), rsides_(symbols.get_nterm_count()), root_(set_root(root_name))
{
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

size_t ruleset::get_rside_part_flat_index(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const
{
    validate_symbol_idx(nterm_idx, rside_idx, symbol_idx);

    size_t max_rsides = get_max_rside_count();
    size_t max_symbols = get_max_symbol_count();
    return nterm_idx * max_rsides * max_symbols + rside_idx * max_symbols + symbol_idx;
}

size_t ruleset::get_nterm_count() const
{
    return symbols_.get_nterm_count();
}

std::string_view ruleset::get_nterm_name(size_t nterm_idx) const
{
    return symbols_.get_nterm_name(nterm_idx);
}
    
} // namespace ptg
