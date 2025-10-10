#pragma once

#include <flat_indexer.h>
#include <vector>

namespace ptg
{

template <size_t Dim>
class base_index_subset
{
private:
    flat_indexer<Dim> indexer_;
    std::vector<bool> bits_;

public:
    base_index_subset(const std::array<size_t, Dim>& sizes, bool value = false)
        : indexer_(sizes), bits_()
    {
        bits_.reserve(indexer_.get_total_size());
        bits_.assign(indexer_.get_total_size(), value);
    }

    ~base_index_subset() = default;
    base_index_subset(const base_index_subset&) = default;
    base_index_subset& operator = (const base_index_subset&) = default;
    base_index_subset(base_index_subset&&) = default;

    template <typename... Idx>
    bool add(Idx... indices)
    {
        size_t flat = indexer_.to_flat(indices...);
        if (bits_[flat])
        {
            return false;
        }
        bits_[flat] = true;
        return true;
    }

    void add(const base_index_subset<Dim>& other)
    {
        indexer_.validate_sizes(other.indexer_);
        
        for (size_t i = 0; i < bits_.size(); ++i)
        {
            bits_[i] = bits_[i] || other.bits_[i];
        }
    }
    
    template <typename... Idx>
    bool remove(Idx... indices)
    {
        size_t flat = indexer_.to_flat(indices...);
        if (!bits_[flat])
        {
            return false;
        }
        bits_[flat] = false;
        return true;
    }

    template <typename... Idx>
    bool contains(Idx... indices) const
    {
        size_t flat = indexer_.to_flat(indices...);
        return bits_[flat];
    }

    size_t get_size() const
    {
        return indexer_.get_total_size();
    }
};

} // namespace ptg
