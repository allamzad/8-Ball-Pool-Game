#include "body.h"
#include "collision.h"
#include "color.h"
#include "forces.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const vector_t WINDOW_SIZE = (vector_t){.x = 1000, .y = 500};

const size_t ENEMY_RESOLUTION = 30;
const double ENEMY_RADIUS = 30;
const double ENEMY_MASS = 6.9;
const rgb_color_t ENEMY_COLOR = {0.41, 0.41, 0.41};
const double ENEMY_ANGLE = M_PI / 6;
const size_t ENEMY_ROWS = 3;
const size_t NUM_ENEMY_IN_ROW = 8;
const double ENEMY_SPACING = 8;
const vector_t SPACE_WINDOW_AND_ENEMY = (vector_t){.x = 50, .y = 10};
const vector_t ENEMY_VELOCITY = (vector_t){.x = 4, .y = 0};
const float ENEMY_PROJECTILE_PROB = 0.005;

const size_t LASER_RESOLUTION = 4;
const double LASER_HEIGHT = 10;
const double LASER_WIDTH = 5;
const double LASER_SPEED = 10;
const double LASER_MASS = 0.1;

const size_t PLAYER_RESOLUTION = 69;
const double PLAYER_HEIGHT = 20;
const double PLAYER_WIDTH = 40;
const double PLAYER_SPEED = 10;
const double PLAYER_MASS = 0.1;
const rgb_color_t PLAYER_COLOR = {0.3, 1.0, 0.3};

const size_t ENEMY_ID = 1;
const size_t PLAYER_ID = 2;
const size_t ENEMY_LASER_ID = 3;
const size_t PLAYER_LASER_ID = 4;

const char SPACE_BAR_CHAR = '_';

typedef struct state {
  scene_t *scene;
} state_t;

/** Generates an enemy on the screen at the given position */
body_t *generate_enemy(double x_pos, double y_pos) {
  list_t *enemy_shape = list_init(ENEMY_RESOLUTION, free);
  double dalpha = (M_PI - 2 * ENEMY_ANGLE) / (ENEMY_RESOLUTION - 2);

  for (size_t point = 1; point < ENEMY_RESOLUTION; point++) {
    double angle = dalpha * (point - 1) + ENEMY_ANGLE;
    vector_t *enemy_point_vec = malloc(sizeof(vector_t));
    assert(enemy_point_vec != NULL);
    enemy_point_vec->x = x_pos + ENEMY_RADIUS * cos(angle);
    enemy_point_vec->y = y_pos + ENEMY_RADIUS * sin(angle);
    list_add(enemy_shape, enemy_point_vec);
  }

  vector_t *enemy_point_vec = malloc(sizeof(vector_t));
  assert(enemy_point_vec != NULL);
  enemy_point_vec->x = x_pos;
  enemy_point_vec->y = y_pos;
  list_add(enemy_shape, enemy_point_vec);
  size_t *my_enemy_id = malloc(sizeof(size_t));
  assert(my_enemy_id != NULL);
  *my_enemy_id = ENEMY_ID;

  body_t *enemy = body_init_with_info(enemy_shape, ENEMY_MASS, ENEMY_COLOR,
                                      my_enemy_id, free);
  body_set_velocity(enemy, ENEMY_VELOCITY);
  return enemy;
}

/** Generates a laser for the enemy or player depending on shooter's ID */
body_t *generate_laser(body_t *shooter) {
  list_t *laser_shape = list_init(LASER_RESOLUTION, free);
  double laser_x = body_get_centroid(shooter).x;
  double laser_y = body_get_centroid(shooter).y;

  vector_t *laser_point_vec = malloc(sizeof(vector_t));
  assert(laser_point_vec != NULL);
  laser_point_vec->x = (double)(laser_x + LASER_WIDTH / 2);
  laser_point_vec->y = (double)(laser_y + LASER_HEIGHT / 2);
  list_add(laser_shape, laser_point_vec);

  laser_point_vec = malloc(sizeof(vector_t));
  assert(laser_point_vec != NULL);
  laser_point_vec->x = (double)(laser_x + LASER_WIDTH / 2 * -1);
  laser_point_vec->y = (double)(laser_y + LASER_HEIGHT / 2);
  list_add(laser_shape, laser_point_vec);

  laser_point_vec = malloc(sizeof(vector_t));
  laser_point_vec->x = (double)(laser_x + LASER_WIDTH / 2 * -1);
  laser_point_vec->y = (double)(laser_y + LASER_HEIGHT / 2 * -1);
  list_add(laser_shape, laser_point_vec);

  laser_point_vec = malloc(sizeof(vector_t));
  assert(laser_point_vec != NULL);
  laser_point_vec->x = (double)(laser_x + LASER_WIDTH / 2);
  laser_point_vec->y = (double)(laser_y + LASER_HEIGHT / 2 * -1);
  list_add(laser_shape, laser_point_vec);

  size_t *laser_ID = malloc(sizeof(size_t));
  assert(laser_ID != NULL);
  vector_t laser_velocity = (vector_t){0, -LASER_SPEED};

  if (*(size_t *)body_get_info(shooter) != PLAYER_ID) {
    *laser_ID = ENEMY_LASER_ID;
  } else {
    *laser_ID = PLAYER_LASER_ID;
    laser_velocity = (vector_t){0, LASER_SPEED};
  }
  rgb_color_t laser_color = body_get_color(shooter);

  body_t *laser =
      body_init_with_info(laser_shape, LASER_MASS, laser_color, laser_ID, free);
  body_set_velocity(laser, laser_velocity);
  return laser;
}

