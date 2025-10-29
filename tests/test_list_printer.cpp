#include <catch2/catch_test_macros.hpp>

#include <list_printer.h>

#include <sstream>
#include <string>

using namespace muskox;

TEST_CASE("list_printer basic operations", "[list_printer]")
{
    SECTION("print_list empty call with defaults")
    {
        list_printer lp;
        std::string s = lp.print_list();
        REQUIRE(s == "");
    }

    SECTION("print_list empty call with delimiters")
    {
        list_printer lp("[", ", ", "]");
        std::string s = lp.print_list();
        REQUIRE(s == "[]");
    }

    SECTION("print_list single non-empty string")
    {
        list_printer lp;
        std::string s = lp.print_list("hello");
        REQUIRE(s == "hello");
    }

    SECTION("print_list single empty string")
    {
        list_printer lp;
        std::string s = lp.print_list("");
        REQUIRE(s == "");
    }

    SECTION("print_list single non-empty with delimiters")
    {
        list_printer lp("[", ", ", "]");
        std::string s = lp.print_list("hello");
        REQUIRE(s == "[hello]");
    }

    SECTION("print_list single empty with delimiters")
    {
        list_printer lp("[", ", ", "]");
        std::string s = lp.print_list("");
        REQUIRE(s == "[]");
    }

    SECTION("print_list multiple non-empty strings")
    {
        list_printer lp;
        std::string s = lp.print_list("a", "b", "c");
        REQUIRE(s == "a b c");
    }

    SECTION("print_list multiple with empties")
    {
        list_printer lp;
        std::string s = lp.print_list("a", "", "b", "", "c", "");
        REQUIRE(s == "a b c");
    }

    SECTION("print_list all empties")
    {
        list_printer lp;
        std::string s = lp.print_list("", "", "");
        REQUIRE(s == "");
    }

    SECTION("print_list multiple with delimiters")
    {
        list_printer lp("(", " | ", ")");
        std::string s = lp.print_list("one", "two", "three");
        REQUIRE(s == "(one | two | three)");
    }

    SECTION("print_list multiple with empties and delimiters")
    {
        list_printer lp("(", " | ", ")");
        std::string s = lp.print_list("one", "", "two", "", "three", "");
        REQUIRE(s == "(one | two | three)");
    }

    SECTION("print_list all empties with delimiters")
    {
        list_printer lp("(", " | ", ")");
        std::string s = lp.print_list("", "", "");
        REQUIRE(s == "()");
    }

    SECTION("print_list custom separator no delimiters")
    {
        list_printer lp("", ", ", "");
        std::string s = lp.print_list("apple", "banana", "cherry");
        REQUIRE(s == "apple, banana, cherry");
    }

    SECTION("print_list leading and trailing empties")
    {
        list_printer lp("[", " ", "]");
        std::string s = lp.print_list("", "", "a", "b", "", "", "c", "", "");
        REQUIRE(s == "[a b c]");
    }

    SECTION("print_list consecutive empties in middle")
    {
        list_printer lp;
        std::string s = lp.print_list("x", "", "", "y", "", "z");
        REQUIRE(s == "x y z");
    }

    SECTION("print_list single numeric int")
    {
        list_printer lp;
        std::string s = lp.print_list(42);
        REQUIRE(s == "42");
    }

    SECTION("print_list single numeric double")
    {
        list_printer lp;
        std::string s = lp.print_list(3.14);
        REQUIRE(s == "3.14");
    }

    SECTION("print_list mixed strings and numerics")
    {
        list_printer lp;
        std::string s = lp.print_list("hello", 42, "world", 3.14);
        REQUIRE(s == "hello 42 world 3.14");
    }

    SECTION("print_list mixed with empties and numerics")
    {
        list_printer lp;
        std::string s = lp.print_list("a", "", 1, "", "b", 2.0, "");
        REQUIRE(s == "a 1 b 2");
    }

    SECTION("print_list numerics with delimiters")
    {
        list_printer lp("[", ", ", "]");
        std::string s = lp.print_list(1, 2, 3.5);
        REQUIRE(s == "[1, 2, 3.5]");
    }

    SECTION("print_list zero numeric (not empty)")
    {
        list_printer lp;
        std::string s = lp.print_list(0, 0.0);
        REQUIRE(s == "0 0");
    }

    SECTION("print_list char type")
    {
        list_printer lp;
        std::string s = lp.print_list('a', 'b', 'c');
        REQUIRE(s == "a b c");
    }

    SECTION("print_list bool type")
    {
        list_printer lp;
        std::string s = lp.print_list(true, false);
        REQUIRE(s == "true false");
    }

    SECTION("print_list mixed types with delimiters")
    {
        list_printer lp("{", "; ", "}");
        std::string s = lp.print_list("key:", 42, ", active:", true);
        REQUIRE(s == "{key:; 42; , active:; true}");
    }

    SECTION("composed array definition")
    {
        list_printer inner_lp("{", ", ", "}");
        std::string inner = inner_lp.print_list(1, 2, 3, 4);
        list_printer outer_lp("", " ", ";");
        std::string outer = outer_lp.print_list("const", "int", "arr[]", "=", inner);
        REQUIRE(outer == "const int arr[] = {1, 2, 3, 4};");
    }

    SECTION("nested list composition")
    {
        list_printer inner_lp("[", ", ", "]");
        std::string inner1 = inner_lp.print_list(1, 2, 3);
        std::string inner2 = inner_lp.print_list(4, 5, 6);
        list_printer outer_lp("[", ", ", "]");
        std::string outer = outer_lp.print_list(inner1, inner2);
        REQUIRE(outer == "[[1, 2, 3], [4, 5, 6]]");
    }

    SECTION("print_list single empty string with print_empty true")
    {
        list_printer lp("", ", ", "", true);
        std::string s = lp.print_list("");
        REQUIRE(s == "");
    }

    SECTION("print_list single empty with delimiters print_empty true")
    {
        list_printer lp("[", ", ", "]", true);
        std::string s = lp.print_list("");
        REQUIRE(s == "[]");
    }

    SECTION("print_list multiple with empties print_empty true")
    {
        list_printer lp("", ",", "", true);
        std::string s = lp.print_list("a", "", "b", "", "c");
        REQUIRE(s == "a,,b,,c");
    }

    SECTION("print_list all empties print_empty true")
    {
        list_printer lp("", ",", "", true);
        std::string s = lp.print_list("", "", "");
        REQUIRE(s == ",,");
    }

    SECTION("print_list multiple with empties and delimiters print_empty true")
    {
        list_printer lp("(", " | ", ")", true);
        std::string s = lp.print_list("one", "", "two", "", "three");
        REQUIRE(s == "(one |  | two |  | three)");
    }

    SECTION("print_list all empties with delimiters print_empty true")
    {
        list_printer lp("(", " | ", ")", true);
        std::string s = lp.print_list("", "", "");
        REQUIRE(s == "( |  | )");
    }

    SECTION("print_list mixed with empties and numerics print_empty true")
    {
        list_printer lp("", ",", "", true);
        std::string s = lp.print_list("a", "", 1, "", "b", 2.0);
        REQUIRE(s == "a,,1,,b,2");
    }
}
