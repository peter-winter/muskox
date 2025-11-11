#include <vector>
#include <ranges>
#include <algorithm>
#include <functional>

namespace muskox
{
    
template <typename T, typename PartialComparer>
class sorted_grouped_vector
{
public:
    using partial_comparer_type = PartialComparer;
    using value_type = T;
    
    sorted_grouped_vector(partial_comparer_type cmp)
        : comparer_(cmp)
    {}
    
    sorted_grouped_vector(sorted_grouped_vector&& other) = default;        
    sorted_grouped_vector& operator = (sorted_grouped_vector&& other) = default;
    
    sorted_grouped_vector(sorted_grouped_vector&) = delete;        
    sorted_grouped_vector& operator = (sorted_grouped_vector&) = delete;

    void insert(const value_type& value)
    {
        auto comp = [this](const value_type& a, const value_type& b)
        {
            return comparer_.get_part(a) < comparer_.get_part(b);
        };
        auto it = std::upper_bound(data_.begin(), data_.end(), value, comp);
        data_.insert(it, value);
    }

    auto grouped_view() const
    {
        auto equal_pred = [this](const value_type& a, const value_type& b)
        {
            return comparer_.get_part(a) == comparer_.get_part(b);
        };
        return data_ | std::views::chunk_by(equal_pred);
    }

    const std::vector<value_type>& get_all() const
    {
        return data_;
    }
    
    void clear()
    {
        data_.clear();
    }
    
    bool empty() const
    {
        return data_.empty();
    }
    
private:
    std::vector<value_type> data_;
    partial_comparer_type comparer_;
};

}
