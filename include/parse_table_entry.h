/**
 * @file parse_table_entry.h
 * @brief Defines the parse table entry class for LR(1) parsing actions.
 *
 * This header provides the parse_table_entry class, which represents an entry
 * in the LR(1) parse table. Entries can indicate shifts, reductions, reduce-reduce
 * conflicts, or errors. It includes an enum for entry types and a union for
 * storing type-specific data, with methods for creation, setting, querying, and
 * comparison.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <stdexcept>
#include <limits>

namespace muskox
{

/**
 * @brief Enumeration for parse table entry types.
 *
 * Distinguishes between error, shift, reduce, and reduce-reduce conflict entries.
 */
enum class entry_type : uint8_t { error, shift, reduce, rr_conflict };

/**
 * @class parse_table_entry
 * @brief Represents an entry in the LR(1) parse table.
 *
 * Stores the type of action (shift, reduce, rr_conflict, or error) and
 * associated data (e.g., state index for shifts, reduction indices for reduces).
 * Provides static factory methods, setters, type checks, and getters.
 */
class parse_table_entry
{
public:
    /**
     * @brief Struct for reduction data.
     *
     * Holds the non-terminal and reduction length for a reduction.
     */
    struct reduction
    {
        uint16_t nterm_idx_; /// Non-terminal index.
        uint16_t length_; /// Reduction length (number of symbols to pop).
    };
    
    /**
     * @brief Struct for reduce-reduce conflict.
     *
     * Holds the start index for rr_conflict and conflicts count.
     */
    struct rr_conflict_data
    {
        uint16_t rr_conflict_start_idx_; /// Start index for rr_conflict.
        uint16_t rr_conflict_count_; /// Count for rr_conflict.
    };
    
    /**
     * @brief Default constructor initializes to error type.
     */
    parse_table_entry() = default;
    
    /**
     * @brief Static factory for a shift entry.
     *
     * @param state_idx The target state index for the shift.
     * @return A shift entry.
     */
    static parse_table_entry shift(size_t state_idx);

    /**
     * @brief Static factory for a reduce entry.
     *
     * @param nterm_idx The non-terminal index.
     * @param length The reduction length.
     * @return A reduce entry.
     */
    static parse_table_entry reduce(size_t nterm_idx, size_t length);

    /**
     * @brief Static factory for a reduce-reduce conflict entry.
     *
     * @param rr_conflict_start_idx The starting index in the conflict table.
     * @param rr_conflict_count The number of conflicting reductions.
     * @return A rr_conflict entry.
     */
    static parse_table_entry rr_conflict(size_t rr_conflict_start_idx, size_t rr_conflict_count);
    
    /**
     * @brief Sets the entry to a shift action.
     *
     * @param state_idx The target state index.
     * @throw std::overflow_error If state_idx exceeds 32-bit limit.
     */
    void set_shift(size_t state_idx);

    /**
     * @brief Sets the entry to a reduce action.
     *
     * @param nterm_idx The non-terminal index.
     * @param length The reduction length.
     * @throw std::overflow_error If indices exceed 16-bit limits.
     */
    void set_reduce(size_t nterm_idx, size_t length);

    /**
     * @brief Sets the entry to a reduce-reduce conflict.
     *
     * @param rr_conflict_start_idx The starting index in the conflict table.
     * @param rr_conflict_count The number of conflicting reductions.
     * @throw std::overflow_error If values exceed 16-bit limits.
     */
    void set_rr_conflict(size_t rr_conflict_start_idx, size_t rr_conflict_count);
    
    /**
     * @brief Checks if the entry is an error.
     *
     * @return True if error type.
     */
    bool is_error() const;

    /**
     * @brief Checks if the entry is a shift.
     *
     * @return True if shift type.
     */
    bool is_shift() const;

    /**
     * @brief Checks if the entry is a reduce.
     *
     * @return True if reduce type.
     */
    bool is_reduce() const;

    /**
     * @brief Checks if the entry is a reduce-reduce conflict.
     *
     * @return True if rr_conflict type.
     */
    bool is_rr_conflict() const;
    
    /**
     * @brief Gets the shift state index.
     *
     * @return The state index.
     * @throw Undefined behavior if not a shift entry.
     */
    size_t get_shift_state_idx() const;

    /**
     * @brief Gets the reduce non-terminal index.
     *
     * @return The non-terminal index.
     * @throw Undefined behavior if not a reduce entry.
     */
    size_t get_reduce_nterm_idx() const;

    /**
     * @brief Gets the reduce length.
     *
     * @return The length.
     * @throw Undefined behavior if not a reduce entry.
     */
    size_t get_reduce_length() const;

    /**
     * @brief Gets the reduce-reduce conflict start index.
     *
     * @return The start index.
     * @throw Undefined behavior if not a rr_conflict entry.
     */
    size_t get_rr_conflict_start_idx() const;

    /**
     * @brief Gets the reduce-reduce conflict count.
     *
     * @return The count.
     * @throw Undefined behavior if not a rr_conflict entry.
     */
    size_t get_rr_conflict_count() const;
    
    /**
     * @brief Equality operator for comparing entries.
     *
     * @param other The other entry.
     * @return True if types and data match.
     */
    bool operator==(const parse_table_entry& other) const;

private:
    entry_type type_ = entry_type::error; /// The entry type.
    union
    {
        reduction reduce_; /// Data for reduce.
        uint32_t shift_state_idx_; /// Data for shift.        
        rr_conflict_data rr_conflict_; /// Data for rr_conflict.
    } data_ = {}; /// Union for type-specific data.
};

}
