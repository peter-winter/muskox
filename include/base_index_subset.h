/**
 * @file base_index_subset.h
 * @brief Base class for managing subsets of multi-dimensional indices using bitsets.
 *
 * This header provides a templated base class for tracking subsets of indices
 * in multi-dimensional spaces using a bit vector. It uses flat_indexer for
 * conversions and supports add, remove, contains, and union operations.
 *
 * Designed for efficient membership testing in the MuskOx project.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include "flat_indexer.h"

#include <vector>

namespace muskox
{

/**
 * @class base_index_subset
 * @brief Bit-based subset for multi-dimensional indices.
 *
 * @tparam Dim The number of dimensions.
 */
template <size_t Dim>
class base_index_subset
{
public:
    /**
     * @brief Type alias for element (array of indices).
     */
    using element_type = std::array<size_t, Dim>;
    
private:
    flat_indexer<Dim> indexer_; //!< The indexer for flat conversions.
    std::vector<char> bits_; //!< Bit vector for membership.

public:
    /**
     * @brief Constructs the subset with given sizes.
     *
     * @param sizes The sizes of dimensions.
     * @param value Initial bit value (default false).
     */
    base_index_subset(const element_type& sizes, bool value = false)
        : indexer_(sizes), bits_()
    {
        bits_.reserve(indexer_.get_total_size());
        bits_.assign(indexer_.get_total_size(), value ? 1 : 0);
    }

    /**
     * @brief Destructor.
     */
    ~base_index_subset() = default;

    /**
     * @brief Copy constructor.
     */
    base_index_subset(const base_index_subset&) = default;

    /**
     * @brief Copy assignment.
     */
    base_index_subset& operator = (const base_index_subset&) = default;

    /**
     * @brief Move constructor.
     */
    base_index_subset(base_index_subset&&) = default;

    /**
     * @brief Adds element with indices to the subset.
     *
     * @tparam Idx Index types.
     * @param indices The element indices.
     * @return True if added (was not present), false otherwise.
     */
    template <typename... Idx>
    bool add(Idx... indices)
    {
        size_t flat = indexer_.to_flat(indices...);
        if (bits_[flat] != 0)
        {
            return false;
        }
        bits_[flat] = 1;
        return true;
    }

    /**
     * @brief Unions with another subset.
     *
     * @param other The other subset.
     * @throw std::invalid_argument If sizes don't match.
     */
    void add(const base_index_subset<Dim>& other)
    {
        validate_sizes(other);
        
        for (size_t i = 0; i < bits_.size(); ++i)
        {
            bits_[i] = (bits_[i] != 0 || other.bits_[i] != 0) ? 1 : 0;
        }
    }
    
    /**
     * @brief Adds an element with indices in array.
     *
     * @param indices The element.
     * @return True if added (was not present), false otherwise.
     */
    bool add(const element_type& indices)
    {
        return add_impl(indices, std::make_index_sequence<Dim>{});
    }
        
    /**
     * @brief Removes element with indices from the subset.
     *
     * @tparam Idx Index types.
     * @param indices The element indices.
     * @return True if removed (was present), false otherwise.
     */
    template <typename... Idx>
    bool remove(Idx... indices)
    {
        size_t flat = indexer_.to_flat(indices...);
        if (bits_[flat] == 0)
        {
            return false;
        }
        bits_[flat] = 0;
        return true;
    }

    /**
     * @brief Checks if element with indices is in the subset.
     *
     * @tparam Idx Index types.
     * @param indices The element indices.
     * @return True if present.
     */
    template <typename... Idx>
    bool contains(Idx... indices) const
    {
        size_t flat = indexer_.to_flat(indices...);
        return bits_[flat] != 0;
    }
    
    /**
     * @brief Checks if element with indices array is in the subset.
     *
     * @tparam Idx Index types.
     * @param indices The element indices.
     * @return True if present.
     */
    bool contains(const element_type& indices) const
    {
        return contains_impl(indices, std::make_index_sequence<Dim>{});
    }
        
    /**
     * @brief Gets the total possible size.
     *
     * @return The total size from indexer.
     */
    size_t get_size() const
    {
        return indexer_.get_total_size();
    }

    /**
     * @brief Validates sizes against another subset.
     *
     * @param other The other subset.
     * @throw std::invalid_argument If sizes don't match.
     */
    void validate_sizes(const base_index_subset<Dim>& other) const
    {
        indexer_.validate_sizes(other.indexer_);
    }
    
private:
    /**
     * @brief Implementation for contains with index sequence.
     *
     * @param indices The element.
     * @param seq Index sequence.
     * @return True if present.
     */
    template <std::size_t... I>
    bool contains_impl(const element_type& indices, std::index_sequence<I...>) const
    {
        return contains(indices[I]...);
    }
    
    /**
     * @brief Implementation for add with index sequence.
     *
     * @param indices The element.
     * @param seq Index sequence.
     * @return True if added (was not present), false otherwise.
     */
    template <std::size_t... I>
    bool add_impl(const element_type& indices, std::index_sequence<I...>)
    {
        return add(indices[I]...);
    }
};

} // namespace muskox
