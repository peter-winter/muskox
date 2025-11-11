/**
 * @file sorted_grouped_vector.h
 * @brief Defines a templated container that maintains sorted order and supports grouped views.
 *
 * This header provides a generic class for storing elements in sorted order based on a partial comparer.
 * Elements are inserted using binary search for efficiency, and the container supports a grouped view
 * where consecutive elements with equal partial keys (as defined by the comparer) are chunked together.
 * The class is move-only, preventing copies to ensure efficiency with potentially large data sets.
 *
 * Useful for scenarios requiring sorted insertion and iteration over groups, such as organizing LR(1) items
 * by symbols in parsing algorithms.
 *
 * Part of the larger MuskOx project.
 */

#include <vector>
#include <ranges>
#include <algorithm>
#include <functional>

namespace muskox
{
    
/**
 * @class sorted_grouped_vector
 * @brief A move-only vector that keeps elements sorted by a partial key and provides grouped views.
 *
 * @tparam T The type of elements stored in the vector.
 * @tparam PartialComparer The type of the comparer providing the partial key and comparison logic.
 *                         Must have a `get_part` method that returns a comparable key from T.
 */
template <typename T, typename PartialComparer>
class sorted_grouped_vector
{
public:
    /**
     * @brief Type alias for the partial comparer.
     */
    using partial_comparer_type = PartialComparer;

    /**
     * @brief Type alias for the value type stored.
     */
    using value_type = T;
    
    /**
     * @brief Constructs the container with a given partial comparer.
     *
     * @param cmp The partial comparer used for sorting and grouping.
     */
    sorted_grouped_vector(partial_comparer_type cmp)
        : comparer_(cmp)
    {}
    
    /**
     * @brief Move constructor.
     */
    sorted_grouped_vector(sorted_grouped_vector&& other) = default;        

    /**
     * @brief Move assignment operator.
     */
    sorted_grouped_vector& operator = (sorted_grouped_vector&& other) = default;
    
    /**
     * @brief Copy constructor (deleted).
     */
    sorted_grouped_vector(sorted_grouped_vector&) = delete;        

    /**
     * @brief Copy assignment operator (deleted).
     */
    sorted_grouped_vector& operator = (sorted_grouped_vector&) = delete;

    /**
     * @brief Inserts a value into the container while maintaining sorted order.
     *
     * Uses binary search via std::upper_bound to find the insertion point based on the partial key.
     *
     * @param value The value to insert.
     */
    void insert(const value_type& value)
    {
        auto comp = [this](const value_type& a, const value_type& b)
        {
            return comparer_.get_part(a) < comparer_.get_part(b);
        };
        auto it = std::upper_bound(data_.begin(), data_.end(), value, comp);
        data_.insert(it, value);
    }

    /**
     * @brief Returns a grouped view of the elements using std::views::chunk_by.
     *
     * Groups are formed by consecutive elements where the partial keys are equal.
     *
     * @return A range view of chunks (groups) of elements.
     */
    auto grouped_view() const
    {
        auto equal_pred = [this](const value_type& a, const value_type& b)
        {
            return comparer_.get_part(a) == comparer_.get_part(b);
        };
        return data_ | std::views::chunk_by(equal_pred);
    }

    /**
     * @brief Gets a const reference to the underlying vector of all elements.
     *
     * @return Const reference to the data vector.
     */
    const std::vector<value_type>& get_all() const
    {
        return data_;
    }
    
    /**
     * @brief Clears the container.
     */
    void clear()
    {
        data_.clear();
    }
    
    /**
     * @brief Checks if the container is empty.
     *
     * @return True if empty, false otherwise.
     */
    bool empty() const
    {
        return data_.empty();
    }
    
private:
    std::vector<value_type> data_; /// The underlying storage vector.
    partial_comparer_type comparer_; /// The partial comparer for sorting and grouping.
};

}
