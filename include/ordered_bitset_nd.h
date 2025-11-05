/**
 * @file ordered_bitset_nd.h
 * @brief Multi-dimensional bitset with insertion order tracking.
 *
 * This header provides a templated class for managing multi-dimensional bitsets for fast lookups.
 * Supports add, contains, union, and checks like contains_all. Includes a 1D
 * specialization and a builder class for fluent addition.
 *
 * Extends bitset_nd for insertion order (or custom order using a Comp parameter) tracking and enumeration in the MuskOx project.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include "bitset_nd.h"

#include <array>
#include <vector>
#include <utility>
#include <algorithm>

namespace muskox
{

/**
 * @brief Type representing no comparator
 */
struct no_comp
{};

/**
 * @class ordered_bitset_nd
 * @brief Multi-dimensional bitset with order and membership check.
 *
 * @tparam Dim The number of dimensions.
 * @tparam Comp The comparator.
 */
template <size_t Dim, typename Comp = no_comp>
class ordered_bitset_nd
{
public:
    /**
     * @brief Type alias for element (array of indices).
     */
    using element_type = std::array<size_t, Dim>;

private:
    bitset_nd<Dim> base_; /// Base bitset for membership.
    std::vector<element_type> indices_; /// List of added indices.
    Comp comp_;
    
public:
    /**
     * @brief Constructs the bitset with sizes.
     *
     * @param sizes The sizes of dimensions.
     * @param comp The comparator.
     */
    ordered_bitset_nd(const element_type& sizes, Comp comp)
        : base_(sizes), indices_(), comp_(comp)
    {}
    
    /**
     * @brief Constructs the bitset with sizes.
     *
     * @param sizes The sizes of dimensions.
     */
    ordered_bitset_nd(const element_type& sizes)
        : base_(sizes), indices_()
    {}

    /**
     * @brief Destructor.
     */
    ~ordered_bitset_nd() = default;

    /**
     * @brief Copy constructor.
     */
    ordered_bitset_nd(const ordered_bitset_nd&) = default;

    /**
     * @brief Copy assignment.
     */
    ordered_bitset_nd& operator = (const ordered_bitset_nd&) = default;

    /**
     * @brief Move constructor.
     */
    ordered_bitset_nd(ordered_bitset_nd&&) = default;

    /**
     * @brief Base bitset accessor
     * @return The base object
     */
    const bitset_nd<Dim>& get_base() const
    {
        return base_;
    }
    
    /**
     * @brief Adds element with indices to the bitset.
     *
     * @tparam Idx Index types.
     * @param indices The element indices.
     * @return True if added (new), false if already present.
     */
    template <typename... Idx>
    bool add(Idx... indices)
    {
        bool inserted = base_.add(indices...);
        if (inserted)
        {
            element_type val{static_cast<size_t>(indices)...};
            if constexpr (std::is_same_v<Comp, no_comp>)
            {
                indices_.push_back(val);
            }
            else
            {
                auto it = std::upper_bound(indices_.begin(), indices_.end(), val, comp_);
                indices_.insert(it, val);
            }
        }
        return inserted;
    }

    /**
     * @brief Unions with another bitset.
     *
     * @param other The other bitset.
     * @tparam OtherComp Other bitset comparator
     * @throw std::invalid_argument If sizes don't match.
     */
    template<typename OtherComp>
    void add(const ordered_bitset_nd<Dim, OtherComp>& other)
    {
        base_.validate_sizes(other.get_base());
        
        for (const auto& arr : other.get_indices())
        {
            add(arr);
        }
    }

    /**
     * @brief Checks if element is in the bitset.
     *
     * @tparam Idx Index types.
     * @param indices The element indices.
     * @return True if present.
     */
    template <typename... Idx>
    bool contains(Idx... indices) const
    {
        return base_.contains(indices...);
    }
    
    /**
     * @brief Checks if element is in the bitset.
     *
     * @param indices The element indices array.
     * @return True if present.
     */
    bool contains(const element_type& indices) const
    {
        return contains_impl(indices, std::make_index_sequence<Dim>{});
    }
    
    /**
     * @brief Checks if all of another's elements are contained.
     *
     * @param other The other bitset.
     * @return True if all are contained.
     * @throw std::invalid_argument If sizes don't match.
     */
    bool contains_all(const ordered_bitset_nd<Dim, Comp>& other) const
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
     * @brief Checks if exactly matches another's elements.
     *
     * @param other The other bitset.
     * @return True if same count and contains all. Order is not matched.
     * @throw std::invalid_argument If sizes don't match.
     */
    bool contains_only_items(const ordered_bitset_nd<Dim, Comp>& other) const
    {
        base_.validate_sizes(other.base_);
        return get_count() == other.get_count() && contains_all(other);
    }

