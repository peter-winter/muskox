#pragma once

#include <base_index_subset.h>

#include <vector>

namespace ptg
{

class index_subset
{
private:
    base_index_subset<1> base_;
    
    std::vector<size_t> indices_;

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
    
    void add(const index_subset& other)
    {
        if (base_.get_size() != other.base_.get_size())
        {
            throw std::invalid_argument("Sizes don't match");
        }
        
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

    const std::vector<size_t>& get_indices() const
    {
        return indices_;
    }
        
    size_t get_size() const
    {
        return base_.get_size();
    }
};

} // namespace ptg
