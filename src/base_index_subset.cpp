#include <stdexcept>

#include <base_index_subset.h>

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
    validate(index);
    if (bits_[index])
    {
        return false;  // Already present
    }
    bits_[index] = true;
    return true;  // Newly added
}

bool base_index_subset::remove(size_t index)
{
    validate(index);
    if (!bits_[index])
    {
        return false;  // Not removed
    }
    bits_[index] = false;   // Removed
    return true;
}

bool base_index_subset::contains(size_t index) const
{
    validate(index);
    return bits_[index];
}

size_t base_index_subset::get_size() const
{
    return bits_.size();
}

void base_index_subset::validate(size_t index) const
{
    if (index >= bits_.size())
    {
        throw std::out_of_range("Index out of range");
    }
}

} // namespace ptg
