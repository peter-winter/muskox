#pragma once

#include "parse_table.h"
#include "parse_context.h"
#include "parse_result.h"
#include "name_table.h"
#include "symbol_stream.h"

namespace muskox
{

class parser
{
public:
    parser(parse_table&& pt, name_table&& nt, rr_table&& rt);

    parse_result parse(symbol_stream& stream) const;

private:
    size_t get_lookahead(symbol_stream& stream) const;

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
