#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include "name_table.h"
#include "symbol_collection.h"

using Catch::Matchers::Message;

using namespace muskox;

TEST_CASE("name_table basic operations", "[name_table]")
{
    SECTION("constructor with minimal symbol_collection")
    {
        symbol_collection sc;
        sc.validate();

        name_table nt(sc);

        REQUIRE(nt.get_term_count() == 1);
        REQUIRE(nt.get_nterm_count() == 1);

        REQUIRE(std::string(nt.get_term_name(0)) == "$eof");
        REQUIRE(std::string(nt.get_nterm_name(0)) == "$root");
    }

    SECTION("constructor with added symbols")
    {
        symbol_collection sc;
        size_t term_a = sc.add_term("a");
        size_t term_b = sc.add_term("b");
        size_t nterm_A = sc.add_nterm("A");
        size_t nterm_B = sc.add_nterm("B");
        sc.validate();

        name_table nt(sc);

        REQUIRE(nt.get_term_count() == 3);
        REQUIRE(nt.get_nterm_count() == 3);

        REQUIRE(std::string(nt.get_term_name(0)) == "$eof");
        REQUIRE(std::string(nt.get_term_name(term_a)) == "a");
        REQUIRE(std::string(nt.get_term_name(term_b)) == "b");

        REQUIRE(std::string(nt.get_nterm_name(0)) == "$root");
        REQUIRE(std::string(nt.get_nterm_name(nterm_A)) == "A");
        REQUIRE(std::string(nt.get_nterm_name(nterm_B)) == "B");
    }

    SECTION("out of range term")
    {
        symbol_collection sc;
        sc.validate();

        name_table nt(sc);

        REQUIRE_THROWS_MATCHES(
            nt.get_term_name(1),
            std::out_of_range,
            Message("Invalid terminal index")
        );
    }

    SECTION("out of range nterm")
    {
        symbol_collection sc;
        sc.validate();

        name_table nt(sc);

        REQUIRE_THROWS_MATCHES(
            nt.get_nterm_name(1),
            std::out_of_range,
            Message("Invalid non-terminal index")
        );
    }

    SECTION("long names")
    {
        symbol_collection sc;
        sc.add_term("very_long_terminal_name_1234567890");
        sc.add_nterm("very_long_non_terminal_name_abcdefghij");
        sc.validate();

        name_table nt(sc);

        REQUIRE(std::string(nt.get_term_name(1)) == "very_long_terminal_name_1234567890");
        REQUIRE(std::string(nt.get_nterm_name(1)) == "very_long_non_terminal_name_abcdefghij");
    }
}