/** Generates a player at the bottom of the window */
body_t *generate_player() {
  list_t *player_shape = list_init(PLAYER_RESOLUTION, free);

  double player_x = WINDOW_SIZE.x / 2;
  double player_y = PLAYER_HEIGHT;

  for (size_t i = 0; i < PLAYER_RESOLUTION; i++) {
    double angle = 2 * M_PI * i / PLAYER_RESOLUTION;
    vector_t *v = malloc(sizeof(vector_t));
    assert(v != NULL);
    *v = (vector_t){player_x + PLAYER_WIDTH * cos(angle),
                    player_y + PLAYER_HEIGHT * sin(angle)};
    list_add(player_shape, v);
  }
  size_t *my_player_id = malloc(sizeof(size_t));
  assert(my_player_id != NULL);
  *my_player_id = PLAYER_ID;
  body_t *player = body_init_with_info(player_shape, PLAYER_MASS, PLAYER_COLOR,
                                       my_player_id, free);
  return player;
}

/** Handles key presses of the player */
void player_handler(state_t *state, char key, key_event_type_t type,
                    double held_time) {
  body_t *player = scene_get_body(state->scene, 0);
  vector_t player_velocity = (vector_t){0.0, 0.0};

  if (type == KEY_RELEASED) {
    body_set_velocity(player, player_velocity);
  } else if (key == RIGHT_ARROW) {
    player_velocity.x = PLAYER_SPEED;
  } else if (key == LEFT_ARROW) {
    player_velocity.x = -PLAYER_SPEED;
  } else if (key == SPACE_BAR_CHAR) {
    body_t *my_laser = generate_laser(player);
    scene_add_body(state->scene, my_laser);
  }
  body_set_velocity(player, player_velocity);
}

/** Handles player wrapping around the window */
void player_wrapper(state_t *state) {
  body_t *player = scene_get_body(state->scene, 0);
  vector_t player_centroid = body_get_centroid(player);

  if (player_centroid.x - PLAYER_WIDTH / 2 > WINDOW_SIZE.x) {
    body_set_centroid(player, (vector_t){-PLAYER_WIDTH / 2, player_centroid.y});
  } else if (player_centroid.x + PLAYER_WIDTH / 2 < 0) {
    body_set_centroid(player, (vector_t){WINDOW_SIZE.x + PLAYER_WIDTH / 2,
                                         player_centroid.y});
  }
}

/** Handles enemy wrapping around the window and down the screen */
void enemy_wrapper(state_t *state) {
  for (size_t i = 1; i < scene_bodies(state->scene); i++) {
    body_t *body = scene_get_body(state->scene, i);
    if (*(size_t *)body_get_info(body) == ENEMY_ID) {
      vector_t enemy_centroid = body_get_centroid(body);
      if (enemy_centroid.x + ENEMY_RADIUS > WINDOW_SIZE.x) {
        body_set_centroid(
            body, (vector_t){WINDOW_SIZE.x - ENEMY_RADIUS,
                             enemy_centroid.y -
                                 ENEMY_ROWS * (ENEMY_SPACING + ENEMY_RADIUS)});
        body_set_velocity(body, vec_multiply(-1, body_get_velocity(body)));
      } else if (enemy_centroid.x - ENEMY_RADIUS < 0) {
        body_set_centroid(
            body, (vector_t){ENEMY_RADIUS,
                             enemy_centroid.y -
                                 ENEMY_ROWS * (ENEMY_SPACING + ENEMY_RADIUS)});
        body_set_velocity(body, vec_multiply(-1, body_get_velocity(body)));
      }
    }
  }
}

