#pragma once

#include <ostream>
#include <sstream>
#include <string_view>
#include <type_traits>

namespace ptg
{

class list_printer
{
private:
    std::string_view left_delim_;
    std::string_view separator_;
    std::string_view right_delim_;
    mutable bool printed_any_;
    mutable bool first_;
    bool print_empty_;

    template <typename T>
    const T& to_printable(const T& t) const
    {
        return t;
    }

    const char* to_printable(bool b) const
    {
        return b ? "true" : "false";
    }


    template <typename T>
    void print_item_helper(std::ostream& os, T&& item) const
    {
        std::stringstream temp;
        temp << to_printable(std::forward<T>(item));
        std::string str = temp.str();
        if (!str.empty() || print_empty_)
        {
            if (printed_any_ || (print_empty_ && !first_))
            {
                os << separator_;
            }
            os << str;
            printed_any_ = true;
        }
    }

public:
    list_printer(std::string_view left_delim = "",
                 std::string_view separator = " ",
                 std::string_view right_delim = "",
                 bool print_empty = false)
        : left_delim_(left_delim), separator_(separator), right_delim_(right_delim), printed_any_(false), first_(true), print_empty_(print_empty)
    {
    }

    template <typename Arg>
    std::string print_single(Arg item) const
    {
        std::stringstream ss;
        
        ss << left_delim_;
        printed_any_ = false;
        first_ = true;
        print_item_helper(ss, std::forward<Arg>(item));
        ss << right_delim_;
        
        return ss.str();
    }
    
    template <typename... Args>
    std::string print_list(Args&&... items) const
    {
        std::stringstream ss;
        
        ss << left_delim_;
        printed_any_ = false;
        first_ = true;
        (print_item_helper(ss, std::forward<Args>(items)), ...);
        ss << right_delim_;
        
        return ss.str();
    }
    
    template <typename Container, typename F>
    std::string print_container(const Container& container, F to_string_f) const
    {
        return print_container_from_to(container, to_string_f, 0, container.size());
    }
    
    template <typename Container, typename F>
    std::string print_container_from_to(const Container& container, F to_string_f, size_t from, size_t to) const
    {
        std::stringstream ss;
        
        ss << left_delim_;
        printed_any_ = false;
        first_ = true;
   
        for (size_t i = from; i < container.size() && i < to; ++i)
        {
            print_item_helper(ss, to_string_f(container[i]));
        }
        
        ss << right_delim_;
        
        return ss.str();
    }
};

} // namespace ptg
