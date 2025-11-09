#include <vector>
#include <ranges>
#include <algorithm>
#include <functional>
#include <utility>

namespace muskox
{
    
template <typename T, typename PartialComparer>
class sorted_grouped_vector
{
public:
    using partial_comparer_type = PartialComparer;
    using value_type = T;
    
    sorted_grouped_vector(partial_comparer_type cmp)
        : comparer_(std::move(cmp))
    {
    }

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
    
    std::vector<value_type> take_all()
    {
        return std::exchange(data_, {});
    }
    
private:
    std::vector<value_type> data_;
    partial_comparer_type comparer_;
};

}
