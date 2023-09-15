#include "color.h"
#include "test_util.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

rgb_color_t make_color() {
  rgb_color_t color = color_init(1.0, 0.5, 0.25);
  return color;
}

void test_color_get() {
  rgb_color_t color = make_color();
  assert(color_get_r(color) == 1.0);
  assert(color_get_g(color) == 0.5);
  assert(color_get_b(color) == 0.25);
}

int main(int argc, char *argv[]) {
  // Run all tests? True if there are no command-line arguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }

  DO_TEST(test_color_get)

  puts("color_test PASS");
}