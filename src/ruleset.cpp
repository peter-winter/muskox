#include <ruleset.h>

#include <grammar_error.h>

#include <stdexcept>
#include <sstream>

namespace ptg
{

ruleset::ruleset(const symbol_collection& symbols, std::string_view root_name)
    : symbols_(symbols), rsides_(symbols.get_nterm_count()), root_(set_root(root_name))
{
}

void ruleset::add_rule_impl(std::string_view left, const std::vector<std::string_view>& rights, size_t precedence)
{
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

size_t ruleset::get_nterm_count() const
{
    return symbols_.get_nterm_count();
}

size_t ruleset::get_nterm_rside_count(size_t nterm_idx) const
{
    if (nterm_idx >= rsides_.size())
    {
        throw std::out_of_range("nterm_idx out of range");
    }
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
    if (nterm_idx >= rsides_.size())
    {
        throw std::out_of_range("nterm_idx out of range");
    }
    if (rside_idx >= rsides_[nterm_idx].size())
    {
        throw std::out_of_range("rside_idx out of range");
    }
    return rsides_[nterm_idx][rside_idx].symbols_.size();
}

symbol_ref ruleset::get_symbol(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const
{
    if (nterm_idx >= rsides_.size())
    {
        throw std::out_of_range("nterm_idx out of range");
    }
    if (rside_idx >= rsides_[nterm_idx].size())
    {
        throw std::out_of_range("rside_idx out of range");
    }
    if (symbol_idx >= rsides_[nterm_idx][rside_idx].symbols_.size())
    {
        throw std::out_of_range("symbol_idx out of range");
    }
    return rsides_[nterm_idx][rside_idx].symbols_[symbol_idx];
}

symbol_type ruleset::get_symbol_type(size_t nterm_idx, size_t rside_idx, size_t symbol_idx) const
{
    return get_symbol(nterm_idx, rside_idx, symbol_idx).type_;
}

size_t ruleset::get_rside_precedence(size_t nterm_idx, size_t rside_idx) const
{
    if (nterm_idx >= rsides_.size())
    {
        throw std::out_of_range("nterm_idx out of range");
    }
    if (rside_idx >= rsides_[nterm_idx].size())
    {
        throw std::out_of_range("rside_idx out of range");
    }
    return rsides_[nterm_idx][rside_idx].precedence_;
}

std::string_view ruleset::get_nterm_name(size_t nterm_idx) const
{
    return symbols_.get_nterm_name(nterm_idx);
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

    add_rule_impl("$root", {name});

    return ref;
}

} // namespace ptg
