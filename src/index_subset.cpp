#include <index_subset.h>

#include <stdexcept>

namespace ptg
{

index_subset::index_subset(size_t n)
    : bits_(n, false), indices_()
{
    if (n == 0)
    {
        throw std::invalid_argument("Size must be greater than 0");
    }
}

void index_subset::add(size_t index)
{
    if (index >= bits_.size())
    {
        throw std::out_of_range("Index out of range");
    }
    if (!bits_[index])
    {
        bits_[index] = true;
        indices_.push_back(index);
    }
}

bool index_subset::contains(size_t index) const
{
    if (index >= bits_.size())
    {
        throw std::out_of_range("Index out of range");
    }
    return bits_[index];
}

size_t index_subset::get_count() const
{
    return indices_.size();
}

const std::vector<size_t>& index_subset::get_indices() const
{
    return indices_;
}

size_t index_subset::get_size() const
{
    return bits_.size();
}

} // namespace ptg
