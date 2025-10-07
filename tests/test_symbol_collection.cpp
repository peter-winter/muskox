#include <catch2/catch_test_macros.hpp>

#include <symbol_collection.h>

#include <grammar_error.h>
#include <stdexcept>

TEST_CASE("symbol_collection basic operations", "[symbol_collection]")
{
    ptg::symbol_collection sc;

    SECTION("add_term and contains")
    {
        sc.add_term("term1");
        REQUIRE(sc.contains("term1") == true);
        REQUIRE(sc.contains("unknown") == false);
    }

    SECTION("add_nterm and contains")
    {
        sc.add_nterm("nterm1");
        REQUIRE(sc.contains("nterm1") == true);
        REQUIRE(sc.contains("unknown") == false);
    }

    SECTION("duplicate add throws")
    {
        sc.add_term("dup");
        REQUIRE_THROWS_AS(sc.add_term("dup"), ptg::grammar_error);
        REQUIRE_THROWS_AS(sc.add_nterm("dup"), ptg::grammar_error);

        sc.add_nterm("dup2");
        REQUIRE_THROWS_AS(sc.add_nterm("dup2"), ptg::grammar_error);
        REQUIRE_THROWS_AS(sc.add_term("dup2"), ptg::grammar_error);
    }

    SECTION("get_symbol_ref")
    {
        sc.add_term("term1");
        auto ref = sc.get_symbol_ref("term1");
        REQUIRE(ref.type_ == ptg::symbol_type::terminal);
        REQUIRE(ref.index_ == 1);  // $eof is 0

        sc.add_nterm("nterm1");
        ref = sc.get_symbol_ref("nterm1");
        REQUIRE(ref.type_ == ptg::symbol_type::non_terminal);
        REQUIRE(ref.index_ == 1);  // $root is 0

        REQUIRE_THROWS_AS(sc.get_symbol_ref("unknown"), std::out_of_range);
    }

    SECTION("get_symbol_name")
    {
        sc.add_term("term1");
        sc.add_nterm("nterm1");

        auto ref_term = sc.get_symbol_ref("term1");
        REQUIRE(sc.get_symbol_name(ref_term) == "term1");

        auto ref_nterm = sc.get_symbol_ref("nterm1");
        REQUIRE(sc.get_symbol_name(ref_nterm) == "nterm1");

        ptg::symbol_ref invalid_ref { ptg::symbol_type::terminal, 999 };
        REQUIRE_THROWS_AS(sc.get_symbol_name(invalid_ref), std::out_of_range);

        invalid_ref.type_ = ptg::symbol_type::non_terminal;
        REQUIRE_THROWS_AS(sc.get_symbol_name(invalid_ref), std::out_of_range);

        invalid_ref.type_ = static_cast<ptg::symbol_type>(99);
        REQUIRE_THROWS_AS(sc.get_symbol_name(invalid_ref), std::invalid_argument);
    }

    SECTION("get_term_name")
    {
        sc.add_term("term1");
        REQUIRE(sc.get_term_name(1) == "term1");
        REQUIRE_THROWS_AS(sc.get_term_name(2), std::out_of_range);
    }

    SECTION("get_nterm_name")
    {
        sc.add_nterm("nterm1");
        REQUIRE(sc.get_nterm_name(1) == "nterm1");
        REQUIRE_THROWS_AS(sc.get_nterm_name(2), std::out_of_range);
    }

    SECTION("get_term_assoc and get_term_prec")
    {
        sc.add_term("term1", ptg::associativity::left(), 10);
        REQUIRE(sc.get_term_assoc(1).to_string() == "left");
        REQUIRE(sc.get_term_prec(1) == 10);

        sc.add_term("term2", ptg::associativity::right(), 20);
        REQUIRE(sc.get_term_assoc(2).to_string() == "right");
        REQUIRE(sc.get_term_prec(2) == 20);

        REQUIRE_THROWS_AS(sc.get_term_assoc(3), std::out_of_range);
        REQUIRE_THROWS_AS(sc.get_term_prec(3), std::out_of_range);
    }

    SECTION("rehashing with many adds")
    {
        // Add enough to likely trigger rehash (e.g., > bucket count, typically starts small)
        for (size_t i = 0; i < 100; ++i)
        {
            std::string tname = "term" + std::to_string(i);
            sc.add_term(tname, ptg::associativity::left(), i);
            REQUIRE(sc.contains(tname) == true);
            auto ref = sc.get_symbol_ref(tname);
            REQUIRE(ref.type_ == ptg::symbol_type::terminal);
            REQUIRE(ref.index_ == i + 1);  // $eof is 0
            REQUIRE(sc.get_symbol_name(ref) == tname);
            REQUIRE(sc.get_term_name(i + 1) == tname);
            REQUIRE(sc.get_term_assoc(i + 1).to_string() == "left");
            REQUIRE(sc.get_term_prec(i + 1) == i);
        }

        for (size_t i = 0; i < 100; ++i)
        {
            std::string nname = "nterm" + std::to_string(i);
            sc.add_nterm(nname);
            REQUIRE(sc.contains(nname) == true);
            auto ref = sc.get_symbol_ref(nname);
            REQUIRE(ref.type_ == ptg::symbol_type::non_terminal);
            REQUIRE(ref.index_ == i + 1);  // $root is 0
            REQUIRE(sc.get_symbol_name(ref) == nname);
            REQUIRE(sc.get_nterm_name(i + 1) == nname);
        }

        // Verify all still accessible after potential rehashes
        for (size_t i = 0; i < 100; ++i)
        {
            std::string tname = "term" + std::to_string(i);
            REQUIRE(sc.get_term_name(i + 1) == tname);
            REQUIRE(sc.get_term_prec(i + 1) == i);

            std::string nname = "nterm" + std::to_string(i);
            REQUIRE(sc.get_nterm_name(i + 1) == nname);
        }
    }

    SECTION("counts")
    {
        REQUIRE(sc.get_term_count() == 1);  // $eof
        REQUIRE(sc.get_nterm_count() == 1);  // $root

        sc.add_term("term1");
        sc.add_term("term2");
        REQUIRE(sc.get_term_count() == 3);
        REQUIRE(sc.get_nterm_count() == 1);

        sc.add_nterm("nterm1");
        sc.add_nterm("nterm2");
        REQUIRE(sc.get_term_count() == 3);
        REQUIRE(sc.get_nterm_count() == 3);
    }

    SECTION("special symbols")
    {
        REQUIRE(sc.contains("$root") == true);
        REQUIRE(sc.contains("$eof") == true);

        auto root_ref = sc.get_symbol_ref("$root");
        REQUIRE(root_ref.type_ == ptg::symbol_type::non_terminal);
        REQUIRE(root_ref.index_ == 0);
        REQUIRE(sc.get_nterm_name(0) == "$root");

        auto eof_ref = sc.get_symbol_ref("$eof");
        REQUIRE(eof_ref.type_ == ptg::symbol_type::terminal);
        REQUIRE(eof_ref.index_ == 0);
        REQUIRE(sc.get_term_name(0) == "$eof");
        REQUIRE(sc.get_term_assoc(0).to_string() == "left");
        REQUIRE(sc.get_term_prec(0) == 0);

        REQUIRE_THROWS_AS(sc.add_nterm("$root"), ptg::grammar_error);
        REQUIRE_THROWS_AS(sc.add_term("$eof", ptg::associativity::left(), 0), ptg::grammar_error);
    }

    SECTION("term defaults")
    {
        ptg::term t1("name1");
        REQUIRE(t1.name() == "name1");
        REQUIRE(t1.assoc().to_string() == "left");
        REQUIRE(t1.prec() == 0);

        ptg::term t2("name2", ptg::associativity::right(), 5);
        REQUIRE(t2.name() == "name2");
        REQUIRE(t2.assoc().to_string() == "right");
        REQUIRE(t2.prec() == 5);
    }

    SECTION("add_term defaults")
    {
        sc.add_term("term_default");
        REQUIRE(sc.get_term_assoc(1).to_string() == "left");
        REQUIRE(sc.get_term_prec(1) == 0);

        sc.add_term("term_custom", ptg::associativity::right(), 5);
        REQUIRE(sc.get_term_assoc(2).to_string() == "right");
        REQUIRE(sc.get_term_prec(2) == 5);
    }
}
