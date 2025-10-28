#pragma once

#include <vector>
#include <memory>
#include <stdexcept>
#include <algorithm>

/**
 * @file stack_tree.h
 * @brief A tree-like structure for managing branchable stack sequences with shared prefixes.
 *
 * This design implements a specialized tree where each node contains a vector of elements of type T.
 * Child nodes reference a prefix of their parent's vector up to a specified length (prefix_length_),
 * allowing efficient sharing of common history without copying data. Paths from root to leaves represent
 * sequences (e.g., stack histories), and iterators enable backward traversal along these paths.
 *
 * Key features:
 * - Branching: Create alternative paths from a point in a sequence.
 * - Pushing/Popping: Modify leaf nodes to extend or reduce sequences.
 * - Reducing: Backtrack by popping or decrementing along a path.
 * - Traversal: Backward navigation without modification.
 *
 * This is useful for backtracking algorithms, persistent stacks, parsing trees, or any scenario requiring
 * branchable sequences with shared prefixes. Operations are restricted to leaves for safety, and empty
 * nodes are automatically pruned.
 *
 * Part of the larger PTG project.
 */

namespace ptg {

template <typename T> class tree_node;
template <typename T> class iterator;

/**
 * @brief Creates a new branch from the given iterator with an initial value.
 * @param it The iterator to branch from (must be valid and at a leaf node).
 * @param first_value The initial value for the new child node's data.
 * @return Iterator to the new child's initial element.
 * @throw std::runtime_error If the iterator is invalid or not at a leaf node.
 */
template <typename T>
iterator<T> branch(const iterator<T>& it, T&& first_value);

/**
 * @brief Reduces the sequence by n steps, popping from leaves or decrementing otherwise.
 * @param it The starting iterator (must be valid if n > 0).
 * @param n The number of steps to reduce.
 * @return The resulting iterator after reduction (may be invalid if reduced beyond root).
 * @throw std::runtime_error If n > 0 and the starting iterator is invalid.
 */
template <typename T>
iterator<T> reduce(const iterator<T>& it, size_t n);

/**
 * @brief Traverses backward n steps without modifying the structure.
 * @param it The starting iterator (must be valid if n > 0).
 * @param n The number of steps to traverse back.
 * @return The resulting iterator after traversal (may be invalid if traversed beyond root).
 * @throw std::runtime_error If n > 0 and the starting iterator is invalid.
 */
template <typename T>
iterator<T> traverse_back(const iterator<T>& it, size_t n);

/**
 * @class tree_node
 * @brief A node in the stack tree, holding local data and references to parent prefix.
 *
 * Each node manages a vector of data elements and a list of children. The prefix_length_ determines
 * how much of the parent's data is included in the path's sequence.
 */
template <typename T>
class tree_node : public std::enable_shared_from_this<tree_node<T>>
{
private:
    std::vector<T> data_; ///< Local data elements in this node.
    std::weak_ptr<tree_node> parent_weak_; ///< Weak pointer to parent to avoid cycles.
    size_t prefix_length_ = 0; ///< Length of parent's data prefix visible to this node.
    std::vector<std::shared_ptr<tree_node>> children_; ///< List of child nodes.

    /**
     * @brief Attempts to prune this node if empty and removes it from parent.
     * @return True if pruned (and should propagate), false otherwise.
     *
     * Pruning only occurs if the node has no data and no children (after checking empty()).
     */
    bool prune()
    {
        auto parent = get_parent();
        if (!parent || !empty())
        {
            return false;
        }
        parent->remove_child(this->shared_from_this());
        return true;
    }

    /**
     * @brief Validates if the index is within bounds.
     * @param idx The index to validate.
     * @throw std::out_of_range If idx >= data_.size().
     */
    void validate_out_of_bounds(size_t idx) const
    {
        if (idx >= data_.size())
        {
            throw std::out_of_range("index out of bounds");
        }
    }

public:    
    /**
     * @brief Default constructor for internal use (e.g., by make_shared).
     */
    tree_node() = default;
    
    /**
     * @brief Creates a new root node.
     * @return Shared pointer to the new root node.
     */
    static std::shared_ptr<tree_node<T>> create_root()
    {
        return std::make_shared<tree_node<T>>();
    }

    /**
     * @brief Creates a new child node branched from a parent at a given offset.
     * @param parent The parent node.
     * @param offset The offset in parent's data to branch from.
     * @param first_value The initial value for the child's data.
     * @return Shared pointer to the new child node.
     *
     * Sets prefix_length_ to offset + 1 to include the element at the branch point.
     */
    static std::shared_ptr<tree_node<T>> create_branch(std::shared_ptr<tree_node<T>> parent, size_t offset, T&& first_value)
    {
        auto ptr = std::make_shared<tree_node<T>>();
        ptr->parent_weak_ = parent;
        ptr->prefix_length_ = offset + 1;
        ptr->data_.push_back(std::forward<T>(first_value));
        parent->children_.push_back(ptr);
        return ptr;
    }

