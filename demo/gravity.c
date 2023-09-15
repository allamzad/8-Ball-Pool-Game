#include "assert.h"
#include "body.h"
#include "list.h"
#include "math.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "vec_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const vector_t WINDOW_SIZE = (vector_t){.x = 1000, .y = 500};
const double GND_LEVEL = 0.0;
const size_t MAX_ELASTICITY = 100; // percent
const size_t MIN_ELASTICITY = 70;  // percent

const double OUTER_RADIUS = 50;
const double INNER_RADIUS = OUTER_RADIUS / 3;
const size_t NUM_POINTS = 2;
const vector_t INIT_STAR_CENTER = (vector_t){.x = 500, .y = 250};
const rgb_color_t COLOR = {0.0, 1.0, 0.0};
const vector_t INIT_VELOCITY = (vector_t){.x = 50, .y = 0};
// gravitational accelaration ($\vec{g}$)
const vector_t G = (vector_t){.x = 0, .y = -100};
const double ANG_VELOCITY = 0.5;

const size_t INITIAL_LIST_SIZE = 1;
const size_t INITIAL_POLY_SIZE = 2;
const size_t MAX_SIDES = 10;
const double GAP = 30;
const double MASS = 6.9;

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

body_t *generate_star_body(double outer_radius, double inner_radius,
                           size_t num_points, vector_t init_star_center,
                           rgb_color_t color, vector_t init_velocity,
                           double mass) {
  list_t *polygon = create_star(outer_radius, inner_radius, num_points,
                                init_star_center.x, init_star_center.y);
  body_t *my_body = body_init(polygon, mass, color);
  body_set_velocity(my_body, init_velocity);
  body_set_ang_velocity(my_body, ANG_VELOCITY);
  return my_body;
}

body_t *generate_random_star_body(vector_t init_star_center) {

  double outer_radius = OUTER_RADIUS;
  double inner_radius = INNER_RADIUS;

  size_t num_points = rand() % MAX_SIDES + INITIAL_POLY_SIZE;

  float color_r = (float)rand() / (float)(RAND_MAX);
  float color_g = (float)rand() / (float)(RAND_MAX);
  float color_b = (float)rand() / (float)(RAND_MAX);
  rgb_color_t color = color_init(color_r, color_g, color_b);

  vector_t init_velocity = INIT_VELOCITY;
  double mass = MASS;

  return generate_star_body(outer_radius, inner_radius, num_points,
                            init_star_center, color, init_velocity, mass);
}

//-----------------------------------------------------------------------------
typedef struct state {
  scene_t *scene;
} state_t;

state_t *emscripten_init() {
  // set up seed for rand
  size_t ltime = time(NULL);
  size_t time = (unsigned)ltime / 2;
  srand(time);

  sdl_init(VEC_ZERO, WINDOW_SIZE);
  state_t *init_state = malloc(sizeof(state_t));
  assert(init_state != NULL);

  init_state->scene = scene_init();

  for (size_t i = 1; i <= INITIAL_LIST_SIZE; i++) {
    vector_t init_star_center =
        (vector_t){.x = OUTER_RADIUS * ((INITIAL_LIST_SIZE - i) * 2 + 1) +
                        GAP * (INITIAL_LIST_SIZE - i),
                   .y = WINDOW_SIZE.y - OUTER_RADIUS};

    scene_add_body(init_state->scene,
                   generate_random_star_body(init_star_center));
  }
  return init_state;
}

void update_scene(state_t *state, double dt) {
  // check for out of bounds behavior. Since the stars are all moving at same
  // speed, we will only check first index.

  if (scene_bodies(state->scene) > 0 &&
      body_get_centroid(scene_get_body(state->scene, 0)).x - OUTER_RADIUS >
          WINDOW_SIZE.x) {
    scene_remove_body(state->scene, 0);
  }

  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    body_t *my_body = scene_get_body(state->scene, i);

    if (body_get_centroid(my_body).y - OUTER_RADIUS +
            body_get_velocity(my_body).y * dt <
        GND_LEVEL) {
      double elasticity =
          rand() % (MAX_ELASTICITY - MIN_ELASTICITY + 1) + MIN_ELASTICITY;
      elasticity = elasticity / 100; // transform from percentage to number
      vector_t new_vel = {body_get_velocity(my_body).x,
                          -elasticity * (body_get_velocity(my_body).y)};
      body_set_velocity(my_body, new_vel);
    }
    body_set_velocity(my_body,
                      vec_add(body_get_velocity(my_body), vec_multiply(dt, G)));
    body_update(my_body, dt);
  }
}

void emscripten_main(state_t *state) {
  sdl_clear();

  double dt = time_since_last_tick();

  // check if space for new polygon
  if (scene_bodies(state->scene) > 0 &&
      body_get_centroid(
          scene_get_body(state->scene, scene_bodies(state->scene) - 1))
                  .x -
              OUTER_RADIUS >
          2 * OUTER_RADIUS + GAP) {
    vector_t init_star_center =
        (vector_t){.x = OUTER_RADIUS, .y = WINDOW_SIZE.y - OUTER_RADIUS};
    scene_add_body(state->scene, generate_random_star_body(init_star_center));
  }

  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    body_t *my_body = scene_get_body(state->scene, i);
    body_draw(my_body);
  }

  update_scene(state, dt);
  sdl_show();
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}
