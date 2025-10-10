#pragma once

#include <array>
#include <stdexcept>
#include <numeric>

namespace ptg
{

template <size_t Dim>
class flat_indexer
{
private:
    std::array<size_t, Dim> sizes_;
    std::array<size_t, Dim> strides_;
    size_t total_size_;

    void compute_strides()
    {
        if (Dim == 0)
        {
            return;
        }

        strides_[Dim - 1] = 1;
        for (size_t d = Dim - 1; d-- > 0;)
        {
            strides_[d] = strides_[d + 1] * sizes_[d + 1];
        }
    }

public:
    flat_indexer(const std::array<size_t, Dim>& sizes)
        : sizes_(sizes), strides_(), total_size_(1)
    {
        if (Dim == 0)
        {
            throw std::invalid_argument("At least one dimension required");
        }

        for (auto s : sizes_)
        {
            if (s == 0)
            {
                throw std::invalid_argument("Size must be greater than 0");
            }
            total_size_ *= s;
        }

        compute_strides();
    }

    ~flat_indexer() = default;
    flat_indexer(const flat_indexer&) = default;
    flat_indexer& operator = (const flat_indexer&) = default;
    flat_indexer(flat_indexer&&) = default;

    template <typename... Idx>
    size_t to_flat(Idx... indices) const
    {
        static_assert(sizeof...(Idx) == Dim, "Wrong number of indices");

        size_t flat = 0;
        std::array<size_t, Dim> idx{static_cast<size_t>(indices)...};

        for (size_t d = 0; d < Dim; ++d)
        {
            if (idx[d] >= sizes_[d])
            {
                throw std::out_of_range("Index out of range");
            }
            flat += idx[d] * strides_[d];
        }

        return flat;
    }

    size_t get_total_size() const
    {
        return total_size_;
    }
    
    void validate_sizes(const flat_indexer<Dim>& other) const
    {
        if (sizes_ != other.sizes_)
        {
            throw std::invalid_argument("Sizes don't match");
        }
    }
};

template<>
class flat_indexer<1>
{
public:
    flat_indexer(const std::array<size_t, 1>& sizes)
        :size_(sizes[0])
    {
        if (size_ == 0)
        {
            throw std::invalid_argument("Size must be greater than 0");
        }
    }
    
    size_t to_flat(size_t idx) const
    {
        if (idx >= size_)
        {
            throw std::out_of_range("Index out of range");
        }
        return idx;
    }
    
    size_t get_total_size() const
    {
        return size_;
    }

    void validate_sizes(const flat_indexer<1>& other) const
    {
        if (size_ != other.size_)
        {
            throw std::invalid_argument("Sizes don't match");
        }
    }
    
private:
    std::size_t size_;
};


} // namespace ptg
