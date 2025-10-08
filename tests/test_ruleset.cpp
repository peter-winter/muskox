#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <ruleset.h>
#include <symbol_collection.h>

#include <grammar_error.h>
#include <stdexcept>

TEST_CASE("ruleset add_rule", "[ruleset]")
{
    ptg::symbol_collection sc;
    sc.add_nterm("S");
    sc.add_term("a", ptg::associativity::left(), 0);
    sc.add_nterm("B");
    ptg::ruleset rs(sc, "S");

    SECTION("basic add")
    {
        rs.add_rule("S", {"a", "B"});
        REQUIRE(rs.get_nterm_rside_count(1) == 1);  // S is 1
        REQUIRE(rs.get_symbol_count(1, 0) == 2);
        REQUIRE(rs.get_symbol_type(1, 0, 0) == ptg::symbol_type::terminal);
        REQUIRE(rs.get_symbol(1, 0, 0).index_ == 1);  // a is 1
        REQUIRE(rs.get_symbol_type(1, 0, 1) == ptg::symbol_type::non_terminal);
        REQUIRE(rs.get_symbol(1, 0, 1).index_ == 2);  // B is 2
        REQUIRE(rs.get_rside_precedence(1, 0) == 0);
    }

    SECTION("empty right side")
    {
        rs.add_rule("S", {});
        REQUIRE(rs.get_nterm_rside_count(1) == 1);  // S is 1
        REQUIRE(rs.get_symbol_count(1, 0) == 0);
        REQUIRE(rs.get_rside_precedence(1, 0) == 0);
    }

    SECTION("lside_not_exists")
    {
        REQUIRE_THROWS_MATCHES(rs.add_rule("nonexist", {}), ptg::grammar_error, Catch::Matchers::Message("Left side 'nonexist' does not exist."));
    }

    SECTION("lside_term")
    {
        REQUIRE_THROWS_MATCHES(rs.add_rule("a", {"S"}), ptg::grammar_error, Catch::Matchers::Message("Left side 'a' is a terminal."));
    }

    SECTION("rside_not_exist")
    {
        REQUIRE_THROWS_MATCHES(rs.add_rule("S", {"nonexist"}), ptg::grammar_error, Catch::Matchers::Message("Right side symbol 'nonexist' does not exist."));
    }
    
    SECTION("cannot_refer_special")
    {
        REQUIRE_THROWS_MATCHES(rs.add_rule("S", {"$eof"}), ptg::grammar_error, Catch::Matchers::Message("Cannot refer special '$eof' symbol."));
        REQUIRE_THROWS_MATCHES(rs.add_rule("S", {"$root"}), ptg::grammar_error, Catch::Matchers::Message("Cannot refer special '$root' symbol."));
        REQUIRE_THROWS_MATCHES(rs.add_rule("$eof", {"a"}), ptg::grammar_error, Catch::Matchers::Message("Cannot refer special '$eof' symbol."));
        REQUIRE_THROWS_MATCHES(rs.add_rule("$root", {"a"}), ptg::grammar_error, Catch::Matchers::Message("Cannot refer special '$root' symbol."));
    }

    SECTION("mixed types")
    {
        const char* cc = "a";
        std::string s = "B";
        std::string_view sv = "S";
        rs.add_rule(sv, {cc, s});
        REQUIRE(rs.get_nterm_rside_count(1) == 1);  // S is 1
        REQUIRE(rs.get_symbol_count(1, 0) == 2);
    }

    SECTION("add_rule with precedence")
    {
        rs.add_rule("S", {"a", "B"}, 5);
        REQUIRE(rs.get_nterm_rside_count(1) == 1);  // S is 1
        REQUIRE(rs.get_symbol_count(1, 0) == 2);
        REQUIRE(rs.get_symbol_type(1, 0, 0) == ptg::symbol_type::terminal);
        REQUIRE(rs.get_symbol(1, 0, 0).index_ == 1);  // a is 1
        REQUIRE(rs.get_symbol_type(1, 0, 1) == ptg::symbol_type::non_terminal);
        REQUIRE(rs.get_symbol(1, 0, 1).index_ == 2);  // B is 2
        REQUIRE(rs.get_rside_precedence(1, 0) == 5);
    }

    SECTION("empty rside as first prod")
    {
        rs.add_rule("S", {});
        rs.add_rule("S", {"a"});
        REQUIRE(rs.get_nterm_rside_count(1) == 2);  // S is 1
        REQUIRE(rs.get_symbol_count(1, 0) == 0);
        REQUIRE(rs.get_symbol_count(1, 1) == 1);
        REQUIRE(rs.get_symbol_type(1, 1, 0) == ptg::symbol_type::terminal);
        REQUIRE(rs.get_symbol(1, 1, 0).index_ == 1);  // a is 1
    }

    SECTION("empty rside as non-first prod")
    {
        rs.add_rule("S", {"a"});
        rs.add_rule("S", {});
        REQUIRE(rs.get_nterm_rside_count(1) == 2);  // S is 1
        REQUIRE(rs.get_symbol_count(1, 0) == 1);
        REQUIRE(rs.get_symbol_type(1, 0, 0) == ptg::symbol_type::terminal);
        REQUIRE(rs.get_symbol(1, 0, 0).index_ == 1);  // a is 1
        REQUIRE(rs.get_symbol_count(1, 1) == 0);
    }

    SECTION("out of range validations")
    {
        REQUIRE_THROWS_MATCHES(rs.get_nterm_rside_count(999), std::out_of_range, Catch::Matchers::Message("nterm_idx out of range"));
        REQUIRE_THROWS_MATCHES(rs.get_symbol_count(1, 0), std::out_of_range, Catch::Matchers::Message("rside_idx out of range"));
        REQUIRE_THROWS_MATCHES(rs.get_symbol(1, 0, 0), std::out_of_range, Catch::Matchers::Message("rside_idx out of range"));
        REQUIRE_THROWS_MATCHES(rs.get_symbol_type(1, 0, 0), std::out_of_range, Catch::Matchers::Message("rside_idx out of range"));
        REQUIRE_THROWS_MATCHES(rs.get_rside_precedence(1, 0), std::out_of_range, Catch::Matchers::Message("rside_idx out of range"));
        REQUIRE_THROWS_MATCHES(rs.get_nterm_name(999), std::out_of_range, Catch::Matchers::Message("Nterm index out of range"));
    }

    SECTION("get_max_rside_count")
    {
        REQUIRE(rs.get_max_rside_count() == 1);  // Special rule added

        rs.add_rule("S", {"a"});
        rs.add_rule("S", {});
        REQUIRE(rs.get_max_rside_count() == 2);

        rs.add_rule("B", {"a", "B"});
        REQUIRE(rs.get_max_rside_count() == 2);  // Still 2
    }

    SECTION("get_max_symbol_count")
    {
        REQUIRE(rs.get_max_symbol_count() == 1);  // Special rule has 1 symbol

        rs.add_rule("S", {"a"});
        REQUIRE(rs.get_max_symbol_count() == 1);

        rs.add_rule("B", {"a", "B"});
        REQUIRE(rs.get_max_symbol_count() == 2);

        rs.add_rule("S", {});
        REQUIRE(rs.get_max_symbol_count() == 2);  // Empty doesn't increase
    }

    SECTION("get_nterm_name")
    {
        REQUIRE(rs.get_nterm_name(1) == "S");
        REQUIRE(rs.get_nterm_name(2) == "B");
    }
}

