#include <grammar_error.h>

#include <iostream>

int main()
{
    using namespace muskox;
    
    grammar_error err(grammar_error::code::ok);
    auto msg = err.what();
    std::cout << "Hello, muskox!" << std::endl;
    std::cout << msg << std::endl;
    return 0;
}   
