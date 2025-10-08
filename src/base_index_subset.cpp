#include <base_index_subset.h>

#include <algorithm>
#include <stdexcept>

namespace ptg
{

base_index_subset::base_index_subset(size_t n, bool value)
    : bits_(n, value)
{
    if (n == 0)
    {
        throw std::invalid_argument("Size must be greater than 0");
    }
}

bool base_index_subset::add(size_t index)
{
    if (index >= bits_.size())
    {
        throw std::out_of_range("Index out of range");
    }
    if (bits_[index])
    {
        return false;  // Already present
    }
    bits_[index] = true;
    return true;  // Newly added
}

bool base_index_subset::contains(size_t index) const
{
    if (index >= bits_.size())
    {
        throw std::out_of_range("Index out of range");
    }
    return bits_[index];
}

size_t base_index_subset::get_size() const
{
    return bits_.size();
}

} // namespace ptg