TEST_CASE("ruleset to_string", "[ruleset]")
{
    ptg::symbol_collection sc;
    sc.add_nterm("S");
    sc.add_term("a");
    sc.add_term("b");
    sc.add_term("c");
    sc.add_nterm("B");

    SECTION("basic")
    {
        ptg::ruleset rs(sc, "S");
        rs.add_rule("S", {"a", "B"});
        rs.add_rule("S", {"b"});
        rs.add_rule("S", {});
        rs.add_rule("B", {"c"}, 3);

        std::string expected =
            "$root : S\n"
            "      ;\n\n"
            "S : a B\n"
            "  | b\n"
            "  | \n"
            "  ;\n\n"
            "B : c [3]\n"
            "  ;\n\n";
        REQUIRE(rs.to_string() == expected);
    }

    SECTION("empty rside as first")
    {
        ptg::ruleset rs(sc, "S");
        rs.add_rule("S", {});
        rs.add_rule("S", {"a"});

        std::string expected_empty_first =
            "$root : S\n"
            "      ;\n\n"
            "S : \n"
            "  | a\n"
            "  ;\n\n";
        REQUIRE(rs.to_string() == expected_empty_first);
    }

    SECTION("empty rside as non-first")
    {
        ptg::ruleset rs(sc, "S");
        rs.add_rule("S", {"a"});
        rs.add_rule("S", {});

        std::string expected_empty_non_first =
            "$root : S\n"
            "      ;\n\n"
            "S : a\n"
            "  | \n"
            "  ;\n\n";
        REQUIRE(rs.to_string() == expected_empty_non_first);
    }
}

TEST_CASE("ruleset root", "[ruleset]")
{
    ptg::symbol_collection sc;
    sc.add_nterm("S");
    sc.add_term("a");
    
    SECTION("valid root")
    {
        ptg::ruleset rs(sc, "S");
        REQUIRE(sc.get_symbol_name(rs.get_root()) == "S");
    }

    SECTION("set invalid name")
    {
        REQUIRE_THROWS_MATCHES([&]{ ptg::ruleset rs(sc, "nonexist"); }(), ptg::grammar_error, Catch::Matchers::Message("Root symbol 'nonexist' does not exist."));
    }

    SECTION("set term")
    {
        REQUIRE_THROWS_MATCHES([&]{ ptg::ruleset rs(sc, "a"); }(), ptg::grammar_error, Catch::Matchers::Message("Root symbol 'a' is a terminal."));
    }
    
    SECTION("set $root")
    {
        REQUIRE_THROWS_MATCHES([&]{ ptg::ruleset rs(sc, "$root"); }(), ptg::grammar_error, Catch::Matchers::Message("Cannot refer special '$root' symbol."));
    }
}   
