/**
 * Quick and easy implementation because I don't want to add a unit testing 
 * library.
 */
#include <cstring>
#include "unit_test.hpp"

void assert_buf_equal(void* a, void* b, size_t n, std::string desc)
{
    std::cout << "Test : " << desc << " - ";
    if (!memcmp(a, b, n)) std::cout << "Passed" << std::endl << std::endl;
    else std::cout << "Failed" << std::endl << std::endl;
}

void assert_buf_not_equal(void* a, void* b, size_t n, std::string desc)
{
    std::cout << "Test : " << desc << " - ";
    if (memcmp(a, b, n)) std::cout << "Passed" << std::endl << std::endl;
    else std::cout << "Failed" << std::endl << std::endl;
}

void assert_true(bool a, std::string desc)
{
    std::cout << "Test : " << desc << " - ";
    if (a) std::cout << "Passed" << std::endl << std::endl;
    else std::cout << "Failed" << std::endl << std::endl;
}

void assert_false(bool a, std::string desc)
{
    std::cout << "Test : " << desc << " - ";
    if (!a) std::cout << "Passed" << std::endl << std::endl;
    else std::cout << "Failed" << std::endl << std::endl;
}