    /**
     * @brief Checks if this node is a leaf (no children).
     * @return True if no children, false otherwise.
     */
    bool is_leaf() const
    {
        return children_.empty();
    }

    /**
     * @brief Checks if this node's data is empty.
     * @return True if data_ is empty, false otherwise.
     */
    bool empty() const
    {
        return data_.empty();
    }

    /**
     * @brief Gets the size of this node's local data.
     * @return Number of elements in data_.
     */
    size_t data_size() const
    {
        return data_.size();
    }

    /**
     * @brief Accesses the element at the given index in data_ (non-const).
     * @param idx The index to access.
     * @return Reference to the element.
     * @throw std::out_of_range If idx is out of bounds.
     */
    T& at(size_t idx)
    {
        validate_out_of_bounds(idx);
        return data_[idx];
    }

    /**
     * @brief Accesses the element at the given index in data_ (const).
     * @param idx The index to access.
     * @return Const reference to the element.
     * @throw std::out_of_range If idx is out of bounds.
     */
    const T& at(size_t idx) const
    {
        validate_out_of_bounds(idx);
        return data_[idx];
    }

    /**
     * @brief Gets the parent node.
     * @return Shared pointer to parent, or nullptr if expired or root.
     */
    std::shared_ptr<tree_node<T>> get_parent() const
    {
        return parent_weak_.lock();
    }

    /**
     * @brief Gets the prefix length from the parent.
     * @return The prefix_length_ value.
     */
    size_t get_prefix_length() const
    {
        return prefix_length_;
    }

    /**
     * @brief Gets an iterator to the last element in a leaf node's data.
     * @return Iterator to the leaf element.
     * @throw std::runtime_error If not a leaf or empty.
     */
    iterator<T> get_leaf_element()
    {
        if (!is_leaf())
        {
            throw std::runtime_error("can only get leaf element from leaf nodes");
        }
        if (empty())
        {
            throw std::runtime_error("cannot get leaf element from empty node");
        }
        return iterator<T>(this->shared_from_this(), data_.size() - 1);
    }

    /**
     * @brief Pops the last element from a leaf node's data and prunes if necessary.
     * @return Iterator to the new leaf element, or invalid if fully pruned.
     * @throw std::runtime_error If not a leaf or empty.
     *
     * After popping, prunes upward if nodes become empty (no data, no children).
     */
    iterator<T> pop()
    {
        if (!is_leaf())
        {
            throw std::runtime_error("can only pop from leaf nodes");
        }
        if (empty())
        {
            throw std::runtime_error("cannot pop from empty node");
        }
        data_.pop_back();
        auto current = this->shared_from_this();
        while (current->prune())
        {
            // Propagate pruning to parent if current was removed.
            current = current->get_parent();
        }
        if (current->empty())
        {
            return iterator<T>();
        }
        return current->get_leaf_element();
    }

    /**
     * @brief Removes a specific child from this node's children list.
     * @param child The child to remove.
     */
    void remove_child(const std::shared_ptr<tree_node<T>>& child)
    {
        std::erase(children_, child);
    }

    /**
     * @brief Pushes a value to the end of a leaf node's data and returns iterator to it.
     * @tparam U The type of value (forwarded to T).
     * @param value The value to push.
     * @return Iterator to the newly pushed element.
     * @throw std::runtime_error If not a leaf.
     */
    template <typename U>
    iterator<T> push_back(U&& value)
    {
        if (!is_leaf())
        {
            throw std::runtime_error("can only push to leaf nodes");
        }
        data_.push_back(std::forward<U>(value));
        return iterator<T>(this->shared_from_this(), data_.size() - 1);
    }
};

/**
 * @class iterator
 * @brief Backward-traversing iterator for stack tree paths.
 *
 * Allows decrementing along the logical sequence: current node's data backward,
 * then parent's prefix backward, etc.
 */
template <typename T>
class iterator
{
private:
    std::shared_ptr<tree_node<T>> current_; ///< Current node.
    size_t offset_; ///< Offset within current node's data.

