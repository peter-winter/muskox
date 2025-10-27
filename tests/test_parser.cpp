#include <catch2/catch_test_macros.hpp>

#include <parse_table_generator.h>
#include <parser.h>
#include <ruleset.h>
#include <symbol_collection.h>
#include <symbol_stream.h>

using ptg::symbol_ref;
using ptg::symbol_type;

TEST_CASE("parser basics", "[parser]")
{
    ptg::symbol_collection sc;
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t c_idx = sc.add_term("c");
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t b_idx = sc.add_nterm("B");

    ptg::ruleset rs(sc);
    rs.add_rule("S", {"a", "B"});
    rs.add_rule("B", {"c"});

    ptg::parse_table_generator ptg(rs);
    ptg::parse_table pt = ptg.create_parse_table();

    ptg::parser p(pt, rs);

    SECTION("valid input ac")
    {
        std::vector<size_t> input {a_idx, c_idx};
        ptg::symbol_stream stream(input);
        auto res = p.parse(stream);
        REQUIRE(res.is_success());
        REQUIRE(res.get_errors().empty());
    }

    SECTION("invalid input aa")
    {
        std::vector<size_t> input {a_idx, a_idx};
        ptg::symbol_stream stream(input);
        auto res = p.parse(stream);
        REQUIRE_FALSE(res.is_success());
        REQUIRE(res.get_errors().size() == 1);
        REQUIRE(res.get_errors()[0] == "Syntax error: unexpected a");
    }

    SECTION("invalid input a")
    {
        std::vector<size_t> input {a_idx};
        ptg::symbol_stream stream(input);
        auto res = p.parse(stream);
        REQUIRE_FALSE(res.is_success());
        REQUIRE(res.get_errors().size() == 1);
        REQUIRE(res.get_errors()[0] == "Syntax error: unexpected $eof");
    }

    SECTION("invalid input c")
    {
        std::vector<size_t> input {c_idx};
        ptg::symbol_stream stream(input);
        auto res = p.parse(stream);
        REQUIRE_FALSE(res.is_success());
        REQUIRE(res.get_errors().size() == 1);
        REQUIRE(res.get_errors()[0] == "Syntax error: unexpected c");
    }

    SECTION("empty input")
    {
        std::vector<size_t> input {};
        ptg::symbol_stream stream(input);
        auto res = p.parse(stream);
        REQUIRE_FALSE(res.is_success());
        REQUIRE(res.get_errors().size() == 1);
        REQUIRE(res.get_errors()[0] == "Syntax error: unexpected $eof");
    }

    SECTION("longer invalid input acc")
    {
        std::vector<size_t> input {a_idx, c_idx, c_idx};
        ptg::symbol_stream stream(input);
        auto res = p.parse(stream);
        REQUIRE_FALSE(res.is_success());
        REQUIRE(res.get_errors().size() == 1);
        REQUIRE(res.get_errors()[0] == "Syntax error: unexpected c");
    }
}

TEST_CASE("parser with epsilon production", "[parser]")
{
    ptg::symbol_collection sc;
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t e_idx = sc.add_nterm("E");

    ptg::ruleset rs(sc);
    rs.add_rule("S", {"a", "E"});
    rs.add_rule("E", {});

    ptg::parse_table_generator ptg(rs);
    ptg::parse_table pt = ptg.create_parse_table();

    ptg::parser p(pt, rs);

    SECTION("valid input a")
    {
        std::vector<size_t> input {a_idx};
        ptg::symbol_stream stream(input);
        auto res = p.parse(stream);
        REQUIRE(res.is_success());
        REQUIRE(res.get_errors().empty());
    }

    SECTION("invalid input aa")
    {
        std::vector<size_t> input {a_idx, a_idx};
        ptg::symbol_stream stream(input);
        auto res = p.parse(stream);
        REQUIRE_FALSE(res.is_success());
        REQUIRE(res.get_errors().size() == 1);
        REQUIRE(res.get_errors()[0] == "Syntax error: unexpected a");
    }

    SECTION("invalid empty")
    {
        std::vector<size_t> input {};
        ptg::symbol_stream stream(input);
        auto res = p.parse(stream);
        REQUIRE_FALSE(res.is_success());
        REQUIRE(res.get_errors().size() == 1);
        REQUIRE(res.get_errors()[0] == "Syntax error: unexpected $eof");
    }
}
