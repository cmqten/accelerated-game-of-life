/**
 * util.hpp
 * 
 * Various utility macros, functions, variables.
 * 
 * Author: Carl Marquez
 * Created on: April 21, 2018
 */
#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <chrono>
#include <cstdint>

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
            std::chrono::duration<double, std::milli> ms = end - _start;
            return ms.count();
        }
        return -1.0;
    };
};

static inline bool is_power_of_2(int val) {
    return __builtin_popcount(val) == 1;
}

static inline int nearest_le_pow_2(int val) 
{
    val |= val >> 1;
    val |= val >> 2;
    val |= val >> 4;
    val |= val >> 8;
    val |= val >> 16;
    val += 1;
    return val >> 1;
}

static inline void swap_ptr(void** a, void** b)
{
    void* temp = *a;
    *a = *b;
    *b = temp;
}

#endif
