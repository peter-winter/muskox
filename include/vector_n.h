/**
 * @file vector_n.h
 * @brief Multi-dimensional vector with flat storage.
 *
 * This header provides a templated class for multi-dimensional vectors
 * stored in a flat std::vector, using flat_indexer for access.
 * Supports arbitrary dimensions and initialization with a value.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include "flat_indexer.h"

#include <vector>

namespace muskox
{

/**
 * @class vector_n
 * @brief Multi-dimensional vector with flat underlying storage.
 *
 * @tparam T The element type.
 * @tparam Dim The number of dimensions.
 */
template <typename T, size_t Dim>
class vector_n
{
private:
    flat_indexer<Dim> indexer_; /// The indexer for access.
    std::vector<T> data_; /// The flat data storage.

public:
    /**
     * @brief Constructs the vector with sizes and initial value.
     *
     * @param sizes The sizes of dimensions.
     * @param init_value The initial value for all elements.
     */
    vector_n(const std::array<size_t, Dim>& sizes, const T& init_value)
        : indexer_(sizes), data_()
    {
        data_.assign(indexer_.get_total_size(), init_value);
    }

    /**
     * @brief Destructor.
     */
    ~vector_n() = default;

    /**
     * @brief Gets mutable reference to element at indices.
     *
     * @tparam Idx Index types.
     * @param indices The indices.
     * @return Reference to the element.
     */
    template <typename... Idx>
    T& get(Idx... indices)
    {
        size_t flat = indexer_.to_flat(indices...);
        return data_[flat];
    }

    /**
     * @brief Gets const reference to element at indices.
     *
     * @tparam Idx Index types.
     * @param indices The indices.
     * @return Const reference to the element.
     */
    template <typename... Idx>
    const T& get(Idx... indices) const
    {
        size_t flat = indexer_.to_flat(indices...);
        return data_[flat];
    }

    /**
     * @brief Gets the total number of elements.
     *
     * @return The total size.
     */
    size_t get_size() const
    {
        return indexer_.get_total_size();
    }
};

} // namespace muskox
