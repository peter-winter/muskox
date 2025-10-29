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
 * The stack_tree class serves as a wrapper managing the root node and providing public methods for operations.
 *
 * Part of the larger MuskOx project.
 */

namespace muskox
{

template <typename T> class tree_node;
template <typename T> class iterator;
template <typename T> class const_iterator;

/**
 * @brief Default no-op callable for reduce.
 */
struct noop_reduce
{
    template <typename U>
    void operator()(U&&) const noexcept {}
};

/**
 * @class stack_tree
 * @brief Wrapper class managing the root of the stack tree and providing operation methods.
 *
 * This class encapsulates the root node and exposes methods for branching, reducing, and traversing the tree.
 */
template <typename T>
class stack_tree
{
public:
    using node_ptr = std::shared_ptr<tree_node<T>>;
    using iter = iterator<T>;
    using const_iter = const_iterator<T>;

private:
    node_ptr root_;

public:
    /**
     * @brief Constructs a new stack_tree with an empty root node.
     */
    stack_tree() : root_(tree_node<T>::create_root()) {}

    /**
     * @brief Gets the root node of the tree.
     * @return Shared pointer to the root node.
     */
    node_ptr get_root() const
    {
        return root_;
    }

    /**
     * @brief Creates a new branch from the given iterator with an initial value.
     * @param it The iterator to branch from (must be valid and at a leaf node).
     * @param first_value The initial value for the new child node's data.
     * @return Iterator to the new child's initial element.
     * @throw std::runtime_error If the iterator is invalid or not at a leaf node.
     */
    iter branch(const iter& it, T&& first_value)
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
        return iter(child, 0);
    }

    /**
     * @brief Reduces the sequence by n steps, popping from leaves or decrementing otherwise, and applies a callable to each reduced element.
     * @tparam F The type of the callable (defaults to noop_reduce).
     * @param it The starting iterator (must be valid if n > 0).
     * @param n The number of steps to reduce.
     * @param f The callable to apply to each reduced element (non-const ref when popping, const ref when traversing).
     * @return The resulting iterator after reduction (may be invalid if reduced beyond root).
     * @throw std::runtime_error If n > 0 and the starting iterator is invalid.
     */
    template <typename F = noop_reduce>
    iter reduce(const iter& it, size_t n, F f = {})
    {
        if (n == 0)
        {
            return it;
        }
        if (!it.is_valid())
        {
            throw std::runtime_error("invalid iterator for reduce");
        }
        iter result = it;
        for (size_t i = 0; i < n; ++i)
        {
            if (result.is_leaf_element())
            {
                // Popping: apply f to non-const ref
                f(*result);
                // At leaf end: pop and get new position.
                result = result.get_current()->pop();
                if (!result.is_valid())
                {
                    break;
                }
            }
            else
            {
                // Traversing: apply f to const ref
                f(*const_iterator<T>(result));
                // Not at leaf end: just move back without pop.
                --result;
                if (!result.is_valid())
                {
                    break;
                }
            }
        }
        return result;
    }

    /**
     * @brief Traverses backward n steps without modifying the structure.
     * @tparam It The iterator type (iter or const_iter).
     * @param it The starting iterator (must be valid if n > 0).
     * @param n The number of steps to traverse back.
     * @return The resulting iterator after traversal (may be invalid if traversed beyond root).
     * @throw std::runtime_error If n > 0 and the starting iterator is invalid.
     */
    template <typename It>
    It traverse_back(const It& it, size_t n) const
    {
        if (n == 0)
        {
            return it;
        }
        if (!it.is_valid())
        {
            throw std::runtime_error("invalid iterator for traverse_back");
        }
        It result = it;
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
};

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
public:
    using node_ptr = std::shared_ptr<tree_node>;
    using node_const_ptr = std::shared_ptr<const tree_node>;
    using node_weak_ptr = std::weak_ptr<tree_node>;

private:
    std::vector<T> data_; ///< Local data elements in this node.
    node_weak_ptr parent_weak_; ///< Weak pointer to parent to avoid cycles.
    size_t prefix_length_ = 0; ///< Length of parent's data prefix visible to this node.
    std::vector<node_ptr> children_; ///< List of child nodes.

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

