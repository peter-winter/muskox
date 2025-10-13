#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <grammar_error.h>
#include <lr1_state_set.h>
#include <ruleset.h>
#include <symbol_collection.h>

using Catch::Matchers::Message;

TEST_CASE("lr1_state_set validate", "[lr1_state_set]")
{
    ptg::symbol_collection sc;

    SECTION("with nterms no rules")
    {
        [[maybe_unused]] size_t a_idx = sc.add_nterm("A");
        [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
        ptg::ruleset rs(sc);
        
        REQUIRE_THROWS_MATCHES(
            [&]{ ptg::lr1_state_set s(rs); }(),
            ptg::grammar_error,
            Message("Nonterminal 'A' has no productions.")
        );
    }
}
