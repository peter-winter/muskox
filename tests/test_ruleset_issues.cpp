#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <grammar_error.h>
#include <ruleset.h>
#include <symbol_collection.h>

using Catch::Matchers::Message;

using namespace muskox;

TEST_CASE("ruleset issues no productions", "[ruleset]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
    
    sc.validate();
    
    ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a", "B"});
    
    rs.validate();

    REQUIRE(rs.get_errors().size() == 1);
    REQUIRE(rs.get_errors()[0] == "Nonterminal 'B' has no productions");
    REQUIRE(rs.get_warnings().empty());
    REQUIRE(rs.is_validated() == true);
}

TEST_CASE("ruleset issues unused symbols", "[ruleset]")
{
    symbol_collection sc;
    [[maybe_unused]] size_t s_idx = sc.add_nterm("S");
    [[maybe_unused]] size_t a_idx = sc.add_term("a");
    [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
    [[maybe_unused]] size_t c_idx = sc.add_term("c");
    [[maybe_unused]] size_t u_idx = sc.add_nterm("U");
    [[maybe_unused]] size_t v_idx = sc.add_term("v");
    
    sc.validate();

    ruleset rs(sc);
    [[maybe_unused]] size_t s_r0 = rs.add_rule("S", {"a", "B"});
    [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"c"});
    [[maybe_unused]] size_t u_r0 = rs.add_rule("U", {"v"});

    rs.validate();
    
    REQUIRE(rs.get_warnings().size() == 2);
    REQUIRE(rs.get_warnings()[0] == "Nonterminal 'U' is unused");
    REQUIRE(rs.get_warnings()[1] == "Terminal 'v' is unused");
}

TEST_CASE("ruleset issues unresolved nterm", "[ruleset]")
{
    SECTION("direct left recursion no base")
    {
        symbol_collection sc;
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_term_idx = sc.add_term("b");
        sc.validate();

        ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"A", "b"});

        rs.validate();
        
        REQUIRE(rs.get_errors().size() == 1);
        REQUIRE(rs.get_errors()[0] == "Nonterminal 'A' is unsolvable");
    }

    SECTION("indirect left recursion no base")
    {
        symbol_collection sc;
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
        [[maybe_unused]] size_t c_term_idx = sc.add_term("c");
        sc.validate();

        ruleset rs(sc);
        [[maybe_unused]] size_t a_r0 = rs.add_rule("A", {"B"});
        [[maybe_unused]] size_t b_r0 = rs.add_rule("B", {"A", "c"});

        rs.validate();
        REQUIRE(rs.get_errors().size() == 2);
        REQUIRE(rs.get_errors()[0] == "Nonterminal 'A' is unsolvable");
        REQUIRE(rs.get_errors()[1] == "Nonterminal 'B' is unsolvable");
    }
}
