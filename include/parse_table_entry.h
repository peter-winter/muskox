#pragma once

#include <cstdint>
#include <cstddef>
#include <stdexcept>
#include <limits>

namespace muskox
{

enum class entry_type : uint8_t { error, shift, reduce, rr_conflict };

class parse_table_entry
{
public:
    struct reduction
    {
        uint16_t nterm_idx_;
        uint16_t rside_idx_;
    };
    
    // Default constructor (error)
    parse_table_entry() = default;
    
    static parse_table_entry shift(size_t state_idx);
    static parse_table_entry reduce(size_t nterm_idx, size_t rside_idx);
    static parse_table_entry rr_conflict(size_t rr_conflict_start_idx, size_t rr_conflict_count);
    
    void set_shift(size_t state_idx);
    void set_reduce(size_t nterm_idx, size_t rside_idx);
    void set_rr_conflict(size_t rr_conflict_start_idx, size_t rr_conflict_count);
    
    bool is_error() const;
    bool is_shift() const;
    bool is_reduce() const;
    bool is_rr_conflict() const;
    
    size_t get_shift_state_idx() const;
    size_t get_reduce_nterm_idx() const;
    size_t get_reduce_rside_idx() const;
    size_t get_rr_conflict_start_idx() const;
    size_t get_rr_conflict_count() const;
    
    bool operator==(const parse_table_entry& other) const;

private:
    entry_type type_ = entry_type::error;
    union
    {
        reduction reduce_;

        uint32_t shift_state_idx_;
        
        struct
        {
            uint16_t rr_conflict_start_idx_;
            uint16_t rr_conflict_count_;
        } rr_conflict_;
    } data_ = {};
};

}
