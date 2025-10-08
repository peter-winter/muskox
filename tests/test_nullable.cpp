#include <catch2/catch_test_macros.hpp>

#include <nullable.h>
#include <ruleset.h>
#include <symbol_collection.h>

#include <grammar_error.h>

TEST_CASE("nullable sizes", "[nullable]")
{
    ptg::symbol_collection sc;

    SECTION("with nterms no rules")
    {
        sc.add_nterm("A");
        sc.add_nterm("B");
        ptg::ruleset rs(sc, "A");
        REQUIRE_THROWS_AS([&]{ ptg::nullable n(rs); }(), ptg::grammar_error);  // Multiple have no rules
    }

    SECTION("with rules varying rsides and symbols")
    {
        sc.add_nterm("A");
        sc.add_nterm("B");
        sc.add_nterm("C");
        sc.add_term("x");
        sc.add_term("y");
        sc.add_term("z");

        ptg::ruleset rs(sc, "A");
        
        rs.add_rule("A", {"x"});
        rs.add_rule("A", {});

        rs.add_rule("B", {"x", "y"});
        rs.add_rule("B", {"z"});

        rs.add_rule("C", {"x", "y", "z"});
        rs.add_rule("C", {});

        ptg::nullable n(rs);
        REQUIRE(n.get_nterms().get_size() == 4);  // $root + A + B + C
        REQUIRE(n.get_rside_parts().get_size() == 4 * 2 * 3);  // nterms=4 (incl $root), max_rsides=2, max_symbols=3
    }

    SECTION("max from different nterms")
    {
        sc.add_nterm("A");
        sc.add_nterm("B");
        sc.add_nterm("C");
        sc.add_term("x");
        sc.add_term("y");
        sc.add_term("z");
        sc.add_term("w");
        
        ptg::ruleset rs(sc, "A");
        
        rs.add_rule("A", {"x"});

        rs.add_rule("B", {});
        rs.add_rule("B", {});
        rs.add_rule("B", {});  // 3 rsides, 0 symbols

        rs.add_rule("C", {"x", "y", "z", "w"});  // 1 rside, 4 symbols

        ptg::nullable n(rs);
        REQUIRE(n.get_nterms().get_size() == 4);  // $root + A + B + C
        REQUIRE(n.get_rside_parts().get_size() == 4 * 3 * 4);  // max_rsides=3 (B), max_symbols=4 (C)
    }
}
