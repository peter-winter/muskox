#include <stdexcept>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <grammar_error.h>
#include <ruleset.h>
#include <symbol_collection.h>

using Catch::Matchers::Message;

TEST_CASE("ruleset add_rule", "[ruleset]")
{
    ptg::symbol_collection sc;
    size_t s_idx = sc.add_nterm("S");
    size_t a_idx = sc.add_term("a");
    size_t b_idx = sc.add_nterm("B");
    ptg::ruleset rs(sc);

    SECTION("basic add")
    {
        size_t ridx = rs.add_rule("S", {"a", "B"});
        REQUIRE(ridx == 0);
        REQUIRE(rs.get_nterm_rside_count(s_idx) == 1);
        REQUIRE(rs.get_symbol_count(s_idx, ridx) == 2);
        REQUIRE(rs.get_symbol_type(s_idx, ridx, 0) == ptg::symbol_type::terminal);
        REQUIRE(rs.get_symbol_index(s_idx, ridx, 0) == a_idx);
        REQUIRE(rs.get_symbol_type(s_idx, ridx, 1) == ptg::symbol_type::non_terminal);
        REQUIRE(rs.get_symbol_index(s_idx, ridx, 1) == b_idx);
        REQUIRE(rs.get_rside_precedence(s_idx, ridx) == 0);
    }

    SECTION("empty right side")
    {
        size_t ridx = rs.add_rule("S", {});
        REQUIRE(ridx == 0);
        REQUIRE(rs.get_nterm_rside_count(s_idx) == 1);
        REQUIRE(rs.get_symbol_count(s_idx, ridx) == 0);
        REQUIRE(rs.get_rside_precedence(s_idx, ridx) == 0);
    }

    SECTION("lside_not_exists")
    {
        REQUIRE_THROWS_MATCHES(
            rs.add_rule("nonexist", {}),
            ptg::grammar_error,
            Message("Left side 'nonexist' does not exist.")
        );
    }

    SECTION("lside_term")
    {
        REQUIRE_THROWS_MATCHES(
            rs.add_rule("a", {"S"}),
            ptg::grammar_error,
            Message("Left side 'a' is a terminal.")
        );
    }

    SECTION("rside_not_exist")
    {
        REQUIRE_THROWS_MATCHES(
            rs.add_rule("S", {"nonexist"}),
            ptg::grammar_error,
            Message("Right side symbol 'nonexist' does not exist.")
        );
    }
    
    SECTION("cannot_refer_special")
    {
        REQUIRE_THROWS_MATCHES(
            rs.add_rule("S", {"$eof"}),
            ptg::grammar_error,
            Message("Cannot refer special '$eof' symbol.")
        );
        REQUIRE_THROWS_MATCHES(
            rs.add_rule("S", {"$root"}),
            ptg::grammar_error,
            Message("Cannot refer special '$root' symbol.")
        );
        REQUIRE_THROWS_MATCHES(
            rs.add_rule("$eof", {"a"}),
            ptg::grammar_error,
            Message("Cannot refer special '$eof' symbol.")
        );
        REQUIRE_THROWS_MATCHES(
            rs.add_rule("$root", {"a"}),
            ptg::grammar_error,
            Message("Cannot refer special '$root' symbol.")
        );
    }

    SECTION("mixed types")
    {
        const char* cc = "a";
        std::string s = "B";
        std::string_view sv = "S";
        size_t ridx = rs.add_rule(sv, {cc, s});
        REQUIRE(ridx == 0);
        REQUIRE(rs.get_nterm_rside_count(s_idx) == 1);
        REQUIRE(rs.get_symbol_count(s_idx, ridx) == 2);
    }

    SECTION("add_rule with precedence")
    {
        size_t ridx = rs.add_rule("S", {"a", "B"}, 5);
        REQUIRE(ridx == 0);
        REQUIRE(rs.get_nterm_rside_count(s_idx) == 1);
        REQUIRE(rs.get_symbol_count(s_idx, ridx) == 2);
        REQUIRE(rs.get_symbol_type(s_idx, ridx, 0) == ptg::symbol_type::terminal);
        REQUIRE(rs.get_symbol_index(s_idx, ridx, 0) == a_idx);
        REQUIRE(rs.get_symbol_type(s_idx, ridx, 1) == ptg::symbol_type::non_terminal);
        REQUIRE(rs.get_symbol_index(s_idx, ridx, 1) == b_idx);
        REQUIRE(rs.get_rside_precedence(s_idx, ridx) == 5);
    }

    SECTION("empty rside as first prod")
    {
        size_t ridx0 = rs.add_rule("S", {});
        size_t ridx1 = rs.add_rule("S", {"a"});
        REQUIRE(ridx0 == 0);
        REQUIRE(ridx1 == 1);
        REQUIRE(rs.get_nterm_rside_count(s_idx) == 2);
        REQUIRE(rs.get_symbol_count(s_idx, ridx0) == 0);
        REQUIRE(rs.get_symbol_count(s_idx, ridx1) == 1);
        REQUIRE(rs.get_symbol_type(s_idx, ridx1, 0) == ptg::symbol_type::terminal);
        REQUIRE(rs.get_symbol_index(s_idx, ridx1, 0) == a_idx);
    }

    SECTION("empty rside as non-first prod")
    {
        size_t ridx0 = rs.add_rule("S", {"a"});
        size_t ridx1 = rs.add_rule("S", {});
        REQUIRE(ridx0 == 0);
        REQUIRE(ridx1 == 1);
        REQUIRE(rs.get_nterm_rside_count(s_idx) == 2);
        REQUIRE(rs.get_symbol_count(s_idx, ridx0) == 1);
        REQUIRE(rs.get_symbol_type(s_idx, ridx0, 0) == ptg::symbol_type::terminal);
        REQUIRE(rs.get_symbol_index(s_idx, ridx0, 0) == a_idx);
        REQUIRE(rs.get_symbol_count(s_idx, ridx1) == 0);
    }

    SECTION("out of range validations")
    {
        REQUIRE_THROWS_MATCHES(
            rs.get_nterm_rside_count(999),
            std::out_of_range,
            Message("Nterm index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            rs.get_symbol_count(s_idx, 0),
            std::out_of_range,
            Message("Rside index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            rs.get_symbol(s_idx, 0, 0),
            std::out_of_range,
            Message("Rside index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            rs.get_symbol_type(s_idx, 0, 0),
            std::out_of_range,
            Message("Rside index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            rs.get_symbol_index(s_idx, 0, 0),
            std::out_of_range,
            Message("Rside index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            rs.get_rside_precedence(s_idx, 0),
            std::out_of_range,
            Message("Rside index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            rs.get_nterm_name(999),
            std::out_of_range,
            Message("Nterm index out of range")
        );
    }

    SECTION("get_max_rside_count")
    {
        REQUIRE(rs.get_max_rside_count() == 1);  // Special rule added

        size_t ridx0 = rs.add_rule("S", {"a"});
        size_t ridx1 = rs.add_rule("S", {});
        REQUIRE(ridx0 == 0);
        REQUIRE(ridx1 == 1);
        REQUIRE(rs.get_max_rside_count() == 2);

        size_t bridx = rs.add_rule("B", {"a", "B"});
        REQUIRE(bridx == 0);
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
        REQUIRE(rs.get_nterm_name(s_idx) == "S");
        REQUIRE(rs.get_nterm_name(b_idx) == "B");
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
        ptg::ruleset rs(sc);
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
        ptg::ruleset rs(sc);
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
        ptg::ruleset rs(sc);
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
    sc.add_nterm("Other");
    
    SECTION("valid root")
    {
        ptg::ruleset rs(sc);
        REQUIRE(sc.get_symbol_name(rs.get_root()) == "S");
        REQUIRE(rs.set_root("Other") == ptg::symbol_ref{ptg::symbol_type::non_terminal, 2});
        REQUIRE(rs.get_root().index_ == 2);
        REQUIRE(sc.get_symbol_name(rs.get_root()) == "Other");
        REQUIRE(rs.get_symbol(0, 0, 0) == sc.get_symbol_ref("Other"));
    }

    SECTION("set invalid name")
    {
        ptg::ruleset rs(sc);
        REQUIRE_THROWS_MATCHES(
            rs.set_root("nonexist"),
            ptg::grammar_error,
            Message("Root symbol 'nonexist' does not exist.")
        );
    }

    SECTION("set term")
    {
        ptg::ruleset rs(sc);
        REQUIRE_THROWS_MATCHES(
            rs.set_root("a"),
            ptg::grammar_error,
            Message("Root symbol 'a' is a terminal.")
        );
    }
    
    SECTION("set $root")
    {
        ptg::ruleset rs(sc);
        REQUIRE_THROWS_MATCHES(
            rs.set_root("$root"),
            ptg::grammar_error,
            Message("Cannot refer special '$root' symbol.")
        );
    }

    SECTION("no nterms")
    {
        ptg::symbol_collection empty_sc;
        REQUIRE_THROWS_MATCHES(
            [&]{ ptg::ruleset rs(empty_sc); }(),
            ptg::grammar_error,
            Message("No nonterminals.")
        );
    }
}
