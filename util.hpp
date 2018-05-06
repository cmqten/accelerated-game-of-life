/**
 * Various utility macros, functions, variables.
 */
#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <type_traits>

#define swap_ptr(t, a, b) \
{ \
    a = (t)((uintptr_t)a ^ (uintptr_t)b); \
    b = (t)((uintptr_t)a ^ (uintptr_t)b); \
    a = (t)((uintptr_t)a ^ (uintptr_t)b); \
}

class my_timer
{
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> _start;
    bool _started;

public:
    inline my_timer() : _started(false) {};

    inline void start() 
    {
        if (!_started) {
            _start = std::chrono::high_resolution_clock::now();
            _started = true;
        } 
    };

    inline double stop()
    {
        if (_started) {
            _started = false;
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> seconds = end - _start;
            return seconds.count();
        }
        return -1.0;
    };
};

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
