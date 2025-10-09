#pragma once

#include <array>
#include <vector>
#include <stdexcept>
#include <numeric>

namespace ptg
{

template <size_t Dim>
class base_index_subset
{
private:
    std::array<size_t, Dim> sizes_;
    std::array<size_t, Dim> strides_;
    std::vector<bool> bits_;

    void compute_strides()
    {
        if (Dim == 0)
        {
            return;
        }
        
        strides_[Dim - 1] = 1;
        for (size_t d = Dim - 1; d-- > 0; )
        {
            strides_[d] = strides_[d + 1] * sizes_[d + 1];
        }
    }

    template <typename... Idx>
    size_t to_flat(Idx... indices) const
    {
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

public:
    base_index_subset(const std::array<size_t, Dim>& sizes, bool value = false)
        : sizes_(sizes), strides_(), bits_()
    {
        if (Dim == 0)
        {
            throw std::invalid_argument("At least one dimension required");
        }

        size_t prod = 1;

        for (auto s : sizes_)
        {
            if (s == 0)
            {
                throw std::invalid_argument("Size must be greater than 0");
            }
            prod *= s;
        }
        
        bits_.reserve(prod);
        bits_.assign(prod, value);
        compute_strides();
    }

    ~base_index_subset() = default;

    template <typename... Idx>
    bool add(Idx... indices)
    {
        static_assert(sizeof...(Idx) == Dim, "Wrong number of indices");
        
        size_t flat = to_flat(indices...);
        if (bits_[flat])
        {
            return false;
        }
        bits_[flat] = true;
        return true;
    }
    
    template <typename... Idx>
    bool remove(Idx... indices)
    {
        static_assert(sizeof...(Idx) == Dim, "Wrong number of indices");
        
        size_t flat = to_flat(indices...);
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
        static_assert(sizeof...(Idx) == Dim, "Wrong number of indices");
        
        size_t flat = to_flat(indices...);
        return bits_[flat];
    }
    
    size_t get_size() const
    {
        return bits_.size();
    }
};

} // namespace ptg
