/**
 * @file flat_indexer.h
 * @brief Multi-dimensional flat indexer for converting indices to flat offsets.
 *
 * This header provides a templated class for handling multi-dimensional arrays
 * by computing flat indices from multi-dimensional coordinates. It supports
 * arbitrary dimensions and includes a specialization for 1D. Strides are precomputed
 * for efficient access, and validation ensures positive sizes and valid indices.
 *
 * Useful for flattening multi-dimensional data structures in the MuskOx project.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include <array>
#include <stdexcept>

namespace muskox
{

/**
 * @class flat_indexer
 * @brief Computes flat indices from multi-dimensional coordinates.
 *
 * @tparam Dim The number of dimensions (must be > 0).
 */
template <size_t Dim>
class flat_indexer
{
private:
    std::array<size_t, Dim> sizes_; //!< The sizes of each dimension.
    std::array<size_t, Dim> strides_; //!< Precomputed strides for each dimension.
    size_t total_size_; //!< Total number of elements (product of sizes).

    /**
     * @brief Computes the strides based on sizes.
     */
    void compute_strides()
    {
        if (Dim == 0)
        {
            return;
        }

        strides_[Dim - 1] = 1;
        for (size_t d = Dim - 1; d-- > 0;)
        {
            strides_[d] = strides_[d + 1] * sizes_[d + 1];
        }
    }

public:
    /**
     * @brief Constructs the indexer with given sizes.
     *
     * @param sizes Array of sizes for each dimension.
     * @throw std::invalid_argument If Dim == 0 or any size == 0.
     */
    flat_indexer(const std::array<size_t, Dim>& sizes)
        : sizes_(sizes), strides_(), total_size_(1)
    {
        if (Dim == 0)
        {
            throw std::invalid_argument("At least one dimension required");
        }

        for (auto s : sizes_)
        {
            if (s == 0)
            {
                throw std::invalid_argument("Size must be greater than 0");
            }
            total_size_ *= s;
        }

        compute_strides();
    }

    /**
     * @brief Destructor.
     */
    ~flat_indexer() = default;

    /**
     * @brief Copy constructor.
     */
    flat_indexer(const flat_indexer&) = default;

    /**
     * @brief Copy assignment operator.
     */
    flat_indexer& operator = (const flat_indexer&) = default;

    /**
     * @brief Move constructor.
     */
    flat_indexer(flat_indexer&&) = default;

    /**
     * @brief Converts multi-dimensional indices to flat index.
     *
     * @tparam Idx Index types (must match Dim).
     * @param indices The indices for each dimension.
     * @return The flat index.
     * @throw std::out_of_range If any index >= size.
     */
    template <typename... Idx>
    size_t to_flat(Idx... indices) const
    {
        static_assert(sizeof...(Idx) == Dim, "Wrong number of indices");

        size_t flat = 0;
        std::array<size_t, Dim> idx{static_cast<size_t>(indices)...};

        for (size_t d = 0; d < Dim; ++d)
        {
            if (idx[d] >= sizes_[d])
            {
                throw std::out_of_range("Index out of range");
            }
            flat += idx[d] * strides_[d];
        }

        return flat;
    }

    /**
     * @brief Gets the total number of elements.
     *
     * @return The total size.
     */
    size_t get_total_size() const
    {
        return total_size_;
    }
    
    /**
     * @brief Validates if sizes match another indexer.
     *
     * @param other The other indexer.
     * @throw std::invalid_argument If sizes don't match.
     */
    void validate_sizes(const flat_indexer<Dim>& other) const
    {
        if (sizes_ != other.sizes_)
        {
            throw std::invalid_argument("Sizes don't match");
        }
    }
};

/**
 * @class flat_indexer<1>
 * @brief Specialization for 1D indexer.
 */
template<>
class flat_indexer<1>
{
public:
    /**
     * @brief Constructs the 1D indexer.
     *
     * @param sizes Array with single size.
     * @throw std::invalid_argument If size == 0.
     */
    flat_indexer(const std::array<size_t, 1>& sizes)
        :size_(sizes[0])
    {
        if (size_ == 0)
        {
            throw std::invalid_argument("Size must be greater than 0");
        }
    }
    
    /**
     * @brief Converts 1D index to flat (itself).
     *
     * @param idx The index.
     * @return The index.
     * @throw std::out_of_range If idx >= size.
     */
    size_t to_flat(size_t idx) const
    {
        if (idx >= size_)
        {
            throw std::out_of_range("Index out of range");
        }
        return idx;
    }
    
    /**
     * @brief Gets the total size.
     *
     * @return The size.
     */
    size_t get_total_size() const
    {
        return size_;
    }

    /**
     * @brief Validates if sizes match another 1D indexer.
     *
     * @param other The other indexer.
     * @throw std::invalid_argument If sizes don't match.
     */
    void validate_sizes(const flat_indexer<1>& other) const
    {
        if (size_ != other.size_)
        {
            throw std::invalid_argument("Sizes don't match");
        }
    }
    
private:
    std::size_t size_; //!< The size of the single dimension.
};


} // namespace muskox
