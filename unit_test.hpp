/**
 * Quick and easy implementation because I don't want to add a unit testing 
 * library.
 */
#ifndef __UNIT_TEST_HPP__
#define __UNIT_TEST_HPP__

#include <cstring>
#include <iostream>
#include <string>

template <typename T>
void assert_equal(T a, T b, std::string desc)
{
    std::cout << "Test : " << desc << " - ";
    if (a == b) std::cout << "Passed" << std::endl << std::endl;
    else std::cout << "Failed" << std::endl << std::endl;
}

template <typename T>
void assert_not_equal(T a, T b, std::string desc)
{
    std::cout << "Test : " << desc << " - ";
    if (a != b) std::cout << "Passed" << std::endl << std::endl;
    else std::cout << "Failed" << std::endl << std::endl;
}

static void assert_buf_equal(void* a, void* b, size_t n, std::string desc)
{
    std::cout << "Test : " << desc << " - ";
    if (!memcmp(a, b, n)) std::cout << "Passed" << std::endl << std::endl;
    else std::cout << "Failed" << std::endl << std::endl;
}

static void assert_buf_not_equal(void* a, void* b, size_t n, std::string desc)
{
    std::cout << "Test : " << desc << " - ";
    if (memcmp(a, b, n)) std::cout << "Passed" << std::endl << std::endl;
    else std::cout << "Failed" << std::endl << std::endl;
}

static void assert_true(bool a, std::string desc)
{
    std::cout << "Test : " << desc << " - ";
    if (a) std::cout << "Passed" << std::endl << std::endl;
    else std::cout << "Failed" << std::endl << std::endl;
}

static void assert_false(bool a, std::string desc)
{
    std::cout << "Test : " << desc << " - ";
    if (!a) std::cout << "Passed" << std::endl << std::endl;
    else std::cout << "Failed" << std::endl << std::endl;
}

#endif
