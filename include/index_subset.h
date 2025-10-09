#pragma once

#include <base_index_subset.h>

#include <vector>

namespace ptg
{

template <size_t Dim>
class index_subset
{
private:
    base_index_subset<Dim> base_;
    
    using element_type = std::conditional_t<Dim == 1, std::size_t, std::array<size_t, Dim>>;
    std::vector<element_type> indices_;

public:
    index_subset(const std::array<size_t, Dim>& sizes)
        : base_(sizes), indices_()
    {}
        
    ~index_subset() = default;

    template <typename... Idx>
    bool add(Idx... indices)
    {
        static_assert(sizeof...(Idx) == Dim, "Wrong number of indices");
        bool inserted = base_.add(indices...);
        if (inserted)
        {
            indices_.push_back({static_cast<size_t>(indices)...});
        }
        return inserted;
    }
    
    template <typename... Idx>
    bool contains(Idx... indices) const
    {
        static_assert(sizeof...(Idx) == Dim, "Wrong number of indices");
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
};

} // namespace ptg
