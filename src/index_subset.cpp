#include <index_subset.h>

namespace ptg
{

index_subset::index_subset(size_t n)
    : base_(n), indices_()
{
}

bool index_subset::add(size_t index)
{
    bool inserted = base_.add(index);
    if (inserted)
    {
        indices_.push_back(index);
    }
    return inserted;
}

bool index_subset::contains(size_t index) const
{
    return base_.contains(index);
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
    return base_.get_size();
}

} // namespace ptg
