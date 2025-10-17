#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <grammar_error.h>
#include <parse_table_generator.h>
#include <ruleset.h>
#include <symbol_collection.h>

using Catch::Matchers::Message;

TEST_CASE("parse_table_generator validate", "[parse_table_generator]")
{
    ptg::symbol_collection sc;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_idx = sc.add_nterm("B");

    SECTION("valid grammar")
    {
        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a"});
        [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"a"});

        REQUIRE_NOTHROW(ptg::parse_table_generator(rs));
    }

    SECTION("nterm no rules")
    {
        ptg::ruleset rs(sc);
        [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a"});

        REQUIRE_THROWS_MATCHES(
            ptg::parse_table_generator(rs),
            ptg::grammar_error,
            Message("Nonterminal 'B' has no productions.")
        );
    }
}

TEST_CASE("parse_table_generator warnings", "[parse_table_generator]")
{
    ptg::symbol_collection sc;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
    [[maybe_unused]] size_t c_idx = sc.add_term("c");
    [[maybe_unused]] size_t u_idx = sc.add_nterm("U");
    [[maybe_unused]] size_t v_idx = sc.add_term("v");

    ptg::ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a", "B"});
    [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"c"});
    [[maybe_unused]] size_t u_r0 = rs.add_rule("U", {"v"});

    ptg::parse_table_generator lss(rs);

    const auto& warnings = lss.get_warnings();
    REQUIRE(warnings.size() == 2);
    REQUIRE(warnings[0] == "Nonterminal 'U' is unused.");
    REQUIRE(warnings[1] == "Terminal 'v' is unused.");
}