    /**
     * @brief Decrements the iterator position.
     * @throw std::runtime_error If already invalid.
     *
     * Traverses backward within the node, then to parent's prefix if needed,
     * continuing upward if prefix_length_ is 0.
     */
    void decrement()
    {
        if (!is_valid())
        {
            throw std::runtime_error("cannot decrement invalid iterator");
        }
        while (true)
        {
            if (offset_ > 0)
            {
                --offset_;
                return;
            }
            auto parent = current_->get_parent();
            if (!parent)
            {
                // Reached root with offset 0: invalidate.
                current_ = nullptr;
                offset_ = 0;
                return;
            }
            size_t pl = current_->get_prefix_length();
            current_ = parent;
            if (pl > 0)
            {
                // Jump to end of parent's prefix.
                offset_ = pl - 1;
                return;
            }
            // Prefix length 0: skip this parent, continue up.
        }
    }

public:
    /**
     * @brief Constructs an iterator (possibly invalid).
     * @param node The node to point to.
     * @param off The offset in the node's data.
     *
     * Validity is checked separately via is_valid().
     */
    iterator(std::shared_ptr<tree_node<T>> node = nullptr, size_t off = 0)
    : current_(node), offset_(off)
    {
        // No check in constructor for flexibility, validity checked separately
    }

    /**
     * @brief Dereferences the iterator to access the element.
     * @return Reference to the element at current position.
     * @throw std::runtime_error If invalid.
     */
    T& operator*()
    {
        if (!is_valid())
        {
            throw std::runtime_error("invalid iterator");
        }
        return current_->at(offset_);
    }

    /**
     * @brief Checks if the iterator is valid.
     * @return True if valid (points to a real element), false otherwise.
     *
     * Ensures current_ exists, offset_ < data_size(), and handles empty nodes.
     */
    bool is_valid() const
    {
        if (!current_)
        {
            return false;
        }
        if (offset_ >= current_->data_size())
        {
            return false;
        }
        if (current_->empty() && offset_ == 0)
        {
            return false;
        }
        return true;
    }

    /**
     * @brief Checks if the current node is a leaf.
     * @return True if valid and current node is leaf, false otherwise.
     */
    bool is_leaf_node() const
    {
        if (!is_valid())
        {
            return false;
        }
        return current_->is_leaf();
    }

    /**
     * @brief Checks if at the last element of a leaf node.
     * @return True if valid, leaf, non-empty, and at end, false otherwise.
     */
    bool is_leaf_element() const
    {
        if (!is_valid())
        {
            return false;
        }
        return current_->is_leaf() && !current_->empty() && offset_ == current_->data_size() - 1;
    }

    /**
     * @brief Gets the current node.
     * @return Shared pointer to current node.
     */
    std::shared_ptr<tree_node<T>> get_current() const
    {
        return current_;
    }

    /**
     * @brief Gets the current offset.
     * @return The offset_ value.
     */
    size_t get_offset() const
    {
        return offset_;
    }

    /**
     * @brief Pre-decrement operator.
     * @return Reference to this iterator after decrement.
     */
    iterator& operator--()
    {
        decrement();
        return *this;
    }

    /**
     * @brief Post-decrement operator.
     * @return Copy of iterator before decrement.
     */
    iterator operator--(int)
    {
        iterator temp = *this;
        decrement();
        return temp;
    }
    
    /**
     * @brief Equality operator for iterators.
     * @param other The other iterator to compare with.
     * @return True if both point to the same node and offset, false otherwise.
     */
    bool operator==(const iterator& other) const
    {
        return current_ == other.current_ && offset_ == other.offset_;
    }
};

template <typename T>
iterator<T> branch(const iterator<T>& it, T&& first_value)
{
    if (!it.is_valid())
    {
        throw std::runtime_error("invalid iterator for branching");
    }
    if (!it.is_leaf_node())
    {
        throw std::runtime_error("can only branch from leaf nodes");
    }
    auto child = tree_node<T>::create_branch(it.get_current(), it.get_offset(), std::forward<T>(first_value));
    return iterator<T>(child, 0);
}

template <typename T>
iterator<T> reduce(const iterator<T>& it, size_t n)
{
    if (n == 0)
    {
        return it;
    }
    if (!it.is_valid())
    {
        throw std::runtime_error("invalid iterator for reduce");
    }
    iterator<T> result = it;
    for (size_t i = 0; i < n; ++i)
    {
        if (!result.is_leaf_element())
        {
            // Not at leaf end: just move back without pop.
            --result;
            if (!result.is_valid())
            {
                break;
            }
            continue;
        }
        // At leaf end: pop and get new position.
        result = result.get_current()->pop();
        if (!result.is_valid())
        {
            break;
        }
    }
    return result;
}

template <typename T>
iterator<T> traverse_back(const iterator<T>& it, size_t n)
{
    if (n == 0)
    {
        return it;
    }
    if (!it.is_valid())
    {
        throw std::runtime_error("invalid iterator for traverse_back");
    }
    iterator<T> result = it;
    for (size_t i = 0; i < n; ++i)
    {
        --result;
        if (!result.is_valid())
        {
            break;
        }
    }
    return result;
}

} // namespace ptg
