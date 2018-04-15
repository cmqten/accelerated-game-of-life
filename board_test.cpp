/**
 * Unit tests for board.cpp. Please do not use the filename __boardtesttemp.pbm
 * as it will be used for testing load and save.
 */
#include <cstdlib>
#include "board.hpp"
#include "unit_test.hpp"

int main(int argc, char** argv)
{
    char buf1[] = {'0', '1', '1', '0'};
    char buf2[] = {0, 1, 1, 0};
    char buf3[] = {'0', '1', '1', '0'};
    int w = 2;
    int h = 2;
    int s = w * h;
    std::string f = "__boardtesttemp.pbm";

    // ascii_to_int tests
    std::cout << "===== ascii_to_int =====" << std::endl;
    assert_false(ascii_to_int(nullptr, w, h), "null buffer");
    assert_false(ascii_to_int(buf3, MIN_WIDTH - 1, h), "width < min width");
    assert_false(ascii_to_int(buf3, MAX_WIDTH + 1, h), "width > max_width");
    assert_false(ascii_to_int(buf3, w, MIN_HEIGHT - 1), "height < min height");
    assert_false(ascii_to_int(buf3, w, MAX_HEIGHT + 1), "height > max height");
    assert_true(ascii_to_int(buf3, w, h), "ascii_to_int successful");
    assert_buf_equal(buf3, buf2, s, "ascii_to_int correct buffer result");
    
    // int_to_ascii tests
    std::cout << "===== int_to_ascii =====" << std::endl;
    assert_false(int_to_ascii(nullptr, w, h), "null buffer");
    assert_false(int_to_ascii(buf3, MIN_WIDTH - 1, h), "width < min width");
    assert_false(int_to_ascii(buf3, MAX_WIDTH + 1, h), "width > max_width");
    assert_false(int_to_ascii(buf3, w, MIN_HEIGHT - 1), "height < min height");
    assert_false(int_to_ascii(buf3, w, MAX_HEIGHT + 1), "height > max height");
    assert_true(int_to_ascii(buf3, w, h), "int_to_ascii successful");
    assert_buf_equal(buf3, buf1, s, "int_to_ascii correct buffer result");

    // save_board tests
    std::cout << "===== save_board =====" << std::endl;
    assert_false(save_board(nullptr, w, h, f), "null buffer");
    assert_false(save_board(buf3, MIN_WIDTH - 1, h, f), "width < min width");
    assert_false(save_board(buf3, MAX_WIDTH + 1, h, f), "width > max_width");
    assert_false(save_board(buf3, w, MIN_HEIGHT - 1, f), "height < min height");
    assert_false(save_board(buf3, w, MAX_HEIGHT + 1, f), "height > max height");
    assert_true(save_board(buf3, w, h, f), "save_board successful");

    // load_board tests
    int wl = -1;
    int hl = -1;
    char* new_buf;

    std::cout << "===== load_board =====" << std::endl;
    assert_equal<char*>(nullptr, load_board(&wl, &hl, "board.cpp"), 
                        "not a pbm file");
    assert_equal<int>(-1, wl, "width did not change after failed load");
    assert_equal<int>(-1, hl, "height did not change after failed load");

    new_buf = load_board(&wl, &hl, f);
    assert_not_equal<char*>(nullptr, new_buf, "successful load");
    assert_buf_equal(new_buf, buf3, s, "board correctly loaded to buffer");
    assert_equal<int>(w, wl, "width correctly loaded");
    assert_equal<int>(h, hl, "height correctly loaded");
    delete[] new_buf;
    
    std::string rm_cmd = "rm -f " + f;
    system(rm_cmd.c_str());

    assert_equal<char*>(nullptr, load_board(nullptr, nullptr, f), 
                        "file doesn't exist");

    return 0;
}