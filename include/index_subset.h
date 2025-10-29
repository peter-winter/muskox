/**
 * @file index_subset.h
 * @brief Subset management for multi-dimensional indices with list and bitset.
 *
 * This header provides a templated class for managing subsets of multi-dimensional
 * indices, combining a list of added indices with a bitset for fast lookups.
 * Supports add, contains, union, and checks like contains_all. Includes a 1D
 * specialization and a builder class for fluent addition.
 *
 * Extends base_index_subset for efficiency in the MuskOx project.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include <base_index_subset.h>

#include <array>
#include <vector>
#include <utility>

namespace muskox
{

/**
 * @class index_subset
 * @brief Subset of multi-dimensional indices with list and membership check.
 *
 * @tparam Dim The number of dimensions.
 */
template <size_t Dim>
class index_subset
{
public:
    /**
     * @brief Type alias for element (array of indices).
     */
    using element_type = std::array<size_t, Dim>;

private:
    base_index_subset<Dim> base_; //!< Base bitset for membership.
    std::vector<element_type> indices_; //!< List of added indices.

public:
    /**
     * @brief Constructs the subset with sizes.
     *
     * @param sizes The sizes of dimensions.
     */
    index_subset(const element_type& sizes)
        : base_(sizes), indices_()
    {}

    /**
     * @brief Destructor.
     */
    ~index_subset() = default;

    /**
     * @brief Copy constructor.
     */
    index_subset(const index_subset&) = default;

    /**
     * @brief Copy assignment.
     */
    index_subset& operator = (const index_subset&) = default;

    /**
     * @brief Move constructor.
     */
    index_subset(index_subset&&) = default;

    /**
     * @brief Adds indices to the subset.
     *
     * @tparam Idx Index types.
     * @param indices The indices.
     * @return True if added (new), false if already present.
     */
    template <typename... Idx>
    bool add(Idx... indices)
    {
        bool inserted = base_.add(indices...);
        if (inserted)
        {
            indices_.push_back({static_cast<size_t>(indices)...});
        }
        return inserted;
    }

    /**
     * @brief Unions with another subset.
     *
     * @param other The other subset.
     * @throw std::invalid_argument If sizes don't match.
     */
    void add(const index_subset<Dim>& other)
    {
        base_.validate_sizes(other.base_);
        
        for (const auto& arr : other.indices_)
        {
            add(arr);
        }
    }

    /**
     * @brief Checks if indices are in the subset.
     *
     * @tparam Idx Index types.
     * @param indices The indices.
     * @return True if present.
     */
    template <typename... Idx>
    bool contains(Idx... indices) const
    {
        return base_.contains(indices...);
    }
    
    /**
     * @brief Checks if element is in the subset.
     *
     * @param indices The element array.
     * @return True if present.
     */
    bool contains(const element_type& indices) const
    {
        return contains_impl(indices, std::make_index_sequence<Dim>{});
    }
    