    /**
     * @brief Gets the number of added elements.
     *
     * @return The count.
     */
    size_t get_count() const
    {
        return indices_.size();
    }

    /**
     * @brief Gets the list of added elements.
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
     * @brief Adds an element with indices in array.
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
 * @class ordered_bitset_nd<1>
 * @brief Specialization for 1D index bitset.
 * 
 * @tparam Comp The comparator.
 */
template <typename Comp>
class ordered_bitset_nd<1, Comp>
{
public:
    /**
     * @brief Type alias for element (size_t).
     */
    using element_type = size_t;

private:
    bitset_nd<1> base_; /// Base bitset.
    std::vector<element_type> indices_; /// List of indices.
    Comp comp_;
    
public:
    /**
     * @brief Constructs with size.
     *
     * @param size The size.
     * @param comp The comparator
     */
    ordered_bitset_nd(size_t size, Comp comp)
        : base_({size}), indices_(), comp_(comp)
    {}
    
    /**
     * @brief Constructs with size.
     *
     * @param size The size.
     */
    ordered_bitset_nd(size_t size)
        : base_({size}), indices_()
    {}

    /**
     * @brief Destructor.
     */
    ~ordered_bitset_nd() = default;

    /**
     * @brief Copy constructor.
     */
    ordered_bitset_nd(const ordered_bitset_nd&) = default;

    /**
     * @brief Copy assignment.
     */
    ordered_bitset_nd& operator = (const ordered_bitset_nd&) = default;

    /**
     * @brief Move constructor.
     */
    ordered_bitset_nd(ordered_bitset_nd&&) = default;

    /**
     * @brief Base bitset accessor
     * @return The base object
     */
    const bitset_nd<1>& get_base() const
    {
        return base_;
    }
    
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
            if constexpr (std::is_same_v<Comp, no_comp>)
            {
                indices_.push_back(idx);
            }
            else
            {
                auto it = std::upper_bound(indices_.begin(), indices_.end(), idx, comp_);
                indices_.insert(it, idx);
            }
        }
        return inserted;
    }

    /**
     * @brief Unions with another 1D bitset.
     *
     * @param other The other.
     * @tparam OtherComp Other bitset comparator
     * @throw std::invalid_argument If sizes don't match.
     */
    template<typename OtherComp>
    void add(const ordered_bitset_nd<1, OtherComp>& other)
    {
        base_.validate_sizes(other.get_base());
        
        for (const auto& idx : other.get_indices())
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
     * @brief Checks if all of another's elements are contained.
     *
     * @param other The other.
     * @return True if all contained.
     * @throw std::invalid_argument If sizes don't match.
     */
    bool contains_all(const ordered_bitset_nd<1, Comp>& other) const
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
     * @brief Checks if exactly matches another's elements. Order is not matched
     *
     * @param other The other.
     * @return True if same.
     * @throw std::invalid_argument If sizes don't match.
     */
    bool contains_only_items(const ordered_bitset_nd<1, Comp>& other) const
    {
        base_.validate_sizes(other.base_);
        return get_count() == other.get_count() && contains_all(other);
    }
    
    /**
     * @brief Gets the count of added elements.
     *
     * @return The count.
     */
    size_t get_count() const
    {
        return indices_.size();
    }

    /**
     * @brief Gets the list of added elements.
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
 * @class ordered_bitset_nd_builder
 * @brief Fluent builder for ordered_bitset_nd.
 *
 * @tparam Dim Dimensions.
 * @tparam Comp The comparator.
 */
template <size_t Dim, typename Comp = no_comp>
class ordered_bitset_nd_builder
{
public:
    /**
     * @brief Type alias for bitset.
     */
    using bitset_type = ordered_bitset_nd<Dim, Comp>;

    /**
     * @brief Type alias for sizes.
     */
    using sizes_type = typename bitset_type::element_type;

    /**
     * @brief Constructs builder with sizes.
     *
     * @param sizes The sizes.
     */
    ordered_bitset_nd_builder(const sizes_type& sizes)
        : sizes_(sizes), subset_(sizes)
    {
        reset();
    }

    /**
     * @brief Adds element fluently.
     *
     * @tparam Idx Indices.
     * @param indices The element indices.
     * @return Reference to builder.
     */
    template <typename... Idx>
    ordered_bitset_nd_builder& operator()(Idx... indices)
    {
        subset_.add(indices...);
        return *this;
    }

    /**
     * @brief Builds the bitset.
     *
     * @return The built bitset.
     */
    bitset_type build()
    {
        return subset_;
    }

    /**
     * @brief Resets the builder.
     *
     * @return Reference to builder.
     */
    ordered_bitset_nd_builder& reset()
    {
        subset_ = bitset_type(sizes_);
        return *this;
    }
    
private:
    sizes_type sizes_; /// The sizes.
    bitset_type subset_; /// The building subset.
};

} // namespace muskox
