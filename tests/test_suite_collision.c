#include "collision.h"
#include "polygon.h"
#include "test_util.h"
#include "vec_list.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

// Make square at for the given x and y coordinates
list_t *make_square(double x1, double x2, double y1, double y2) {
  list_t *sq = list_init(4, free);
  vector_t *v = malloc(sizeof(*v));
  *v = (vector_t){x1, y1};
  list_add(sq, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){x1, y2};
  list_add(sq, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){x2, y2};
  list_add(sq, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){x2, y1};
  list_add(sq, v);
  return sq;
}

void test_colliding_shapes() {
  list_t *sq1 = make_square(-1, 1, -1, 1);
  list_t *sq2 = make_square(0.5, 2.5, -1, 1);
  assert(find_collision(sq1, sq2).collided);
  polygon_translate(sq2, (vector_t){0, 2});
  assert(find_collision(sq1, sq2).collided);
  polygon_rotate(sq2, M_PI_4, polygon_centroid(sq1));
  polygon_translate(sq2, (vector_t){0, -2});
  assert(find_collision(sq1, sq2).collided);
  list_free(sq1);
  list_free(sq2);
}

void test_noncolliding_shapes() {
  list_t *sq1 = make_square(-1, 1, -1, 1);
  list_t *sq2 = make_square(5, 7, -1, 1);
  assert(find_collision(sq1, sq2).collided == false);
  polygon_rotate(sq2, M_PI_4, polygon_centroid(sq1));
  assert(find_collision(sq1, sq2).collided == false);
  polygon_translate(sq2, (vector_t){0, 100});
  assert(find_collision(sq1, sq2).collided == false);
  list_free(sq1);
  list_free(sq2);
}

int main(int argc, char *argv[]) {
  // Run all tests if there are no command-line arguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }

  DO_TEST(test_colliding_shapes)
  DO_TEST(test_noncolliding_shapes)

  puts("collision_test PASS");
}
