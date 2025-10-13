#include <stdexcept>
#include <sstream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <grammar_error.h>
#include <symbol_collection.h>

using Catch::Matchers::Message;

TEST_CASE("symbol_collection basic operations", "[symbol_collection]")
{
    ptg::symbol_collection sc;
    
    SECTION("add_term and contains")
    {
        [[maybe_unused]] size_t term1_idx = sc.add_term("term1");
        REQUIRE(sc.contains("term1") == true);
        REQUIRE(sc.contains("unknown") == false);
    }
    
    SECTION("add_nterm and contains")
    {
        [[maybe_unused]] size_t nterm1_idx = sc.add_nterm("nterm1");
        REQUIRE(sc.contains("nterm1") == true);
        REQUIRE(sc.contains("unknown") == false);
    }

    SECTION("duplicate add throws") 
    {
        [[maybe_unused]] size_t dup_idx = sc.add_term("dup");
        REQUIRE_THROWS_MATCHES(
            sc.add_term("dup"),
            ptg::grammar_error,
            Message("Symbol 'dup' already exists.")
        );
        REQUIRE_THROWS_MATCHES(
            sc.add_nterm("dup"),
            ptg::grammar_error,
            Message("Symbol 'dup' already exists.")
        );

        [[maybe_unused]] size_t dup2_idx = sc.add_nterm("dup2");
        REQUIRE_THROWS_MATCHES(
            sc.add_nterm("dup2"),
            ptg::grammar_error,
            Message("Symbol 'dup2' already exists.")
        );
        REQUIRE_THROWS_MATCHES(
            sc.add_term("dup2"),
            ptg::grammar_error,
            Message("Symbol 'dup2' already exists.")
        );
    }

    SECTION("get_symbol_ref") 
    {
        [[maybe_unused]] size_t term1_idx = sc.add_term("term1");
        auto ref = sc.get_symbol_ref("term1");
        REQUIRE(ref.type_ == ptg::symbol_type::terminal);
        REQUIRE(ref.index_ == 1);  // $eof is 0

        [[maybe_unused]] size_t nterm1_idx = sc.add_nterm("nterm1");
        ref = sc.get_symbol_ref("nterm1");
        REQUIRE(ref.type_ == ptg::symbol_type::non_terminal);
        REQUIRE(ref.index_ == 1);  // $root is 0

        REQUIRE_THROWS_MATCHES(
            sc.get_symbol_ref("unknown"),
            std::out_of_range,
            Message("Symbol not found")
        );
    }

    SECTION("get_symbol_name") 
    {
        [[maybe_unused]] size_t term1_idx = sc.add_term("term1");
        [[maybe_unused]] size_t nterm1_idx = sc.add_nterm("nterm1");

        auto ref_term = sc.get_symbol_ref("term1");
        REQUIRE(sc.get_symbol_name(ref_term) == "term1");

        auto ref_nterm = sc.get_symbol_ref("nterm1");
        REQUIRE(sc.get_symbol_name(ref_nterm) == "nterm1");

        ptg::symbol_ref invalid_ref { ptg::symbol_type::terminal, 999 };
        REQUIRE_THROWS_MATCHES(
            sc.get_symbol_name(invalid_ref),
            std::out_of_range,
            Message("Term index out of range")
        );

        invalid_ref.type_ = ptg::symbol_type::non_terminal;
        REQUIRE_THROWS_MATCHES(
            sc.get_symbol_name(invalid_ref),
            std::out_of_range,
            Message("Nterm index out of range")
        );

        invalid_ref.type_ = static_cast<ptg::symbol_type>(99);
        REQUIRE_THROWS_MATCHES(
            sc.get_symbol_name(invalid_ref),
            std::invalid_argument,
            Message("Unknown symbol type")
        );
    }

    SECTION("get_term_name") 
    {
        [[maybe_unused]] size_t term1_idx = sc.add_term("term1");
        REQUIRE(sc.get_term_name(1) == "term1");
        REQUIRE_THROWS_MATCHES(
            sc.get_term_name(2),
            std::out_of_range,
            Message("Term index out of range")
        );
    }

    SECTION("get_nterm_name") 
    {
        [[maybe_unused]] size_t nterm1_idx = sc.add_nterm("nterm1");
        REQUIRE(sc.get_nterm_name(1) == "nterm1");
        REQUIRE_THROWS_MATCHES(
            sc.get_nterm_name(2),
            std::out_of_range,
            Message("Nterm index out of range")
        );
    }

    SECTION("get_term_assoc and get_term_prec") 
    {
        [[maybe_unused]] size_t term1_idx = sc.add_term("term1", ptg::associativity::left(), 10);
        REQUIRE(sc.get_term_assoc(1).to_string() == "left");
        REQUIRE(sc.get_term_prec(1) == 10);

        [[maybe_unused]] size_t term2_idx = sc.add_term("term2", ptg::associativity::right(), 20);
        REQUIRE(sc.get_term_assoc(2).to_string() == "right");
        REQUIRE(sc.get_term_prec(2) == 20);

        REQUIRE_THROWS_MATCHES(
            sc.get_term_assoc(3),
            std::out_of_range,
            Message("Term index out of range")
        );
        REQUIRE_THROWS_MATCHES(
            sc.get_term_prec(3),
            std::out_of_range,
            Message("Term index out of range")
        );
    }

    SECTION("rehashing with many adds") 
    {
        // Add enough to likely trigger rehash (e.g., > bucket count, typically starts small)
        for (size_t i = 0; i < 100; ++i)
        {
            std::string tname = "term" + std::to_string(i);
            [[maybe_unused]] size_t term_idx = sc.add_term(tname, ptg::associativity::left(), i);
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
            [[maybe_unused]] size_t nterm_idx = sc.add_nterm(nname);
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

        [[maybe_unused]] size_t term1_idx = sc.add_term("term1");
        [[maybe_unused]] size_t term2_idx = sc.add_term("term2");
        REQUIRE(sc.get_term_count() == 3);
        REQUIRE(sc.get_nterm_count() == 1);

        [[maybe_unused]] size_t nterm1_idx = sc.add_nterm("nterm1");
        [[maybe_unused]] size_t nterm2_idx = sc.add_nterm("nterm2");
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

        REQUIRE_THROWS_MATCHES(
            sc.add_nterm("$root"),
            ptg::grammar_error,
            Message("Symbol '$root' already exists.")
        );
        REQUIRE_THROWS_MATCHES(
            sc.add_term("$eof", ptg::associativity::left(), 0),
            ptg::grammar_error,
            Message("Symbol '$eof' already exists.")
        );
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
        [[maybe_unused]] size_t term_default_idx = sc.add_term("term_default");
        REQUIRE(sc.get_term_assoc(1).to_string() == "left");
        REQUIRE(sc.get_term_prec(1) == 0);

        [[maybe_unused]] size_t term_custom_idx = sc.add_term("term_custom", ptg::associativity::right(), 5);
        REQUIRE(sc.get_term_assoc(2).to_string() == "right");
        REQUIRE(sc.get_term_prec(2) == 5);
    }

    SECTION("print_symbol_list") 
    {
        [[maybe_unused]] size_t a_idx = sc.add_term("a");
        [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
        [[maybe_unused]] size_t c_idx = sc.add_term("c");

        ptg::symbol_list sl { sc.get_symbol_ref("a"), sc.get_symbol_ref("B"), sc.get_symbol_ref("c") };
        std::stringstream ss;
        sc.print_symbol_list(ss, sl);
        REQUIRE(ss.str() == "a B c");
    }

    SECTION("print_symbol_list empty") 
    {
        ptg::symbol_list sl;
        std::stringstream ss;
        sc.print_symbol_list(ss, sl);
        REQUIRE(ss.str().empty() == true);
    }

    SECTION("print_symbol_list_from_to") 
    {
        [[maybe_unused]] size_t a_idx = sc.add_term("a");
        [[maybe_unused]] size_t b_idx = sc.add_nterm("B");
        [[maybe_unused]] size_t c_idx = sc.add_term("c");
        [[maybe_unused]] size_t d_idx = sc.add_term("d");
        [[maybe_unused]] size_t e_idx = sc.add_nterm("E");

        ptg::symbol_list sl { sc.get_symbol_ref("a"), sc.get_symbol_ref("B"), sc.get_symbol_ref("c"), sc.get_symbol_ref("d"), sc.get_symbol_ref("E") };

        std::stringstream ss;
        sc.print_symbol_list_from_to(ss, sl, 1, 4);
        REQUIRE(ss.str() == "B c d");

        ss.str("");
        sc.print_symbol_list_from_to(ss, sl, 0, 5);
        REQUIRE(ss.str() == "a B c d E");

        ss.str("");
        sc.print_symbol_list_from_to(ss, sl, 2, 3);
        REQUIRE(ss.str() == "c");

        ss.str("");
        sc.print_symbol_list_from_to(ss, sl, 0, 0);
        REQUIRE(ss.str().empty() == true);

        ss.str("");
        sc.print_symbol_list_from_to(ss, sl, 4, 5);
        REQUIRE(ss.str() == "E");

        ss.str("");
        sc.print_symbol_list_from_to(ss, sl, 5, 5);
        REQUIRE(ss.str().empty() == true);

        // If start > end, expect empty (assuming no throw)
        ss.str("");
        sc.print_symbol_list_from_to(ss, sl, 3, 2);
        REQUIRE(ss.str().empty() == true);
    }

    SECTION("print_symbol_list_from_to empty list") 
    {
        ptg::symbol_list sl;
        std::stringstream ss;
        sc.print_symbol_list_from_to(ss, sl, 0, 0);
        REQUIRE(ss.str().empty() == true);
    }

    SECTION("add_term returns index")
    {
        [[maybe_unused]] size_t idx1 = sc.add_term("term1");
        REQUIRE(idx1 == 1);  // $eof is 0

        [[maybe_unused]] size_t idx2 = sc.add_term("term2");
        REQUIRE(idx2 == 2);

        REQUIRE(sc.get_term_name(idx1) == "term1");
        REQUIRE(sc.get_term_name(idx2) == "term2");
    }

    SECTION("add_nterm returns index")
    {
        [[maybe_unused]] size_t idx1 = sc.add_nterm("nterm1");
        REQUIRE(idx1 == 1);  // $root is 0

        [[maybe_unused]] size_t idx2 = sc.add_nterm("nterm2");
        REQUIRE(idx2 == 2);

        REQUIRE(sc.get_nterm_name(idx1) == "nterm1");
        REQUIRE(sc.get_nterm_name(idx2) == "nterm2");
    }
}