    /**
     * @brief Checks if all of another's indices are contained.
     *
     * @param other The other subset.
     * @return True if all are contained.
     * @throw std::invalid_argument If sizes don't match.
     */
    bool contains_all(const index_subset<Dim>& other) const
    {
        base_.validate_sizes(other.base_);
        
        for (const auto& indices : other.indices_)
        {
            if (!contains(indices))
            {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Checks if exactly matches another's items.
     *
     * @param other The other subset.
     * @return True if same count and contains all.
     * @throw std::invalid_argument If sizes don't match.
     */
    bool contains_only_items(const index_subset<Dim>& other) const
    {
        base_.validate_sizes(other.base_);
        return get_count() == other.get_count() && contains_all(other);
    }

    /**
     * @brief Gets the number of added items.
     *
     * @return The count.
     */
    size_t get_count() const
    {
        return indices_.size();
    }

    /**
     * @brief Gets the list of indices.
     *
     * @return Const reference to the vector.
     */
    const std::vector<element_type>& get_indices() const
    {
        return indices_;
    }

    /**
     * @brief Gets the total possible size.
     *
     * @return The size from base.
     */
    size_t get_size() const
    {
        return base_.get_size();
    }

    /**
     * @brief Adds an element array.
     *
     * @param indices The element.
     */
    void add(const element_type& indices)
    {
        add_impl(indices, std::make_index_sequence<Dim>{});
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
        return base_.contains(indices[I]...);
    }
    
    /**
     * @brief Implementation for add with index sequence.
     *
     * @param indices The element.
     * @param seq Index sequence.
     */
    template <std::size_t... I>
    void add_impl(const element_type& indices, std::index_sequence<I...>)
    {
        add(indices[I]...);
    }
};

/**
 * @class index_subset<1>
 * @brief Specialization for 1D index subset.
 */
template <>
class index_subset<1>
{
public:
    /**
     * @brief Type alias for element (size_t).
     */
    using element_type = size_t;

private:
    base_index_subset<1> base_; //!< Base bitset.
    std::vector<element_type> indices_; //!< List of indices.

public:
    /**
     * @brief Constructs with size.
     *
     * @param size The size.
     */
    index_subset(size_t size)
        : base_({size}), indices_()
    {}

    /**
     * @brief Destructor.
     */
    ~index_subset() = default;

    /**
     * @brief Copy constructor.
     */
    index_subset(const index_subset&) = default;

    /**
     * @brief Copy assignment.
     */
    index_subset& operator = (const index_subset&) = default;

    /**
     * @brief Move constructor.
     */
    index_subset(index_subset&&) = default;

    /**
     * @brief Adds an index.
     *
     * @param idx The index.
     * @return True if added.
     */
    bool add(size_t idx)
    {
        bool inserted = base_.add(idx);
        if (inserted)
        {
            indices_.push_back(idx);
        }
        return inserted;
    }

    /**
     * @brief Unions with another 1D subset.
     *
     * @param other The other.
     * @throw std::invalid_argument If sizes don't match.
     */
    void add(const index_subset<1>& other)
    {
        base_.validate_sizes(other.base_);
        
        for (const auto& idx : other.indices_)
        {
            add(idx);
        }
    }

    /**
     * @brief Checks if index is present.
     *
     * @param idx The index.
     * @return True if present.
     */
    bool contains(size_t idx) const
    {
        return base_.contains(idx);
    }

    /**
     * @brief Checks if all of another's indices are contained.
     *
     * @param other The other.
     * @return True if all contained.
     * @throw std::invalid_argument If sizes don't match.
     */
    bool contains_all(const index_subset<1>& other) const
    {
        base_.validate_sizes(other.base_);
        
        for (const auto& arr : other.indices_)
        {
            if (!base_.contains(arr))
            {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Checks if exactly matches another's items.
     *
     * @param other The other.
     * @return True if same.
     * @throw std::invalid_argument If sizes don't match.
     */
    bool contains_only_items(const index_subset<1>& other) const
    {
        base_.validate_sizes(other.base_);
        return get_count() == other.get_count() && contains_all(other);
    }
    
    /**
     * @brief Gets the count of items.
     *
     * @return The count.
     */
    size_t get_count() const
    {
        return indices_.size();
    }

    /**
     * @brief Gets the list of indices.
     *
     * @return Const reference.
     */
    const std::vector<element_type>& get_indices() const
    {
        return indices_;
    }

    /**
     * @brief Gets total size.
     *
     * @return The size.
     */
    size_t get_size() const
    {
        return base_.get_size();
    }
};

/**
 * @class index_subset_builder
 * @brief Fluent builder for index_subset.
 *
 * @tparam Dim Dimensions.
 */
template <size_t Dim>
class index_subset_builder
{
public:
    /**
     * @brief Type alias for subset.
     */
    using subset_type = index_subset<Dim>;

    /**
     * @brief Type alias for sizes.
     */
    using sizes_type = typename subset_type::element_type;

    /**
     * @brief Constructs builder with sizes.
     *
     * @param sizes The sizes.
     */
    index_subset_builder(const sizes_type& sizes)
        : sizes_(sizes), subset_(sizes)
    {
        reset();
    }

    /**
     * @brief Adds indices fluently.
     *
     * @tparam Idx Indices.
     * @param indices The indices.
     * @return Reference to builder.
     */
    template <typename... Idx>
    index_subset_builder& operator()(Idx... indices)
    {
        subset_.add(indices...);
        return *this;
    }

    /**
     * @brief Builds the subset.
     *
     * @return The built subset.
     */
    subset_type build()
    {
        return subset_;
    }

    /**
     * @brief Resets the builder.
     *
     * @return Reference to builder.
     */
    index_subset_builder& reset()
    {
        subset_ = subset_type(sizes_);
        return *this;
    }

private:
    sizes_type sizes_; //!< The sizes.
    subset_type subset_; //!< The building subset.
};

} // namespace muskox
