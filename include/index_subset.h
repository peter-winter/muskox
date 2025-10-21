#pragma once

#include <base_index_subset.h>

#include <array>
#include <vector>

namespace ptg
{

template <size_t Dim>
class index_subset
{
public:
    using element_type = std::array<size_t, Dim>;

private:
    base_index_subset<Dim> base_;
    std::vector<element_type> indices_;

public:
    index_subset(const element_type& sizes)
        : base_(sizes), indices_()
    {}

    ~index_subset() = default;
    index_subset(const index_subset&) = default;
    index_subset& operator = (const index_subset&) = default;
    index_subset(index_subset&&) = default;

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

    void add(const index_subset<Dim>& other)
    {
        base_.validate_sizes(other.base_);
        
        for (const auto& arr : other.indices_)
        {
            add(arr);
        }
    }

    template <typename... Idx>
    bool contains(Idx... indices) const
    {
        return base_.contains(indices...);
    }
    
    bool contains(const element_type& indices) const
    {
        return contains_impl(indices, std::make_index_sequence<Dim>{});
    }
    
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

    bool contains_only_items(const index_subset<Dim>& other) const
    {
        base_.validate_sizes(other.base_);
        return get_count() == other.get_count() && contains_all(other);
    }

    size_t get_count() const
    {
        return indices_.size();
    }

    const std::vector<element_type>& get_indices() const
    {
        return indices_;
    }

    size_t get_size() const
    {
        return base_.get_size();
    }

    void add(const element_type& indices)
    {
        add_impl(indices, std::make_index_sequence<Dim>{});
    }

private:

    template <std::size_t... I>
    bool contains_impl(const element_type& indices, std::index_sequence<I...>) const
    {
        return base_.contains(indices[I]...);
    }
    
    template <std::size_t... I>
    void add_impl(const element_type& indices, std::index_sequence<I...>)
    {
        add(indices[I]...);
    }
};

template <>
class index_subset<1>
{
public:
    using element_type = size_t;

private:
    base_index_subset<1> base_;
    std::vector<element_type> indices_;

public:
    index_subset(size_t size)
        : base_({size}), indices_()
    {}

    ~index_subset() = default;
    index_subset(const index_subset&) = default;
    index_subset& operator = (const index_subset&) = default;
    index_subset(index_subset&&) = default;

    bool add(size_t idx)
    {
        bool inserted = base_.add(idx);
        if (inserted)
        {
            indices_.push_back(idx);
        }
        return inserted;
    }

    void add(const index_subset<1>& other)
    {
        base_.validate_sizes(other.base_);
        
        for (const auto& idx : other.indices_)
        {
            add(idx);
        }
    }

    bool contains(size_t idx) const
    {
        return base_.contains(idx);
    }

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

    bool contains_only_items(const index_subset<1>& other) const
    {
        base_.validate_sizes(other.base_);
        return get_count() == other.get_count() && contains_all(other);
    }
    
    size_t get_count() const
    {
        return indices_.size();
    }

    const std::vector<element_type>& get_indices() const
    {
        return indices_;
    }

    size_t get_size() const
    {
        return base_.get_size();
    }
};

template <size_t Dim>
class index_subset_builder
{
public:
    using subset_type = index_subset<Dim>;
    using sizes_type = typename subset_type::element_type;

    index_subset_builder(const sizes_type& sizes)
        : sizes_(sizes), subset_(sizes)
    {
        reset();
    }

    template <typename... Idx>
    index_subset_builder& operator()(Idx... indices)
    {
        subset_.add(indices...);
        return *this;
    }

    subset_type build()
    {
        return subset_;
    }

    index_subset_builder& reset()
    {
        subset_ = subset_type(sizes_);
        return *this;
    }

private:
    sizes_type sizes_;
    subset_type subset_;
};

} // namespace ptg
