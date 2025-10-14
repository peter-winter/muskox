#include <grammar_error.h>
#include <lr1_state_set.h>
#include <base_index_subset.h>

#include <queue>

namespace ptg
{

lr1_state_set::lr1_state_set(const ruleset& rs)
    : rs_(validate(rs)), null_(rs), warnings_()
{
    collect_unused_warnings();
}

const ruleset& lr1_state_set::validate(const ruleset& rs) const
{
    size_t n = rs.get_nterm_count();
    
    for (size_t i = 0; i < n; ++i)
    {
        if (rs.get_nterm_rside_count(i) == 0)
        {
            const std::string_view name = rs.get_nterm_name(i);
            throw grammar_error(grammar_error::code::nterm_no_rules, name);
        }
    }
    
    return rs;
}

void lr1_state_set::collect_unused_warnings()
{
    base_index_subset<1> reachable_nterms({rs_.get_nterm_count()}, false);
    std::queue<size_t> to_visit;
    size_t start_nterm = rs_.get_root().index_;
    to_visit.push(start_nterm);
    reachable_nterms.add(start_nterm);

    base_index_subset<1> used_terms({rs_.get_term_count()}, false);

    while (!to_visit.empty())
    {
        size_t curr = to_visit.front();
        to_visit.pop();

        for (size_t r = 0; r < rs_.get_nterm_rside_count(curr); ++r)
        {
            for (size_t s = 0; s < rs_.get_symbol_count(curr, r); ++s)
            {
                auto ref = rs_.get_symbol(curr, r, s);
                if (ref.type_ == symbol_type::non_terminal)
                {
                    if (reachable_nterms.add(ref.index_))
                    {
                        to_visit.push(ref.index_);
                    }
                }
                else
                {
                    used_terms.add(ref.index_);
                }
            }
        }
    }

    // Skip $root (index 0)
    for (size_t i = 1; i < rs_.get_nterm_count(); ++i)
    {
        if (!reachable_nterms.contains(i))
        {
            std::string_view name = rs_.get_nterm_name(i);
            warnings_.push_back(grammar_warning(grammar_error_templates::code::unused_nterm, name));
        }
    }

    // Skip $eof (index 0)
    for (size_t i = 1; i < rs_.get_term_count(); ++i)
    {
        if (!used_terms.contains(i))
        {
            std::string_view name = rs_.get_term_name(i);
            warnings_.push_back(grammar_warning(grammar_error_templates::code::unused_term, name));
        }
    }
}

const std::vector<std::string>& lr1_state_set::get_warnings() const
{
    return warnings_;
}

} // namespace ptg
