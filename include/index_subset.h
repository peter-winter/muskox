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
    index_subset(const std::array<size_t, Dim>& sizes)
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

private:
    void add(const element_type& arr)
    {
        add(arr, std::make_index_sequence<Dim>{});
    }

    template <std::size_t... I>
    void add(const element_type& arr, std::index_sequence<I...>)
    {
        add(arr[I]...);
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

} // namespace ptg
