/**
 * @file list_printer.h
 * @brief Utility class for printing lists and containers with customizable delimiters.
 *
 * This header provides a flexible class for formatting and printing lists, single items,
 * or containers to strings. It supports custom delimiters, separators, and optional
 * printing of empty items. Specialized handling for booleans and generic types is included.
 *
 * Useful for debugging output, such as printing symbol lists or parse trees in a readable format.
 *
 * Part of the larger MuskOx project.
 */

#pragma once

#include <ostream>
#include <sstream>
#include <string_view>
#include <utility>

namespace muskox
{

/**
 * @class list_printer
 * @brief Printer for lists and containers.
 */
class list_printer
{
private:
    std::string_view left_delim_; /// Left delimiter.
    std::string_view separator_; /// Separator.
    std::string_view right_delim_; /// Right delimiter.
    mutable bool printed_any_; /// Mutable flag if any printed.
    mutable bool first_; /// Mutable flag for first item.
    bool print_empty_; /// Flag to print empty items.

    /**
     * @brief Converts item to printable.
     *
     * @tparam T Item type.
     * @param t The item.
     * @return Reference to t.
     */
    template <typename T>
    const T& to_printable(const T& t) const
    {
        return t;
    }

    /**
     * @brief Specialization for bool.
     *
     * @param b The bool.
     * @return "true" or "false".
     */
    const char* to_printable(bool b) const
    {
        return b ? "true" : "false";
    }

    /**
     * @brief Helper to print an item.
     *
     * @tparam T Item type.
     * @param os Output stream.
     * @param item The item.
     */
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
    /**
     * @brief Constructor.
     *
     * @param left_delim Left delimiter.
     * @param separator Separator.
     * @param right_delim Right delimiter.
     * @param print_empty Print empty flag.
     */
    list_printer(std::string_view left_delim = "",
                 std::string_view separator = " ",
                 std::string_view right_delim = "",
                 bool print_empty = false)
        : left_delim_(left_delim), separator_(separator), right_delim_(right_delim), printed_any_(false), first_(true), print_empty_(print_empty)
    {
    }

    /**
     * @brief Prints a single item.
     *
     * @tparam Arg Item type.
     * @param item The item.
     * @return String representation.
     */
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
    
    /**
     * @brief Prints a list of items.
     *
     * @tparam Args Item types.
     * @param items The items.
     * @return String representation.
     */
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
    
    /**
     * @brief Prints a container.
     *
     * @tparam Container Container type.
     * @tparam F To-string function type.
     * @param container The container.
     * @param to_string_f Conversion function.
     * @return String representation.
     */
    template <typename Container, typename F>
    std::string print_container(const Container& container, F to_string_f) const
    {
        return print_container_from_to(container, to_string_f, 0, container.size());
    }
    
    /**
     * @brief Prints a range of a container.
     *
     * @tparam Container Container type.
     * @tparam F To-string function type.
     * @param container The container.
     * @param to_string_f Conversion function.
     * @param from Start index.
     * @param to End index.
     * @return String representation.
     */
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

} // namespace muskox
