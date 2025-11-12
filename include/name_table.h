/**
 * @file name_table.h
 * @brief Defines the name_table class for efficient storage and access of symbol names.
 *
 * This header provides the name_table class, which concatenates all terminal and non-terminal
 * names from a symbol_collection into a single contiguous string for memory efficiency.
 * It provides string_view accessors for names by index and count accessors for terminals
 * and non-terminals.
 *
 * The class is designed for read-only access after construction.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include "symbol_collection.h"

#include <string>
#include <string_view>
#include <vector>

namespace muskox
{

/**
 * @class name_table
 * @brief Manages concatenated storage of symbol names with efficient access.
 *
 * Concatenates all terminal and non-terminal names into a single string and
 * maintains position data for quick string_view retrieval by index.
 */
class name_table
{
public:
    /**
     * @brief Constructs the name_table from a symbol_collection.
     *
     * Iterates over terminals and non-terminals, concatenating their names
     * into a single string and recording offsets and lengths.
     *
     * @param sc The symbol_collection to extract names from.
     */
    explicit name_table(const symbol_collection& sc);

    /**
     * @brief Destructor.
     */
    ~name_table() = default;

    /**
     * @brief Gets the name of a terminal as a string_view.
     *
     * @param idx The terminal index.
     * @return The name as string_view.
     * @throw std::out_of_range If idx is invalid.
     */
    std::string_view get_term_name(size_t idx) const;

    /**
     * @brief Gets the name of a non-terminal as a string_view.
     *
     * @param idx The non-terminal index.
     * @return The name as string_view.
     * @throw std::out_of_range If idx is invalid.
     */
    std::string_view get_nterm_name(size_t idx) const;

    /**
     * @brief Gets the number of terminals.
     *
     * @return The count of terminals.
     */
    size_t get_term_count() const;

    /**
     * @brief Gets the number of non-terminals.
     *
     * @return The count of non-terminals.
     */
    size_t get_nterm_count() const;

private:
    std::string memory_; ///< The concatenated string of all names.
    std::vector<std::pair<size_t, size_t>> term_positions_; ///< Offsets and lengths for terminals.
    std::vector<std::pair<size_t, size_t>> nterm_positions_; ///< Offsets and lengths for non-terminals.
};

} // namespace muskox
