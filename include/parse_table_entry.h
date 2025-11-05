#pragma once

#include <cstdint>
#include <cstddef>
#include <stdexcept>
#include <limits>

namespace muskox
{

enum class entry_type : uint8_t { error, shift, reduce };

class parse_table_entry
{
public:
    entry_type type = entry_type::error;

    // Default constructor (error)
    parse_table_entry() = default;
    
    static parse_table_entry shift(size_t state_idx);
    static parse_table_entry reduce(size_t nterm_idx, size_t rside_idx);
    
    void set_shift(size_t state_idx);    
    void set_reduce(size_t nterm_idx, size_t rside_idx);    
    
    bool is_error() const;
    bool is_shift() const;
    bool is_reduce() const;
    
    size_t get_shift_state_idx() const;
    size_t get_reduce_nterm_idx() const;
    size_t get_reduce_rside_idx() const;
    
    bool operator==(const parse_table_entry& other) const;

private:
    union
    {
        struct
        {
            uint16_t nterm_idx;
            uint16_t rside_idx;
        } reduce;

        uint32_t shift_state_idx;
    } data = {};
};

}
