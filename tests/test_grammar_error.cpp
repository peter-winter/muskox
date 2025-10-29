#include <format>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <grammar_error.h>

using Catch::Matchers::Message;

using namespace muskox;

TEST_CASE("grammar_error create", "[grammar_error]")
{
    using gec = grammar_error::code;

    SECTION("ok without symbol")
    {
        grammar_error err(gec::ok);
        REQUIRE(err.what() == std::string("Ok"));
    }

    SECTION("ok with symbol ignored")
    {
        grammar_error err(gec::ok, "ignored");
        REQUIRE(err.what() == std::string("Ok"));
    }

    SECTION("symbol_exists")
    {
        grammar_error err(gec::symbol_exists, "abc");
        REQUIRE(err.what() == std::string("Symbol 'abc' already exists"));
    }

    SECTION("lside_not_exists")
    {
        grammar_error err(gec::lside_not_exists, "def");
        REQUIRE(err.what() == std::string("Left side 'def' does not exist"));
    }

    SECTION("lside_term")
    {
        grammar_error err(gec::lside_term, "ghi");
        REQUIRE(err.what() == std::string("Left side 'ghi' is a terminal"));
    }

    SECTION("rside_not_exist")
    {
        grammar_error err(gec::rside_not_exist, "jkl");
        REQUIRE(err.what() == std::string("Right side symbol 'jkl' does not exist"));
    }

    SECTION("error without symbol")
    {
        REQUIRE_THROWS_AS((grammar_error(gec::symbol_exists)), std::format_error);
    }
}
