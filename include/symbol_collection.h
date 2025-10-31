/**
 * @file symbol_collection.h
 * @brief Manages collections of terminal and non-terminal symbols for grammar definitions.
 *
 * This header defines a class that handles the storage, addition, and querying of
 * symbols (both terminals and non-terminals) in a grammar. It uses a hash map for
 * fast lookups by name and vectors for indexed access. Methods are provided for
 * adding symbols with optional precedence and associativity, retrieving references,
 * and printing symbol lists for debugging.
 *
 * The class ensures uniqueness of symbol names and provides validation for indices.
 * A custom transparent hash is used for efficient string-based lookups.
 * Special symbols "$root" (non-terminal) and "$eof" (terminal) are added automatically.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include "associativity.h"
#include "nterm.h"
#include "symbol_ref.h"
#include "term.h"
#include "symbol_list.h"

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <optional>

namespace muskox
{

/**
 * @struct string_hash
 * @brief Transparent hash functor for strings and views.
 *
 * Allows hashing of std::string, std::string_view, and const char* interchangeably.
 */
struct string_hash
{
    using is_transparent = void; /// Transparent type alias.

    /**
     * @brief Hash operator for const char*.
     *
     * @param txt The text to hash.
     * @return Hash value.
     */
    size_t operator()(const char* txt) const
    {
        return std::hash<std::string_view>{}(txt);
    }

    /**
     * @brief Hash operator for std::string_view.
     *
     * @param txt The text to hash.
     * @return Hash value.
     */
    size_t operator()(std::string_view txt) const
    {
        return std::hash<std::string_view>{}(txt);
    }

    /**
     * @brief Hash operator for std::string.
     *
     * @param txt The text to hash.
     * @return Hash value.
     */
    size_t operator()(const std::string& txt) const
    {
        return std::hash<std::string>{}(txt);
    }
};

/**
 * @class symbol_collection
 * @brief Collection of terminals and non-terminals.
 *
 * Manages unique symbols by name, provides references, and utility methods.
 * Initializes with special symbols "$root" and "$eof".
 */
class symbol_collection
{
public:
    /**
     * @brief Constructor that initializes the collection with special symbols.
     */
    symbol_collection();

    /**
     * @brief Destructor.
     */
    ~symbol_collection() = default;

    /**
     * @brief Validates the symbol collection.
     *
     * Checks for at least one non-terminal besides $root.
     *
     * @throw grammar_error If no non-terminals.
     */
    void validate();

    /**
     * @brief Checks if the collection has been validated.
     *
     * @return True if validated.
     */
    bool is_validated() const { return validated_; }

    /**
     * @brief Adds a terminal symbol after checks.
     *
     * Checks if name starts with '$' or already exists, throws accordingly.
     *
     * @param name The name.
     * @param prec Optional precedence.
     * @param assoc Associativity.
     * @return The index.
     * @throw grammar_error If name starts with '$' or already exists.
     */
    size_t add_term(std::string name, std::optional<size_t> prec = std::nullopt, associativity assoc = associativity::left());

    /**
     * @brief Adds a non-terminal symbol after checks.
     *
     * Checks if name starts with '$' or already exists, throws accordingly.
     *
     * @param name The name.
     * @return The index.
     * @throw grammar_error If name starts with '$' or already exists.
     */
    size_t add_nterm(std::string name);

    /**
     * @brief Checks if a name exists in the collection.
     *
     * @param name The name to check.
     * @return True if the name exists, false otherwise.
     */
    bool contains(std::string_view name) const;

    /**
     * @brief Gets the symbol reference for a given name.
     *
     * @param name The name of the symbol.
     * @return The symbol_ref.
     * @throw std::out_of_range If the symbol is not found.
     */
    symbol_ref get_symbol_ref(std::string_view name) const;

    /**
     * @brief Gets the name for a given symbol reference.
     *
     * @param ref The symbol reference.
     * @return The name as string_view.
     * @throw std::invalid_argument If unknown symbol type.
     */
    std::string_view get_symbol_name(symbol_ref ref) const;

    /**
     * @brief Gets the name of a terminal by index.
     *
     * @param index The index of the terminal.
     * @return The name.
     * @throw std::out_of_range If index is invalid.
     */
    std::string_view get_term_name(size_t index) const;

    /**
     * @brief Gets the name of a non-terminal by index.
     *
     * @param index The index of the non-terminal.
     * @return The name.
     * @throw std::out_of_range If index is invalid.
     */
    std::string_view get_nterm_name(size_t index) const;

    /**
     * @brief Gets the associativity of a terminal by index.
     *
     * @param index The index of the terminal.
     * @return The associativity.
     * @throw std::out_of_range If index is invalid.
     */
    associativity get_term_assoc(size_t index) const;

    /**
     * @brief Gets the precedence of a terminal by index.
     *
     * @param index The index of the terminal.
     * @return Optional precedence.
     * @throw std::out_of_range If index is invalid.
     */
    std::optional<size_t> get_term_prec(size_t index) const;

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

    /**
     * @brief Prints a symbol list as a space-separated string.
     *
     * @param sl The symbol list.
     * @return The string representation.
     */
    std::string print_symbol_list(const symbol_list& sl) const;

    /**
     * @brief Prints a subset of a symbol list as space-separated.
     *
     * @param sl The symbol list.
     * @param start Starting index (inclusive).
     * @param end Ending index (exclusive).
     * @return The string representation.
     */
    std::string print_symbol_list_from_to(const symbol_list& sl, size_t start, size_t end) const;

private:
    std::vector<term> terms_; /// Vector of terminal symbols.
    std::vector<nterm> nterms_; /// Vector of non-terminal symbols.
    std::unordered_map<std::string, symbol_ref, string_hash, std::equal_to<>> name_to_ref_; /// Map from name to symbol reference.
    bool validated_ = false; /// Flag indicating if the collection has been validated.

    /**
     * @brief Validates a non-terminal index.
     *
     * @param index The index to check.
     * @throw std::out_of_range If invalid.
     */
    void validate_nterm_idx(size_t index) const;

    /**
     * @brief Validates a terminal index.
     *
     * @param index The index to check.
     * @throw std::out_of_range If invalid.
     */
    void validate_term_idx(size_t index) const;
    
    /**
     * @brief Internal method to add a terminal without checks.
     *
     * @param name The name.
     * @param prec Optional precedence.
     * @param assoc Associativity.
     * @return The index.
     */
    size_t add_term_impl(std::string name, std::optional<size_t> prec = std::nullopt, associativity assoc = associativity::left());

    /**
     * @brief Internal method to add a non-terminal without checks.
     *
     * @param name The name.
     * @return The index.
     */
    size_t add_nterm_impl(std::string name);
};

} // namespace muskox
