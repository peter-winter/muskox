#include <grammar_error.h>

#include <lr1_state_set.h>

namespace ptg
{

lr1_state_set::lr1_state_set(const ruleset& rs)
    : rs_(validate(rs)), null_(rs)
{
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

} // namespace ptg
