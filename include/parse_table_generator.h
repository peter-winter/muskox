#pragma once

#include <closure.h>
#include <ruleset.h>
#include <lr1_state.h>

#include <vector>
#include <string>


namespace ptg
{

class parse_table_generator
{
public:
    parse_table_generator(const ruleset& rs);
    ~parse_table_generator() = default;

    const std::vector<std::string>& get_warnings() const;
    const std::vector<std::string>& get_infos() const;
    
    std::string states_to_string() const;
    
    const std::vector<lr1_state>& get_states() const;
    
private:
    const ruleset& rs_;
    closure cl_;
    std::vector<std::string> warnings_;
    std::vector<std::string> infos_;
    std::vector<lr1_state> states_;
    
    const ruleset& validate(const ruleset& rs) const;
    void collect_unused_warnings();
    void collect_rr_conflict_warning(size_t state_idx, size_t lookahead_idx, const lr1_state::conflict& c);
    void collect_sr_conflict_info_resolution_reduce(size_t state_idx, size_t lookahead_idx, size_t nterm_idx, size_t rside_idx);
    void collect_sr_conflict_info_resolution_shift(size_t state_idx, size_t term_idx, size_t shifted_state_idx);
    void generate_states();
    
    std::optional<size_t> find_state(const index_subset<4>& kernel) const;
    
    size_t process_shift(size_t state_idx, size_t term_idx, const lr1_state::shift& s);
    void process_reduce(size_t state_idx, size_t lookahead_idx, const lr1_state::reduction& r);
    void process_conflict(size_t state_idx, size_t term_idx, const lr1_state::conflict& c);
};

} // namespace ptg
