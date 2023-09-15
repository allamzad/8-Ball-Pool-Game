#include "body.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "state.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const vector_t WINDOW_SIZE = (vector_t){.x = 1000, .y = 500};

const size_t NUM_PELLETS = 10;
const size_t PELLET_RESOLUTION =
    69;                         // number of points that make up pellet shape
const double PELLET_RADIUS = 5; // side length of pellet
const double PELLET_MASS = 1;
const rgb_color_t PELLET_COLOR = {1, 1, 0.2};

const size_t PACMAN_RESOLUTION = 69;
const double PACMAN_RADIUS = 50;
const double PACMAN_MASS = 6.9;
const rgb_color_t PACMAN_COLOR = {1, 1, 0.2};
const vector_t PACMAN_START = (vector_t){.x = 500, .y = 250};
const double PACMAN_MOUTH_ANGLE = M_PI / 8;

const double BASE_SPEED = 75.0;
const double MY_ACCELERATION = 300.0;

typedef struct state {
  scene_t *scene;
} state_t;

body_t *generate_pellet() {
  list_t *my_pellet_shape = list_init(PELLET_RESOLUTION, free);

  double pellet_x = rand() % (int)WINDOW_SIZE.x;
  double pellet_y = rand() % (int)WINDOW_SIZE.y;

  for (size_t point = 0; point < PELLET_RESOLUTION; point++) {
    double angle = 2 * M_PI * point / PELLET_RESOLUTION;

    vector_t *pellet_point_vec = malloc(sizeof(vector_t));
    assert(pellet_point_vec != NULL);
    pellet_point_vec->x = pellet_x + PELLET_RADIUS * cos(angle);
    pellet_point_vec->y = pellet_y + PELLET_RADIUS * sin(angle);

    list_add(my_pellet_shape, pellet_point_vec);
  }

  body_t *my_pellet = body_init(my_pellet_shape, PELLET_MASS, PELLET_COLOR);

  return my_pellet;
}

body_t *generate_pacman() {
  list_t *my_pacman_shape = list_init(PACMAN_RESOLUTION, free);
  double dalpha = 2 * (M_PI - PACMAN_MOUTH_ANGLE) / (PACMAN_RESOLUTION - 2);

  for (size_t point = 1; point < PACMAN_RESOLUTION; point++) {
    double angle = dalpha * (point - 1) + PACMAN_MOUTH_ANGLE;

    vector_t *pacman_point_vec = malloc(sizeof(vector_t));
    assert(pacman_point_vec != NULL);
    pacman_point_vec->x = PACMAN_START.x + PACMAN_RADIUS * cos(angle);
    pacman_point_vec->y = PACMAN_START.y + PACMAN_RADIUS * sin(angle);
    list_add(my_pacman_shape, pacman_point_vec);
  }

  vector_t *pacman_point_vec = malloc(sizeof(vector_t));
  assert(pacman_point_vec != NULL);
  pacman_point_vec->x = PACMAN_START.x;
  pacman_point_vec->y = PACMAN_START.y;
  list_add(my_pacman_shape, pacman_point_vec);

  body_t *my_pacman = body_init(my_pacman_shape, PACMAN_MASS, PACMAN_COLOR);

  return my_pacman;
}

void pacman_key_handler(state_t *state, char key, key_event_type_t type,
                        double held_time) {
  body_t *pacman = scene_get_body(state->scene, 0);
  vector_t new_velocity = {0.0, 0.0};
  double my_speed = BASE_SPEED;

  if (type == KEY_PRESSED) {
    my_speed += MY_ACCELERATION * held_time;
  }
  if (type == KEY_RELEASED) {
    body_set_velocity(pacman, new_velocity);
    return;
  }

  if (key == UP_ARROW) {
    new_velocity.x = 0;
    new_velocity.y = my_speed;
    body_set_rotation(pacman, M_PI / 2);
  }
  if (key == DOWN_ARROW) {
    new_velocity.x = 0;
    new_velocity.y = -my_speed;
    body_set_rotation(pacman, -M_PI / 2);
  }
  if (key == RIGHT_ARROW) {
    new_velocity.x = my_speed;
    new_velocity.y = 0;
    body_set_rotation(pacman, 0);
  }
  if (key == LEFT_ARROW) {
    new_velocity.x = -my_speed;
    new_velocity.y = 0;
    body_set_rotation(pacman, -M_PI);
  }
  body_set_velocity(pacman, new_velocity);
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
  sdl_on_key(pacman_key_handler);
  scene_add_body(init_state->scene, generate_pacman());

  for (size_t i = 0; i < NUM_PELLETS; i++) {
    scene_add_body(init_state->scene, generate_pellet());
  }
  return init_state;
}

void pellet_pacman_collision(state_t *state) {
  vector_t pacman_centroid = body_get_centroid(scene_get_body(state->scene, 0));
  for (size_t i = 1; i < scene_bodies(state->scene); i++) {
    vector_t pellet_centroid =
        body_get_centroid(scene_get_body(state->scene, i));
    if (vec_magnitude(vec_subtract(pellet_centroid, pacman_centroid)) <=
        PACMAN_RADIUS - PELLET_RADIUS) {
      scene_remove_body(state->scene, i);
      break; // danger of over iteration...
    }
  }
}

void pacman_wrapper(state_t *state) {
  body_t *pacman = scene_get_body(state->scene, 0);
  vector_t pacman_centroid = body_get_centroid(scene_get_body(state->scene, 0));
  if (pacman_centroid.x - PACMAN_RADIUS > WINDOW_SIZE.x) {
    body_set_centroid(pacman, (vector_t){-PACMAN_RADIUS, pacman_centroid.y});
  }
  if (pacman_centroid.x + PACMAN_RADIUS < 0) {
    body_set_centroid(
        pacman, (vector_t){WINDOW_SIZE.x + PACMAN_RADIUS, pacman_centroid.y});
  }
  if (pacman_centroid.y - PACMAN_RADIUS > WINDOW_SIZE.y) {
    body_set_centroid(pacman, (vector_t){pacman_centroid.x, -PACMAN_RADIUS});
  }
  if (pacman_centroid.y + PACMAN_RADIUS < 0) {
    body_set_centroid(
        pacman, (vector_t){pacman_centroid.x, WINDOW_SIZE.y + PACMAN_RADIUS});
  }
}

void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  pacman_wrapper(state);
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    body_update(scene_get_body(state->scene, i), dt);
  }
  pellet_pacman_collision(state);
  scene_tick(state->scene, dt);
  sdl_render_scene(state->scene);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}
