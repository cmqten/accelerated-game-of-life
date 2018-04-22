/**
 * Various utility macros, functions, variables.
 */
#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <stdexcept>
#include <type_traits>

inline bool in_range(int val, int min, int max) 
{
    return val >= min && val <= max;
}

template <class T, class E>
void throw_not_equal(T a, T b, const std::string& message) 
{
    static_assert(std::is_base_of<std::exception, E>(), "template argument"
        " E must be of type std::exception");
    if (a != b) throw E(message);
}

template <class E>
void throw_false(bool a, const std::string& message) 
{
    static_assert(std::is_base_of<std::exception, E>(), "template argument"
        " E must be of type std::exception");
    if (!a) throw E(message);
}

template <class E>
void throw_true(bool a, const std::string& message) 
{
    throw_false<E>(!a, message);
}

template <class E>
void throw_non_zero(int a, const std::string& message)
{
    throw_false<E>(!a, message);
}

template <class E>
void throw_null(void* a, const std::string& message) 
{
    throw_false<E>(a, message);
}

#endif
