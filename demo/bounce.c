#include "assert.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "state.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// color and window constants
const rgb_color_t STAR_COLOR = {0.0, 1.0, 0.0};
const vector_t WINDOW_SIZE = (vector_t){.x = 1000, .y = 500};

const double OUTER_RADIUS = 50;
const double INNER_RADIUS = OUTER_RADIUS / M_E;
const size_t NUM_POINTS = 5;
const vector_t INIT_STAR_CENTER = (vector_t){.x = 500, .y = 250};
const vector_t INIT_VELOCITY = (vector_t){.x = -150, .y = 120};
const double ANGULAR_VELOCITY = M_PI / 2;
const double MASS = 6.9;

typedef struct state {
  scene_t *scene;
} state_t;

/**
 * Creates a star-shaped polygon
 * Returns a polygon stored as a vec_list_t*
 */
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
    out_rad_vec_ptr->x = x_outer;
    out_rad_vec_ptr->y = y_outer;
    list_add(vertices, out_rad_vec_ptr);

    // Creates the vertices of the inner vertices of the star
    x_inner = cos(curr_angle + M_PI / num_points) * inner_radius + center_x;
    y_inner = sin(curr_angle + M_PI / num_points) * inner_radius + center_y;
    vector_t *in_rad_vec_ptr = malloc(sizeof(vector_t));
    in_rad_vec_ptr->x = x_inner;
    in_rad_vec_ptr->y = y_inner;
    list_add(vertices, in_rad_vec_ptr);
  }
  return vertices;
};

state_t *emscripten_init() {
  sdl_init(VEC_ZERO, WINDOW_SIZE);
  state_t *init_state = malloc(sizeof(state_t));
  assert(init_state != NULL);

  init_state->scene = scene_init();

  body_t *star = body_init(create_star(OUTER_RADIUS, INNER_RADIUS, NUM_POINTS,
                                       INIT_STAR_CENTER.x, INIT_STAR_CENTER.y),
                           MASS, STAR_COLOR);
  scene_add_body(init_state->scene, star);
  body_set_velocity(scene_get_body(init_state->scene, 0), INIT_VELOCITY);
  body_set_ang_velocity(scene_get_body(init_state->scene, 0), ANGULAR_VELOCITY);
  return init_state;
}

/**
 * Checks if the star touches one of the boundaries.
 * Returns which boundary it touches if it does.
 *
 * @param state the state of the polygon
 *
 * Returns pointer to 2 boolean array;
 * position 0: True if we need to invert x component of velocity
 * position 1: True if we need to invert y component of velocity
 */
bool *star_on_window(state_t *state, double dt) {
  body_t *star = scene_get_body(state->scene, 0);
  double dx = body_get_velocity(star).x * dt;
  double dy = body_get_velocity(star).y * dt;

  bool *ans = malloc(sizeof(bool) * 2);
  ans[0] = false;
  ans[1] = false;

  size_t number_of_vertices = list_size(body_get_shape(star));
  for (size_t v = 0; v < number_of_vertices; v++) {
    vector_t *my_vector = list_get(body_get_shape(star), v);

    if (WINDOW_SIZE.x < my_vector->x + dx) {
      ans[0] = true;
    }
    if (WINDOW_SIZE.y < my_vector->y + dy) {
      ans[1] = true;
    }
    if (my_vector->x + dx < 0) {
      ans[0] = true;
    }
    if (my_vector->y + dy < 0) {
      ans[1] = true;
    }
  }
  return ans;
}

/**
 * Updates the state of the polygon for given amount of time.
 * Translates and rotates the polygon, and also computes elastic collisions.
 * Note: mutates the original state.
 *
 * @param state the old state of the polygon
 * @param dt the time interval for which the movement is computed
 */
void update_scene(state_t *state, vector_t velocity, double dt) {
  body_t *star = scene_get_body(state->scene, 0);
  vector_t vel = body_get_velocity(star);
  bool *reflects = star_on_window(state, dt);
  // Elastic Collision
  if (reflects[0]) {
    vector_t new_vel = {-vel.x, vel.y};
    body_set_velocity(star, new_vel);
  }
  if (reflects[1]) {
    vector_t new_vel = {vel.x, -vel.y};
    body_set_velocity(star, new_vel);
  }

  body_update(star, dt);
};

void emscripten_main(state_t *state) {
  sdl_clear();
  body_t *star = scene_get_body(state->scene, 0);
  double dt = time_since_last_tick();
  update_scene(state, body_get_velocity(star), dt);
  sdl_draw_polygon(body_get_shape(star), STAR_COLOR);
  sdl_show();
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}
