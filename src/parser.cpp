#include "parser.h"
#include "parse_context.h"
#include "parse_error.h"

#include "symbol_stream_from_vector.h"

#include <optional>

namespace muskox
{

parser::parser(parse_table&& pt, name_table&& nt, rr_table&& rt)
    : pt_(std::move(pt)), nt_(std::move(nt)), rt_(std::move(rt))
{
}

template<typename Stream>
size_t parser::get_lookahead(Stream& stream) const
{
    auto token = stream.next();
    return token ? *token : 0; // eof_idx = 0
}

template<typename Stream>
parse_result parser::parse(Stream& stream) const
{
    parse_context ctx;

    ctx.push_state_on_stack(0, 0);

    size_t term_idx = get_lookahead(stream);
    
    while (!ctx.stack_empty(0))
    {
        size_t current_state = ctx.get_back_state(0);

        const parse_table_entry& entry = pt_.get_term_entry(current_state, term_idx);

        if (entry.is_shift())
        {
            do_shift(ctx, entry.get_shift_state_idx(), term_idx);
            term_idx = get_lookahead(stream);
        }
        else if (entry.is_reduce())
        {
            size_t nterm_idx = entry.get_reduce_nterm_idx();
            size_t reduce_length = entry.get_reduce_length();

            if (do_reduce(ctx, nterm_idx, reduce_length, term_idx))
            {
                return parse_result::ok();
            }
        }
        else
        {
            do_error(ctx, current_state, term_idx);
            return parse_result::error(ctx.take_errors());
        }
    }

    return parse_result::error(ctx.take_errors());
}

void parser::do_shift(parse_context& ctx, size_t new_state, size_t term_idx) const
{
    size_t from_state = ctx.get_back_state(0);
    ctx.push_state_on_stack(0, new_state);
    ctx.on_shift(0, from_state, new_state, term_idx);
}

bool parser::do_reduce(parse_context& ctx, size_t nterm_idx, size_t reduce_length, size_t term_idx) const
{
    if (!ctx.pop_n_from_stack(0, reduce_length))
    {
        do_error(ctx, ctx.get_back_state(0), term_idx);
        return false;
    }

    if (nterm_idx == 0) // $root index = 0
    {
        if (term_idx != 0)
        {
            do_error(ctx, ctx.stack_empty(0) ? 0 : ctx.get_back_state(0), term_idx);
            return false;
        }
        do_success(ctx);
        return true;
    }

    if (ctx.stack_empty(0))
    {
        do_error(ctx, 0, term_idx);
        return false;
    }

    size_t current_state = ctx.get_back_state(0);
    const parse_table_entry& goto_entry = pt_.get_nterm_entry(current_state, nterm_idx);

    if (!goto_entry.is_shift())
    {
        do_error(ctx, current_state, term_idx);
        return false;
    }

    ctx.push_state_on_stack(0, goto_entry.get_shift_state_idx());
    ctx.on_reduce(0, nterm_idx, reduce_length);
    return false;
}

void parser::do_error(parse_context& ctx, size_t state, size_t term_idx) const
{
    symbol_ref lookahead{symbol_type::terminal, term_idx};
    ctx.on_error(0, state, lookahead);
    syntax_error(ctx, term_idx);
}

void parser::do_success(parse_context& ctx) const
{
    ctx.on_accept(0);
}

void parser::syntax_error(parse_context& ctx, size_t term_idx) const
{
    std::string_view symbol = nt_.get_term_name(term_idx);
    parse_error err(parse_error_templates::code::syntax_error, symbol);
    ctx.add_error(err.str());
}

template parse_result parser::parse<symbol_stream_from_vector>(symbol_stream_from_vector&) const;

} // namespace muskox
