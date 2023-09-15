#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "forces.h"
#include "test_util.h"

list_t *make_shape()
{
  list_t *shape = list_init(4, free);
  vector_t *v = malloc(sizeof(*v));
  *v = (vector_t){-1, -1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){+1, -1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){+1, +1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){-1, +1};
  list_add(shape, v);
  return shape;
}

// Tests that a mass falling eventually reaches or is close to its terminal
// velocity v_terminal = M * G / gamma
void test_terminal_velocity()
{
  const double M = 10;
  const double GAMMA = 150;
  const double Y_0 = 1000;
  const double G = 20;
  const double DT = 1e-6;
  const int STEPS = 10000000;
  scene_t *scene = scene_init();
  body_t *mass = body_init(make_shape(), M, (rgb_color_t){0, 0, 0});
  body_set_centroid(mass, (vector_t){0, Y_0});
  scene_add_body(scene, mass);
  create_drag(scene, GAMMA, mass);
  for (int i = 0; i < STEPS; i++)
  {
    body_add_force(mass, (vector_t){0, -M * G});
    scene_tick(scene, DT);
  }
  assert(vec_isclose(body_get_velocity(mass), (vector_t){0, -(M * G) / GAMMA}));
  scene_free(scene);
}

// Tests that a mass sliding down inclined plane matches
// its velocity after a fixed time
void test_inclined_plane()
{
  const double M = 10;
  const double THETA = M_PI / 6;
  const double Y_0 = 1000;
  const double G = 20;
  const double DT = 1e-6;
  const int STEPS = 1000000;
  scene_t *scene = scene_init();
  body_t *mass = body_init(make_shape(), M, (rgb_color_t){0, 0, 0});
  body_set_centroid(mass, (vector_t){0, Y_0});
  scene_add_body(scene, mass);
  for (int i = 0; i < STEPS; i++)
  {
    body_add_force(mass, (vector_t){M * G * sin(THETA), 0}); // Gravity Parallel
    scene_tick(scene, DT);
  }
  assert(vec_isclose(body_get_velocity(mass),
                     (vector_t){STEPS * DT * G * sin(THETA), 0}));
  scene_free(scene);
}

// Tests that different masses experience same acceleration
void test_feather_experiment()
{
  const double M_FEATHER = 10;
  const double M_BOWLING_BALL = 1000;
  const double Y_0 = 1000;
  const double G = 20;
  const double DT = 1e-6;
  const int STEPS = 1000000;
  scene_t *scene = scene_init();
  body_t *feather = body_init(make_shape(), M_FEATHER, (rgb_color_t){0, 0, 0});
  body_t *bowling_ball =
      body_init(make_shape(), M_BOWLING_BALL, (rgb_color_t){0, 0, 0});
  body_set_centroid(feather, (vector_t){0, Y_0});
  body_set_centroid(bowling_ball, (vector_t){0, Y_0});
  scene_add_body(scene, feather);
  scene_add_body(scene, bowling_ball);
  for (int i = 0; i < STEPS; i++)
  {
    body_add_force(feather, (vector_t){0, -M_FEATHER * G});
    body_add_force(bowling_ball, (vector_t){0, -M_BOWLING_BALL * G});
    assert(vec_isclose(body_get_centroid(feather),
                       body_get_centroid(bowling_ball)));
    scene_tick(scene, DT);
  }
  scene_free(scene);
}

int main(int argc, char *argv[])
{
  // Run all tests if there are no command-line arguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests)
  {
    read_testname(argv[1], testname, sizeof(testname));
  }

  DO_TEST(test_terminal_velocity)
  DO_TEST(test_inclined_plane)
  DO_TEST(test_feather_experiment)

  puts("student_test PASS");
}