#include "body.h"
#include "color.h"
#include "forces.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const vector_t WINDOW_SIZE = (vector_t){.x = 1000, .y = 500};
const size_t NUM_BALLS = 60;
const size_t BALL_RESOLUTION = 20;
const double BALL_MASS = 1.0;
const double BALL_SCALE =
    2.0; // Helps with allocation of radius to fit all the balls

const rgb_color_t WHITE = {1, 1, 1};

const double SPRING_CONSTANT = 12;
const double SPRING_DAMPENING = 0.95;
const double DRAG_GAMMA = 0.9;

typedef struct state {
  scene_t *scene;
} state_t;

body_t *generate_ball(double ball_x, double ball_y, rgb_color_t ball_color,
                      double ball_radius) {
  list_t *my_ball_shape = list_init(BALL_RESOLUTION, free);

  for (size_t point = 0; point < BALL_RESOLUTION; point++) {
    double angle = 2 * M_PI * point / BALL_RESOLUTION;

    vector_t *ball_point_vec = malloc(sizeof(vector_t));
    assert(ball_point_vec != NULL);

    ball_point_vec->x = ball_x + ball_radius * cos(angle);
    ball_point_vec->y = ball_y + ball_radius * sin(angle);

    list_add(my_ball_shape, ball_point_vec);
  }

  body_t *my_ball = body_init(my_ball_shape, BALL_MASS, ball_color);

  return my_ball;
}

state_t *emscripten_init() {
  sdl_init(VEC_ZERO, WINDOW_SIZE);
  state_t *init_state = malloc(sizeof(state_t));
  assert(init_state != NULL);

  scene_t *init_scene = scene_init();

  double ball_radius = WINDOW_SIZE.x / (double)NUM_BALLS / BALL_SCALE;

  // Assigns a color gradient. These are not magic numbers! This is hard coded
  // such that we get a gradient of green and blue to just blue, to red and
  // blue, to just red, to green and red, to just green, and then back to green
  // and blue.
  float red = 0.01;
  float green = 0.5;
  float blue = 0.5;
  float color_add_factor = 0.99 / ((float)NUM_BALLS / 3.0);

  for (int i = 0; i < NUM_BALLS; i++) {
    double ball_x = i * WINDOW_SIZE.x / (double)NUM_BALLS + ball_radius;

    double ball_y = WINDOW_SIZE.y / 2.0 +
                    pow(ball_x - WINDOW_SIZE.x / 2, 2) /
                        (pow(WINDOW_SIZE.x / 2, 2) / WINDOW_SIZE.y * 2);

    if (i < NUM_BALLS / 6 || i >= NUM_BALLS * 5 / 6) {
      green -= color_add_factor;
      blue += color_add_factor;
    } else if (i < NUM_BALLS * 3 / 6) {
      blue -= color_add_factor;
      red += color_add_factor;
    } else {
      red -= color_add_factor;
      green += color_add_factor;
    }

    rgb_color_t my_color = color_init(red, green, blue);

    scene_add_body(init_scene, generate_ball(ball_x, WINDOW_SIZE.y - ball_y,
                                             WHITE, ball_radius));
    scene_add_body(init_scene,
                   generate_ball(ball_x, ball_y, my_color, ball_radius));

    create_spring(init_scene,
                  SPRING_CONSTANT * pow(SPRING_DAMPENING, (double)i),
                  scene_get_body(init_scene, 2 * i + 1),
                  scene_get_body(init_scene, 2 * i));
    create_drag(init_scene, DRAG_GAMMA, scene_get_body(init_scene, 2 * i));
    create_drag(init_scene, DRAG_GAMMA, scene_get_body(init_scene, 2 * i + 1));
  }
  init_state->scene = init_scene;
  return init_state;
}

void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  scene_tick(state->scene, dt);
  sdl_render_scene(state->scene);
  sdl_show();
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}
