#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <lr1_state_set.h>
#include <ruleset.h>
#include <symbol_collection.h>

#include <grammar_error.h>

TEST_CASE("lr1_state_set validate", "[lr1_state_set]")
{
    ptg::symbol_collection sc;

    SECTION("with nterms no rules")
    {
        sc.add_nterm("A");
        sc.add_nterm("B");
        ptg::ruleset rs(sc, "A");
        
        REQUIRE_THROWS_MATCHES([&]{ ptg::lr1_state_set s(rs); }(), ptg::grammar_error, Catch::Matchers::Message("Nonterminal 'A' has no productions."));
    }
}
