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
const size_t NUM_BODIES = 20;
const size_t NUM_STAR_POINTS = 4;

const vector_t MIN_BODY_POSITION = (vector_t){.x = 200, .y = 100};
const vector_t MAX_BODY_POSITION = (vector_t){.x = 800, .y = 400};
const double MIN_OUTER_RADIUS = 10;
const double MAX_OUTER_RADIUS = 30;
const double INNER_RAD_SCALING = 2;
const double MIN_MASS = 5;
const double MAX_MASS = 10;
const vector_t MIN_VELOCITY = (vector_t){.x = -10, .y = -10};
const vector_t MAX_VELOCITY = (vector_t){.x = 10, .y = 10};
// gravitational accelaration ($\vec{g}$)
const double G = 15;

typedef struct state {
  scene_t *scene;
} state_t;

list_t *create_star(size_t outer_radius, size_t inner_radius, size_t num_points,
                    size_t center_x, size_t center_y) {
  double curr_angle;
  double x_outer;
  double y_outer;
  double x_inner;
  double y_inner;
  list_t *vertices = list_init(num_points * 2, free);

  for (size_t i = 0; i < num_points; i++) {
    curr_angle = 2 * M_PI * i / num_points;
    // Creates the vertices of the outer vertices of the star
    x_outer = cos(curr_angle) * outer_radius + center_x;
    y_outer = sin(curr_angle) * outer_radius + center_y;
    vector_t *out_rad_vec_ptr = malloc(sizeof(vector_t));
    assert(out_rad_vec_ptr != NULL);
    out_rad_vec_ptr->x = x_outer;
    out_rad_vec_ptr->y = y_outer;
    list_add(vertices, out_rad_vec_ptr);

    // Creates the vertices of the inner vertices of the star
    x_inner = cos(curr_angle + M_PI / num_points) * inner_radius + center_x;
    y_inner = sin(curr_angle + M_PI / num_points) * inner_radius + center_y;
    vector_t *in_rad_vec_ptr = malloc(sizeof(vector_t));
    assert(in_rad_vec_ptr != NULL);
    in_rad_vec_ptr->x = x_inner;
    in_rad_vec_ptr->y = y_inner;
    list_add(vertices, in_rad_vec_ptr);
  }
  return vertices;
};

body_t *generate_body(double outer_radius, double inner_radius,
                      size_t num_points, vector_t init_star_center,
                      rgb_color_t color, vector_t init_velocity, double mass) {
  list_t *polygon = create_star(outer_radius, inner_radius, num_points,
                                init_star_center.x, init_star_center.y);
  body_t *my_body = body_init(polygon, mass, color);
  body_set_velocity(my_body, init_velocity);
  return my_body;
}

double rand_double(double low, double high) {
  return (float)rand() / (float)(RAND_MAX) * (high - low) + low;
}

body_t *generate_random_body() {
  double outer_radius = rand_double(MIN_OUTER_RADIUS, MAX_OUTER_RADIUS);
  double inner_radius = outer_radius / INNER_RAD_SCALING;
  size_t num_points = NUM_STAR_POINTS;
  vector_t init_star_center =
      (vector_t){.x = rand_double(MIN_BODY_POSITION.x, MAX_BODY_POSITION.x),
                 .y = rand_double(MIN_BODY_POSITION.y, MAX_BODY_POSITION.y)};

  rgb_color_t color = generate_random_color();
  vector_t init_velocity =
      (vector_t){rand_double(MIN_VELOCITY.x, MAX_VELOCITY.x),
                 rand_double(MIN_VELOCITY.y, MAX_VELOCITY.y)};
  double mass = rand_double(MIN_MASS, MAX_MASS);

  return generate_body(outer_radius, inner_radius, num_points, init_star_center,
                       color, init_velocity, mass);
}

state_t *emscripten_init() {
  // set up seed for rand
  size_t ltime = time(NULL);
  size_t time = (unsigned)ltime / 2;
  srand(time);

  sdl_init(VEC_ZERO, WINDOW_SIZE);
  state_t *init_state = malloc(sizeof(state_t));
  assert(init_state != NULL);

  init_state->scene = scene_init();
  for (size_t i = 0; i < NUM_BODIES; i++) {
    scene_add_body(init_state->scene, generate_random_body());
  }
  return init_state;
}

void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    for (size_t j = i + 1; j < scene_bodies(state->scene); j++) {
      create_newtonian_gravity(state->scene, G, scene_get_body(state->scene, i),
                               scene_get_body(state->scene, j));
    }
    body_tick(scene_get_body(state->scene, i), dt);
  }
  scene_tick(state->scene, dt);
  sdl_render_scene(state->scene);
  sdl_show();
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}
