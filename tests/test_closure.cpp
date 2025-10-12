#include <catch2/catch_test_macros.hpp>

#include <closure.h>
#include <ruleset.h>
#include <symbol_collection.h>

TEST_CASE("closure instantiation", "[closure]")
{
    ptg::symbol_collection sc;
    sc.add_nterm("S");
    sc.add_term("a");

    ptg::ruleset rs(sc);
    rs.add_rule("S", {"a"});

    ptg::closure c(rs);
}
