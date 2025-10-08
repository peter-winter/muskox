#include <symbol_collection.h>

#include <grammar_error.h>

#include <stdexcept>

namespace ptg
{

symbol_collection::symbol_collection()
    : terms_(), nterms_(), name_to_ref_()
{
    name_to_ref_.reserve(512);
    name_to_ref_.max_load_factor(0.5f);

    add_nterm("$root");
    add_term("$eof", associativity::left(), 0);
}

void symbol_collection::add_term(std::string name, associativity assoc, size_t prec)
{
    if (contains(name))
    {
        throw grammar_error(grammar_error::code::symbol_exists, name);
    }
    size_t index = terms_.size();
    auto [it, inserted] = name_to_ref_.emplace(std::move(name), symbol_ref{symbol_type::terminal, index});
    terms_.emplace_back(it->first, assoc, prec);
}

void symbol_collection::add_nterm(std::string name)
{
    if (contains(name))
    {
        throw grammar_error(grammar_error::code::symbol_exists, name);
    }
    size_t index = nterms_.size();
    auto [it, inserted] = name_to_ref_.emplace(std::move(name), symbol_ref{symbol_type::non_terminal, index});
    nterms_.emplace_back(it->first);
}

bool symbol_collection::contains(std::string_view name) const
{
    return name_to_ref_.contains(name);
}

symbol_ref symbol_collection::get_symbol_ref(std::string_view name) const
{
    auto it = name_to_ref_.find(name);
    if (it == name_to_ref_.end())
    {
        throw std::out_of_range("Symbol not found");
    }
    return it->second;
}

std::string_view symbol_collection::get_symbol_name(symbol_ref ref) const
{
    if (ref.type_ == symbol_type::terminal)
    {
        return get_term_name(ref.index_);
    }
    else if (ref.type_ == symbol_type::non_terminal)
    {
        return get_nterm_name(ref.index_);
    }
    throw std::invalid_argument("Unknown symbol type");
}

std::string_view symbol_collection::get_term_name(size_t index) const
{
    if (index >= terms_.size())
    {
        throw std::out_of_range("Term index out of range");
    }
    return terms_[index].name();
}

std::string_view symbol_collection::get_nterm_name(size_t index) const
{
    if (index >= nterms_.size())
    {
        throw std::out_of_range("Nterm index out of range");
    }
    return nterms_[index].name();
}

associativity symbol_collection::get_term_assoc(size_t index) const
{
    if (index >= terms_.size())
    {
        throw std::out_of_range("Term index out of range");
    }
    return terms_[index].assoc();
}

size_t symbol_collection::get_term_prec(size_t index) const
{
    if (index >= terms_.size())
    {
        throw std::out_of_range("Term index out of range");
    }
    return terms_[index].prec();
}

size_t symbol_collection::get_term_count() const
{
    return terms_.size();
}

size_t symbol_collection::get_nterm_count() const
{
    return nterms_.size();
}

void symbol_collection::print_symbol_list(std::ostream& os, const symbol_list& sl) const
{
    if (sl.empty())
    {
        return;
    }

    bool first = true;
    for (const auto& sym : sl)
    {
        if (!first)
        {
            os << " ";
        }
        first = false;
        os << get_symbol_name(sym);
    }
}

} // namespace ptg
