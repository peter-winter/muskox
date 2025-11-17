#pragma once

#include "parse_table.h"
#include "parse_context.h"
#include "parse_result.h"
#include "name_table.h"

namespace muskox
{

class parser
{
public:
    parser(parse_table&& pt, name_table&& nt, rr_table&& rt);

    template<typename Stream>
    parse_result parse(Stream& stream) const;

private:
    template<typename Stream>
    size_t get_lookahead(Stream& stream) const;

    void do_shift(parse_context& ctx, size_t new_state, size_t term_idx) const;

    bool do_reduce(parse_context& ctx, size_t nterm_idx, size_t reduce_length, size_t term_idx) const;

    void do_error(parse_context& ctx, size_t state, size_t term_idx) const;

    void do_success(parse_context& ctx) const;

    void syntax_error(parse_context& ctx, size_t term_idx) const;

    parse_table pt_;
    name_table nt_;
    rr_table rt_;
};

} // namespace muskox
