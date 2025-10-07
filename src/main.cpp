#include <parse_table_generator.h>
#include <index_subset.h>
#include <grammar_error.h>

#include <iostream>

int main()
{
    //ptg::parse_table_generator ptg;
    //ptg::index_subset is(100);
    ptg::grammar_error err(ptg::grammar_error::code::ok);
    auto msg = err.what();
    std::cout << "Hello, parse_table_generator!" << std::endl;
    std::cout << msg << std::endl;
    return 0;
}
