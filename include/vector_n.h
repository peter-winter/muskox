#pragma once

#include <flat_indexer.h>
#include <vector>

namespace ptg
{

template <typename T, size_t Dim>
class vector_n
{
private:
    flat_indexer<Dim> indexer_;
    std::vector<T> data_;

public:
    vector_n(const std::array<size_t, Dim>& sizes, const T& init_value)
        : indexer_(sizes), data_()
    {
        data_.assign(indexer_.get_total_size(), init_value);
    }

    ~vector_n() = default;

    template <typename... Idx>
    T& get(Idx... indices)
    {
        size_t flat = indexer_.to_flat(indices...);
        return data_[flat];
    }

    template <typename... Idx>
    const T& get(Idx... indices) const
    {
        size_t flat = indexer_.to_flat(indices...);
        return data_[flat];
    }

    size_t get_size() const
    {
        return indexer_.get_total_size();
    }
};

} // namespace ptg
