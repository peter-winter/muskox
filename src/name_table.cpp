/**
 * @file name_table.cpp
 * @brief Implementation of the name_table class.
 *
 * Part of the larger MuskOx project.
 */

#include "name_table.h"

namespace muskox
{

name_table::name_table(const symbol_collection& sc)
    : term_positions_(sc.get_term_count()),
      nterm_positions_(sc.get_nterm_count())
{
    // Calculate total size to reserve memory
    size_t total_size = 0;

    for (size_t i = 0; i < sc.get_term_count(); ++i)
    {
        total_size += sc.get_term_name(i).size();
    }

    for (size_t i = 0; i < sc.get_nterm_count(); ++i)
    {
        total_size += sc.get_nterm_name(i).size();
    }

    memory_.reserve(total_size);

    // Append terminals
    for (size_t i = 0; i < sc.get_term_count(); ++i)
    {
        size_t offset = memory_.size();
        std::string_view name = sc.get_term_name(i);
        memory_.append(name);
        term_positions_[i] = {offset, name.size()};
    }

    // Append non-terminals
    for (size_t i = 0; i < sc.get_nterm_count(); ++i)
    {
        size_t offset = memory_.size();
        std::string_view name = sc.get_nterm_name(i);
        memory_.append(name);
        nterm_positions_[i] = {offset, name.size()};
    }
}

std::string_view name_table::get_term_name(size_t idx) const
{
    if (idx >= term_positions_.size())
    {
        throw std::out_of_range("Invalid terminal index");
    }

    auto [offset, length] = term_positions_[idx];

    return std::string_view(memory_.data() + offset, length);
}

std::string_view name_table::get_nterm_name(size_t idx) const
{
    if (idx >= nterm_positions_.size())
    {
        throw std::out_of_range("Invalid non-terminal index");
    }

    auto [offset, length] = nterm_positions_[idx];

    return std::string_view(memory_.data() + offset, length);
}

size_t name_table::get_term_count() const
{
    return term_positions_.size();
}

size_t name_table::get_nterm_count() const
{
    return nterm_positions_.size();
}

} // namespace muskox