    /**
     * @brief Validates conditions for getting leaf element.
     * @throw std::runtime_error If not a leaf or empty.
     */
    void validate_get_leaf_element() const
    {
        if (!is_leaf())
        {
            throw std::runtime_error("can only get leaf element from leaf nodes");
        }
        if (empty())
        {
            throw std::runtime_error("cannot get leaf element from empty node");
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
    static node_ptr create_root()
    {
        return std::make_shared<tree_node>();
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
    static node_ptr create_branch(node_ptr parent, size_t offset, T&& first_value)
    {
        auto ptr = std::make_shared<tree_node>();
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
     * @brief Gets the parent node (non-const).
     * @return Shared pointer to parent, or nullptr if expired or root.
     */
    node_ptr get_parent()
    {
        return parent_weak_.lock();
    }

    /**
     * @brief Gets the parent node (const).
     * @return Shared pointer to const parent, or nullptr if expired or root.
     */
    node_const_ptr get_parent() const
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
        validate_get_leaf_element();
        return iterator<T>(this->shared_from_this(), data_.size() - 1);
    }

    /**
     * @brief Gets a const_iterator to the last element in a leaf node's data.
     * @return Const iterator to the leaf element.
     * @throw std::runtime_error If not a leaf or empty.
     */
    const_iterator<T> get_leaf_element() const
    {
        validate_get_leaf_element();
        auto const_this = std::const_pointer_cast<const tree_node>(this->shared_from_this());
        return const_iterator<T>(const_this, data_.size() - 1);
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
    void remove_child(const node_ptr& child)
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

namespace detail
{

/**
 * @brief Implementation of is_valid logic shared between iterator and const_iterator.
 * @tparam NodePtr The type of node pointer.
 * @param current The current node pointer.
 * @param offset The current offset.
 * @return True if valid, false otherwise.
 */
template <typename NodePtr>
bool is_valid_impl(const NodePtr& current, size_t offset)
{
    if (!current)
    {
        return false;
    }
    if (offset >= current->data_size())
    {
        return false;
    }
    if (current->empty() && offset == 0)
    {
        return false;
    }
    return true;
}

/**
 * @brief Implementation of decrement logic shared between iterator and const_iterator.
 * @tparam NodePtr The type of node pointer (shared_ptr<tree_node<T>> or shared_ptr<const tree_node<T>>).
 * @param current The current node pointer (modified on decrement).
 * @param offset The current offset (modified on decrement).
 * @throw std::runtime_error If already invalid.
 */
template <typename NodePtr>
void decrement_impl(NodePtr& current, size_t& offset)
{
    if (!is_valid_impl(current, offset))
    {
        throw std::runtime_error("cannot decrement invalid iterator");
    }
    while (true)
    {
        if (offset > 0)
        {
            --offset;
            return;
        }
        auto parent = current->get_parent();
        if (!parent)
        {
            // Reached root with offset 0: invalidate.
            current = nullptr;
            offset = 0;
            return;
        }
        size_t pl = current->get_prefix_length();
        current = parent;
        if (pl > 0)
        {
            // Jump to end of parent's prefix.
            offset = pl - 1;
            return;
        }
        // Prefix length 0: skip this parent, continue up.
    }
}

/**
 * @brief Implementation of is_leaf_node logic shared between iterator and const_iterator.
 * @tparam NodePtr The type of node pointer.
 * @param current The current node pointer.
 * @param offset The current offset (unused but included for consistency).
 * @return True if valid and leaf node, false otherwise.
 */
template <typename NodePtr>
bool is_leaf_node_impl(const NodePtr& current, size_t offset)
{
    if (!is_valid_impl(current, offset))
    {
        return false;
    }
    return current->is_leaf();
}

/**
 * @brief Implementation of is_leaf_element logic shared between iterator and const_iterator.
 * @tparam NodePtr The type of node pointer.
 * @param current The current node pointer.
 * @param offset The current offset.
 * @return True if valid, leaf, non-empty, and at end, false otherwise.
 */
template <typename NodePtr>
bool is_leaf_element_impl(const NodePtr& current, size_t offset)
{
    if (!is_valid_impl(current, offset))
    {
        return false;
    }
    return current->is_leaf() && !current->empty() && offset == current->data_size() - 1;
}

} // namespace detail

/**
 * @class iterator
 * @brief Backward-traversing iterator for stack tree paths (mutable version).
 *
 * Allows decrementing along the logical sequence: current node's data backward,
 * then parent's prefix backward, etc. Dereferences to mutable T&.
 */
template <typename T>
class iterator
{
private:
    tree_node<T>::node_ptr current_; ///< Current node.
    size_t offset_; ///< Offset within current node's data.

public:
    /**
     * @brief Constructs an iterator (possibly invalid).
     * @param node The node to point to.
     * @param off The offset in the node's data.
     *
     * Validity is checked separately via is_valid().
     */
    iterator(tree_node<T>::node_ptr node = nullptr, size_t off = 0)
    : current_(node), offset_(off)
    {
        // No check in constructor for flexibility, validity checked separately
    }

    /**
     * @brief Dereferences the iterator to access the element (mutable).
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
     */
    bool is_valid() const
    {
        return detail::is_valid_impl(current_, offset_);
    }

    /**
     * @brief Checks if the current node is a leaf.
     * @return True if valid and current node is leaf, false otherwise.
     */
    bool is_leaf_node() const
    {
        return detail::is_leaf_node_impl(current_, offset_);
    }

    /**
     * @brief Checks if at the last element of a leaf node.
     * @return True if valid, leaf, non-empty, and at end, false otherwise.
     */
    bool is_leaf_element() const
    {
        return detail::is_leaf_element_impl(current_, offset_);
    }

    /**
     * @brief Gets the current node.
     * @return Shared pointer to current node.
     */
    tree_node<T>::node_ptr get_current() const
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
        detail::decrement_impl(current_, offset_);
        return *this;
    }

    /**
     * @brief Post-decrement operator.
     * @return Copy of iterator before decrement.
     */
    iterator operator--(int)
    {
        iterator temp = *this;
        detail::decrement_impl(current_, offset_);
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

/**
 * @class const_iterator
 * @brief Backward-traversing iterator for stack tree paths (const version).
 *
 * Allows decrementing along the logical sequence: current node's data backward,
 * then parent's prefix backward, etc. Dereferences to const T&.
 */
template <typename T>
class const_iterator
{
private:
    tree_node<T>::node_const_ptr current_; ///< Current node (const).
    size_t offset_; ///< Offset within current node's data.

public:
    /**
     * @brief Constructs a const_iterator (possibly invalid).
     * @param node The node to point to (const).
     * @param off The offset in the node's data.
     *
     * Validity is checked separately via is_valid().
     */
    const_iterator(tree_node<T>::node_const_ptr node = nullptr, size_t off = 0)
    : current_(node), offset_(off)
    {
        // No check in constructor for flexibility, validity checked separately
    }

    /**
     * @brief Converting constructor from mutable iterator.
     */
    const_iterator(const iterator<T>& other)
    : current_(other.get_current()), offset_(other.get_offset())
    {}

    /**
     * @brief Dereferences the iterator to access the element (const).
     * @return Const reference to the element at current position.
     * @throw std::runtime_error If invalid.
     */
    const T& operator*() const
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
     */
    bool is_valid() const
    {
        return detail::is_valid_impl(current_, offset_);
    }

    /**
     * @brief Checks if the current node is a leaf.
     * @return True if valid and current node is leaf, false otherwise.
     */
    bool is_leaf_node() const
    {
        return detail::is_leaf_node_impl(current_, offset_);
    }

    /**
     * @brief Checks if at the last element of a leaf node.
     * @return True if valid, leaf, non-empty, and at end, false otherwise.
     */
    bool is_leaf_element() const
    {
        return detail::is_leaf_element_impl(current_, offset_);
    }

    /**
     * @brief Gets the current node (const).
     * @return Shared pointer to const current node.
     */
    tree_node<T>::node_const_ptr get_current() const
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
     * @return Reference to this const_iterator after decrement.
     */
    const_iterator& operator--()
    {
        detail::decrement_impl(current_, offset_);
        return *this;
    }

    /**
     * @brief Post-decrement operator.
     * @return Copy of const_iterator before decrement.
     */
    const_iterator operator--(int)
    {
        const_iterator temp = *this;
        detail::decrement_impl(current_, offset_);
        return temp;
    }
    
    /**
     * @brief Equality operator for const_iterators.
     * @param other The other const_iterator to compare with.
     * @return True if both point to the same node and offset, false otherwise.
     */
    bool operator==(const const_iterator& other) const
    {
        return current_ == other.current_ && offset_ == other.offset_;
    }
};

} // namespace muskox
