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

static inline void swap_ptr(void** a, void** b)
{
    void* temp = *a;
    *a = *b;
    *b = temp;
}

#endif
