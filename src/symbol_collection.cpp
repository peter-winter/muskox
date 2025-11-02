/**
 * @file symbol_collection.cpp
 * @brief Implementation of the symbol_collection class.
 *
 * This file provides the implementation details for managing terminals and non-terminals
 * in the MuskOx grammar system, including addition, querying, and validation methods.
 *
 * Part of the larger MuskOx project.
 */

#include "symbol_collection.h"
#include "list_printer.h"
#include "grammar_error.h"

#include <stdexcept>

namespace muskox
{

symbol_collection::symbol_collection()
    : terms_(), nterms_(), name_to_ref_()
{
    name_to_ref_.reserve(512);
    name_to_ref_.max_load_factor(0.5f);

    add_nterm_impl("$root");
    add_term_impl("$eof");
}

size_t symbol_collection::validate()
{
    if (validated_)
    {
        throw std::runtime_error("Cannot validate twice");
    }
    validated_ = true;
    errors_.clear();
    warnings_.clear();
    check_no_nterms();
    check_no_terms();
    return errors_.size();
}

bool symbol_collection::is_validated() const
{
    return validated_;
}

size_t symbol_collection::add_term(std::string name, std::optional<size_t> prec, associativity assoc)
{
    if (validated_)
    {
        throw std::runtime_error("Cannot call add_term after validation");
    }
    
    if (name[0] == '$')
    {
        throw grammar_error(grammar_error::code::cannot_refer_special, name);
    }
    
    if (contains(name))
    {
        throw grammar_error(grammar_error::code::symbol_exists, name);
    }
    
    return add_term_impl(name, prec, assoc);
}

size_t symbol_collection::add_nterm(std::string name)
{
    if (validated_)
    {
        throw std::runtime_error("Cannot call add_nterm after validation");
    }
    
    if (name[0] == '$')
    {
        throw grammar_error(grammar_error::code::cannot_refer_special, name);
    }
    
    if (contains(name))
    {
        throw grammar_error(grammar_error::code::symbol_exists, name);
    }
    
    return add_nterm_impl(name);
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
    validate_term_idx(index);
    return terms_[index].name_;
}

std::string_view symbol_collection::get_nterm_name(size_t index) const
{
    validate_nterm_idx(index);
    return nterms_[index].name_;
}

associativity symbol_collection::get_term_assoc(size_t index) const
{
    validate_term_idx(index);
    return terms_[index].assoc_;
}

std::optional<size_t> symbol_collection::get_term_prec(size_t index) const
{
    validate_term_idx(index);
    return terms_[index].prec_;
}

size_t symbol_collection::get_term_count() const
{
    return terms_.size();
}

size_t symbol_collection::get_nterm_count() const
{
    return nterms_.size();
}

std::string symbol_collection::print_symbol_list(const symbol_list& sl) const
{
    return print_symbol_list_from_to(sl, 0, sl.size());
}

std::string symbol_collection::print_symbol_list_from_to(const symbol_list& sl, size_t start, size_t end) const
{
    list_printer lp;
    return lp.print_container_from_to(sl, [&](auto ref){ return get_symbol_name(ref); }, start, end);
}

const std::vector<std::string>& symbol_collection::get_errors() const
{
    return errors_;
}

const std::vector<std::string>& symbol_collection::get_warnings() const
{
    return warnings_;
}

void symbol_collection::validate_nterm_idx(size_t index) const
{
    if (index >= nterms_.size())
    {
        throw std::out_of_range("Nterm index out of range");
    }
}

void symbol_collection::validate_term_idx(size_t index) const
{
    if (index >= terms_.size())
    {
        throw std::out_of_range("Term index out of range");
    }
}

size_t symbol_collection::add_term_impl(std::string name, std::optional<size_t> prec, associativity assoc)
{
    size_t index = terms_.size();
    auto [it, inserted] = name_to_ref_.emplace(std::move(name), symbol_ref{symbol_type::terminal, index});
    terms_.emplace_back(it->first, prec, assoc);
    return index;
}

size_t symbol_collection::add_nterm_impl(std::string name)
{
    size_t index = nterms_.size();
    auto [it, inserted] = name_to_ref_.emplace(std::move(name), symbol_ref{symbol_type::non_terminal, index});
    nterms_.emplace_back(it->first);
    return index;
}

void symbol_collection::check_no_nterms()
{
    if (get_nterm_count() <= 1)
    {
        errors_.push_back(grammar_message(grammar_error_templates::code::no_nterm).str());
    }
}

void symbol_collection::check_no_terms()
{
    if (get_term_count() <= 1)
    {
        warnings_.push_back(grammar_message(grammar_error_templates::code::no_term).str());
    }
}
    
} // namespace muskox