/** Enemy fires at random interval based on ENEMY_PROJECTILE_PROB */
void enemy_random_fire(state_t *state) {
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    body_t *body = scene_get_body(state->scene, i);
    if (*(size_t *)body_get_info(body) == ENEMY_ID) {
      bool enemy_shoot =
          (float)rand() / (float)RAND_MAX < ENEMY_PROJECTILE_PROB;
      if (enemy_shoot) {
        body_t *my_laser = generate_laser(body);
        scene_add_body(state->scene, my_laser);
      }
    }
  }
}

/** Returns whether a player laser hit the enemy */
bool laser_hit_enemy(body_t *body1, body_t *body2) {
  size_t body1_info = *(size_t *)body_get_info(body1);
  size_t body2_info = *(size_t *)body_get_info(body2);
  bool laser_enemy_collision =
      (body1_info == PLAYER_LASER_ID && body2_info == ENEMY_ID) ||
      (body2_info == PLAYER_LASER_ID && body1_info == ENEMY_ID);
  return laser_enemy_collision;
}

/** Returns whether an enemy laser hit the player */
bool laser_hit_player(body_t *body1, body_t *body2) {
  size_t body1_info = *(size_t *)body_get_info(body1);
  size_t body2_info = *(size_t *)body_get_info(body2);
  bool laser_enemy_collision =
      (body1_info == ENEMY_LASER_ID && body2_info == PLAYER_ID) ||
      (body2_info == ENEMY_LASER_ID && body1_info == PLAYER_ID);
  return laser_enemy_collision;
}

/** Checks if there is a valid collision between two bodies */
void collision_checker(state_t *state) {
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    body_t *body1 = scene_get_body(state->scene, i);
    for (size_t j = i + 1; j < scene_bodies(state->scene); j++) {
      body_t *body2 = scene_get_body(state->scene, j);
      double threshold = ENEMY_RADIUS;
      if (PLAYER_HEIGHT / 2 > threshold) {
        threshold = PLAYER_HEIGHT / 2;
      }
      // Checks if bodies are close, and laser hitting either enemy/player
      if (vec_magnitude(vec_subtract(body_get_centroid(body1),
                                     body_get_centroid(body2))) < threshold &&
          (laser_hit_enemy(body1, body2) || laser_hit_player(body1, body2))) {
        create_destructive_collision(state->scene, body1, body2);
      }
    }
  }
}

/** Ends the game when winning/losing conditions are met */
void end_conditions(state_t *state) {
  size_t count_enemies = 0;
  size_t count_player = 0;
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    body_t *body = scene_get_body(state->scene, i);
    if (*(size_t *)body_get_info(body) == ENEMY_ID) {
      // Checks if space invaders below ground
      if (body_get_centroid(body).y - ENEMY_RADIUS < 0) {
        exit(0);
      }
      count_enemies++;
    } else if (*(size_t *)body_get_info(body) == PLAYER_ID) {
      count_player++;
    }
  }
  // Checks if all enemies or player are removed
  if (count_enemies == 0 || count_player == 0) {
    exit(0);
  }
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

  scene_add_body(init_state->scene, generate_player());

  // Creates the initial row of enemies at the top, with appropriate spacing
  for (size_t i = 0; i < ENEMY_ROWS; i++) {
    double enemy_y_pos =
        (WINDOW_SIZE.y - SPACE_WINDOW_AND_ENEMY.y - ENEMY_RADIUS) -
        i * (ENEMY_SPACING + ENEMY_RADIUS);
    for (size_t j = 0; j < NUM_ENEMY_IN_ROW; j++) {
      double enemy_x_pos = SPACE_WINDOW_AND_ENEMY.x +
                           j * (WINDOW_SIZE.x - 2 * SPACE_WINDOW_AND_ENEMY.x) /
                               (NUM_ENEMY_IN_ROW - 1);
      scene_add_body(init_state->scene,
                     generate_enemy(enemy_x_pos, enemy_y_pos));
    }
  }
  sdl_on_key(player_handler);

  return init_state;
}

void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  player_wrapper(state);
  enemy_wrapper(state);
  collision_checker(state);
  enemy_random_fire(state);
  end_conditions(state);
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    scene_tick(state->scene, dt);
  }
  sdl_render_scene(state->scene);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}
