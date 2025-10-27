#pragma once

#include <vector>
#include <stdexcept>
#include <utility>
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <memory>

namespace ptg
{
    
template <typename value_type>
class stack : public std::enable_shared_from_this<stack<value_type>>
{
private:
    using stack_type = stack<value_type>;
    using stack_ptr = std::shared_ptr<stack_type>;

    std::vector<value_type> data_;
    std::weak_ptr<stack_type> parent_;
    std::vector<stack_ptr> children_;
    std::size_t branch_count_ = 0;

    void remove()
    {
        if (!children_.empty())
        {
            throw std::runtime_error("cannot delete stack with children");
        }
        auto p = parent_.lock();
        if (!p)
        {
            throw std::invalid_argument("cannot delete root");
        }
        auto& p_children = p->children_;
        p_children.erase(std::remove(p_children.begin(), p_children.end(), this->shared_from_this()), p_children.end());
    }

public:
    stack_ptr get_parent() const
    {
        return parent_.lock();
    }

    const std::vector<stack_ptr>& get_children() const
    {
        return children_;
    }

    stack_ptr add_child()
    {
        auto child = std::make_shared<stack_type>();
        child->parent_ = this->shared_from_this();
        children_.push_back(child);
        return child;
    }

    bool possibly_remove()
    {
        if (branch_count_ == 0 && empty() && get_children().empty())
        {
            remove();
            return true;
        }
        return false;
    }

    void push(value_type&& value)
    {
        if (!children_.empty())
        {
            throw std::invalid_argument("cannot push to non-leaf stack");
        }
        data_.push_back(std::move(value));
    }

    const value_type& top() const
    {
        if (data_.empty())
        {
            throw std::underflow_error("stack is empty");
        }
        return data_.back();
    }

    value_type& top()
    {
        if (data_.empty())
        {
            throw std::underflow_error("stack is empty");
        }
        return data_.back();
    }

    void pop(std::size_t count)
    {
        if (!children_.empty())
        {
            throw std::invalid_argument("cannot pop from non-leaf stack");
        }
        if (count == 0) return;
        std::size_t sx = data_.size();
        if (count > sx)
        {
            throw std::underflow_error("cannot pop more than available");
        }
        data_.resize(sx - count);
    }

    bool empty() const
    {
        return data_.empty();
    }

    std::size_t size() const
    {
        return data_.size();
    }

    void inc_branch_count()
    {
        branch_count_++;
    }

    void dec_branch_count()
    {
        if (branch_count_ == 0)
        {
            throw std::runtime_error("cannot decrement branch_count_ below zero");
        }
        branch_count_--;
    }

    std::size_t get_branch_count() const
    {
        return branch_count_;
    }

    void move_branch_ref_to(stack_ptr other)
    {
        dec_branch_count();
        other->inc_branch_count();
    }
};

template <typename value_type>
class branch
{
private:
    using stack_type = stack<value_type>;
    using stack_ptr = std::shared_ptr<stack_type>;

    stack_ptr stack_;
    std::size_t size_;
    
    void assign_child_stack()
    {
        auto new_stk = stack_->add_child();
        stack_->move_branch_ref_to(new_stk);
        stack_ = new_stk;
    }

    void assign_new_stack()
    {
        stack_ = std::make_shared<stack_type>();
        stack_->inc_branch_count();
    }

public:
    branch() : stack_(nullptr), size_(0)
    {
    }

    branch fork() const
    {
        branch new_b = *this;
        if (new_b.stack_)
        {
            new_b.stack_->inc_branch_count();
        }
        return new_b;
    }

    void release()
    {
        if (stack_)
        {
            stack_->dec_branch_count();
            stack_->possibly_remove();
            stack_.reset();
            size_ = 0;
        }
    }

    void commit(value_type&& val)
    {
        if (stack_ && stack_->get_branch_count() > 1)
        {
            assign_child_stack();
        }
        else if (!stack_)
        {
            assign_new_stack();
        }
        stack_->push(std::move(val));
        ++size_;
    }

    stack_ptr get_stack() const
    {
        return stack_;
    }

    std::size_t get_size() const
    {
        return size_;
    }
};

template <typename value_type>
class history
{
    static_assert(std::is_move_constructible_v<value_type>, "value_type must be movable");

private:
    std::vector<branch<value_type>> branches_;

    void check_branch_index(std::size_t idx) const
    {
        if (idx >= branches_.size())
        {
            throw std::out_of_range("invalid branch index");
        }
    }

    void create_main_branch()
    {
        branches_.emplace_back();
    }

public:
    history()
    {
        create_main_branch();
    }

    std::size_t create_branch_from(std::size_t from_idx)
    {
        check_branch_index(from_idx);
        auto& from = branches_[from_idx];
        branches_.push_back(from.fork());
        return branches_.size() - 1;
    }

    void delete_branch(std::size_t idx)
    {
        check_branch_index(idx);
        auto& b = branches_[idx];
        b.release();
        branches_.erase(branches_.begin() + idx);
    }

    const branch<value_type>& get_main_branch() const
    {
        return branches_[0];
    }
    
    branch<value_type>& get_main_branch()
    {
        check_branch_index(0);
        return branches_[0];
    }
    
    const branch<value_type>& get_branch(size_t idx) const
    {
        check_branch_index(idx);
        return branches_[idx];
    }
    
    branch<value_type>& get_branch(size_t idx)
    {
        check_branch_index(idx);
        return branches_[idx];
    }
};

}
