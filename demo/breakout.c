#include "body.h"
#include "collision.h"
#include "color.h"
#include "forces.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "vector.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <assert.h>
#include <emscripten.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const vector_t WINDOW_SIZE = (vector_t){.x = 1000, .y = 500};

const size_t RECTANGLE_RESOLUTION = 4;

const size_t COLUMNS = 10;
const size_t ROWS = 3;
const double BRICK_MASS = INFINITY;
const double BRICK_HEIGHT = 20.0;

const vector_t PADDLE_START = (vector_t){.x = 500, .y = 50};
const double PADDLE_SPEED = 500;
const double PADDLE_MASS = INFINITY;
const rgb_color_t PADDLE_COLOR = {1.0, 0.1, 0.1};
const double PADDLE_WIDTH = 100;
const double PADDLE_HEIGHT = 20;

const size_t BALL_RESOLUTION = 420;
const double BALL_MASS = 1;
const rgb_color_t BALL_COLOR = {1.0, 0.1, 0.1};
const vector_t BALL_STARTING_VELOCITY = {300.0, 300.0};
const vector_t BALL_START = {.x = 500, .y = 75};
const size_t BALL_RADIUS = 5;

const double HEALTHBAR_WIDTH = 200;
const double HEALTHBAR_HEIGHT = 10;
const double HEALTHBAR_SPACING = 20;
const vector_t HEALTHBAR_POSITION = {70, 15};
const rgb_color_t HEALTHBAR_COLOR = {0, 1.0, 0};
const rgb_color_t HEALTHBAR_BG_COLOR = {0, 0, 0};
const size_t MAX_LIVES = 3;

const size_t PADDLE_ID = 1;
const size_t BRICK_ID = 2;
const size_t BALL_ID = 3;
const size_t WALL_ID = 4;
const size_t HEALTHBAR_ID = 5;
const size_t HEALTHBAR_BG_ID = 6;

const double STARTING_RED = 0.99;
const double STARTING_GREEN = 0.01;
const double STARTING_BLUE = 0.01;
const float STARTING_RED_MULTIPLIER = -1;
const float STARTING_GREEN_MULTIPLIER = 1.5;
const float STARTING_BLUE_MULTIPLIER = -1;

const double WALL_WIDTH = 20;
const rgb_color_t WALL_COLOR = {1.0, 0.0, 1.0};

const double ELASTICITY = 1.0;

typedef struct state {
  scene_t *scene;
  size_t remaining_lives;
} state_t;

list_t *generate_rectangle_shape(double rec_x, double rec_y, double rec_width,
                                 double rec_height) {
  list_t *rec_shape = list_init(RECTANGLE_RESOLUTION, free);

  vector_t *rec_point_vec = malloc(sizeof(vector_t));
  assert(rec_point_vec != NULL);
  rec_point_vec->x = (double)(rec_x + rec_width / 2);
  rec_point_vec->y = (double)(rec_y + rec_height / 2);
  list_add(rec_shape, rec_point_vec);

  rec_point_vec = malloc(sizeof(vector_t));
  assert(rec_point_vec != NULL);
  rec_point_vec->x = (double)(rec_x + rec_width / 2 * -1);
  rec_point_vec->y = (double)(rec_y + rec_height / 2);
  list_add(rec_shape, rec_point_vec);

  rec_point_vec = malloc(sizeof(vector_t));
  rec_point_vec->x = (double)(rec_x + rec_width / 2 * -1);
  rec_point_vec->y = (double)(rec_y + rec_height / 2 * -1);
  list_add(rec_shape, rec_point_vec);

  rec_point_vec = malloc(sizeof(vector_t));
  assert(rec_point_vec != NULL);
  rec_point_vec->x = (double)(rec_x + rec_width / 2);
  rec_point_vec->y = (double)(rec_y + rec_height / 2 * -1);
  list_add(rec_shape, rec_point_vec);

  return rec_shape;
}

body_t *generate_ball() {
  list_t *my_ball_shape = list_init(BALL_RESOLUTION, free);

  for (size_t point = 0; point < BALL_RESOLUTION; point++) {
    double angle = 2 * M_PI * point / BALL_RESOLUTION;

    vector_t *ball_point_vec = malloc(sizeof(vector_t));
    assert(ball_point_vec != NULL);

    ball_point_vec->x = BALL_START.x + BALL_RADIUS * cos(angle);
    ball_point_vec->y = BALL_START.y + BALL_RADIUS * sin(angle);

    list_add(my_ball_shape, ball_point_vec);
  }

  size_t *ball_ID = malloc(sizeof(size_t));
  assert(ball_ID != NULL);
  *ball_ID = BALL_ID;

  body_t *my_ball =
      body_init_with_info(my_ball_shape, BALL_MASS, BALL_COLOR, ball_ID, free);

  body_set_velocity(my_ball, BALL_STARTING_VELOCITY);

  return my_ball;
}

body_t *generate_brick(double brick_x, double brick_y, double brick_width,
                       double brick_height, rgb_color_t brick_color) {
  list_t *brick_shape =
      generate_rectangle_shape(brick_x, brick_y, brick_width, brick_height);

  size_t *brick_ID = malloc(sizeof(size_t));
  assert(brick_ID != NULL);
  *brick_ID = BRICK_ID;

  body_t *brick =
      body_init_with_info(brick_shape, BRICK_MASS, brick_color, brick_ID, free);
  return brick;
}

body_t *generate_paddle() {
  list_t *paddle_shape = generate_rectangle_shape(
      PADDLE_START.x, PADDLE_START.y, PADDLE_WIDTH, PADDLE_HEIGHT);

  size_t *paddle_ID = malloc(sizeof(size_t));
  assert(paddle_ID != NULL);
  *paddle_ID = PADDLE_ID;

  body_t *paddle = body_init_with_info(paddle_shape, BRICK_MASS, PADDLE_COLOR,
                                       paddle_ID, free);
  return paddle;
}

void generate_health_bar(state_t *state) {
  list_t *healthbar_bg_shape = generate_rectangle_shape(
      HEALTHBAR_POSITION.x +
          (MAX_LIVES - 1) / 2 *
              (HEALTHBAR_WIDTH / MAX_LIVES + HEALTHBAR_SPACING),
      HEALTHBAR_POSITION.y,
      HEALTHBAR_WIDTH + (MAX_LIVES + 2) * HEALTHBAR_SPACING, HEALTHBAR_HEIGHT);
  size_t *healthbar_bg_ID = malloc(sizeof(size_t));
  assert(healthbar_bg_ID != NULL);
  *healthbar_bg_ID = HEALTHBAR_BG_ID;
  body_t *healthbar_bg =
      body_init_with_info(healthbar_bg_shape, BRICK_MASS, HEALTHBAR_BG_COLOR,
                          healthbar_bg_ID, free);
  scene_add_body(state->scene, healthbar_bg);

  for (size_t i = 0; i < MAX_LIVES; i++) {
    list_t *health_brick_shape = generate_rectangle_shape(
        HEALTHBAR_POSITION.x +
            i * (HEALTHBAR_WIDTH / MAX_LIVES + HEALTHBAR_SPACING),
        HEALTHBAR_POSITION.y, HEALTHBAR_WIDTH / MAX_LIVES,
        HEALTHBAR_HEIGHT / MAX_LIVES);
    size_t *healthbar_ID = malloc(sizeof(size_t));
    assert(healthbar_ID != NULL);
    *healthbar_ID = HEALTHBAR_ID;

    body_t *healthbar_brick = body_init_with_info(
        health_brick_shape, BRICK_MASS, HEALTHBAR_COLOR, healthbar_ID, free);
    scene_add_body(state->scene, healthbar_brick);
  }
}

body_t *generate_wall(double wall_x, double wall_y) {
  list_t *wall_shape =
      generate_rectangle_shape(wall_x, wall_y, WALL_WIDTH, WINDOW_SIZE.y);

  size_t *wall_ID = malloc(sizeof(size_t));
  assert(wall_ID != NULL);
  *wall_ID = WALL_ID;

  body_t *wall =
      body_init_with_info(wall_shape, INFINITY, WALL_COLOR, wall_ID, free);
  return wall;
}

body_t *generate_roof() {
  list_t *roof_shape = generate_rectangle_shape(WINDOW_SIZE.x / 2,
                                                WINDOW_SIZE.y + WALL_WIDTH / 2,
                                                WINDOW_SIZE.x, WALL_WIDTH);

  size_t *roof_ID = malloc(sizeof(size_t));
  assert(roof_ID != NULL);
  *roof_ID = WALL_ID;

  body_t *roof =
      body_init_with_info(roof_shape, INFINITY, WALL_COLOR, roof_ID, free);
  return roof;
}

void player_handler(state_t *state, char key, key_event_type_t type,
                    double held_time) {
  body_t *player = scene_get_body(state->scene, 1);
  vector_t player_velocity = (vector_t){0.0, 0.0};

  if (type == KEY_RELEASED) {
    body_set_velocity(player, player_velocity);
  } else if (key == RIGHT_ARROW) {
    player_velocity.x = PADDLE_SPEED;
  } else if (key == LEFT_ARROW) {
    player_velocity.x = -PADDLE_SPEED;
  }
  body_set_velocity(player, player_velocity);
}

void ball_collision_checker(state_t *state) {
  body_t *ball = scene_get_body(state->scene, 0);

  for (size_t i = 1; i < scene_bodies(state->scene); i++) {
    body_t *body1 = scene_get_body(state->scene, i);
    if (*(size_t *)body_get_info(body1) == BRICK_ID) {
      create_destructive_physics_collision(state->scene, ELASTICITY, ball,
                                           body1);
    } else if (*(size_t *)body_get_info(body1) == HEALTHBAR_BG_ID ||
               *(size_t *)body_get_info(body1) == HEALTHBAR_ID) {
      continue;
    } else {
      create_physics_collision(state->scene, ELASTICITY, body1, ball);
    }
  }
}

void paddle_wall_blocker(state_t *state, double dt) {
  body_t *paddle = scene_get_body(state->scene, 1);
  if (body_get_velocity(paddle).x > 0 && body_get_velocity(paddle).x * dt +
                                                 body_get_centroid(paddle).x +
                                                 PADDLE_WIDTH / 2 >=
                                             WINDOW_SIZE.x) {
    body_set_velocity(paddle, (vector_t){0, 0});
    body_set_centroid(
        paddle, (vector_t){WINDOW_SIZE.x - PADDLE_WIDTH / 2, PADDLE_START.y});
  } else if (body_get_velocity(paddle).x < 0 &&
             body_get_velocity(paddle).x * dt + body_get_centroid(paddle).x -
                     PADDLE_WIDTH / 2 <=
                 0) {
    body_set_velocity(paddle, (vector_t){0, 0});
    body_set_centroid(paddle, (vector_t){PADDLE_WIDTH / 2, PADDLE_START.y});
  }
}

void reset_condition(state_t *state, double dt) {
  body_t *ball = scene_get_body(state->scene, 0);
  if (body_get_centroid(ball).y + BALL_RADIUS <= 0) {
    state->remaining_lives = state->remaining_lives - 1;
    for (size_t i = scene_bodies(state->scene) - 1; i > 0; i--) {
      body_t *body = scene_get_body(state->scene, i);
      if (*(size_t *)body_get_info(body) == HEALTHBAR_ID) {
        body_remove(body);
        break;
      }
    }
    body_t *ball = scene_get_body(state->scene, 0);
    body_set_centroid(ball, BALL_START);
    body_set_velocity(ball, BALL_STARTING_VELOCITY);
  }
  if (state->remaining_lives == 0) {
    printf("You lose :(\n");
    scene_tick(state->scene, dt);
    sdl_render_scene(state->scene);
    *state = *emscripten_init();
  }
  size_t count_bricks = 0;
  for (size_t i = scene_bodies(state->scene) - 1; i > 0; i--) {
    body_t *body = scene_get_body(state->scene, i);
    if (*(size_t *)body_get_info(body) == BRICK_ID) {
      count_bricks++;
    }
  }
  if (count_bricks == 0) {
    printf("You win! :)\n");
    scene_tick(state->scene, dt);
    sdl_render_scene(state->scene);
    emscripten_cancel_main_loop();
    emscripten_force_exit(0);
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
  init_state->remaining_lives = MAX_LIVES;

  scene_add_body(init_state->scene, generate_ball());
  scene_add_body(init_state->scene, generate_paddle());
  scene_add_body(
      init_state->scene,
      generate_wall(WINDOW_SIZE.x + WALL_WIDTH / 2, WINDOW_SIZE.y / 2));
  scene_add_body(init_state->scene,
                 generate_wall(-WALL_WIDTH / 2, WINDOW_SIZE.y / 2));
  scene_add_body(init_state->scene, generate_roof());

  // Create the bricks
  double brick_width = WINDOW_SIZE.x / (COLUMNS + 1);
  double brick_spacing = brick_width / (COLUMNS - 1);

  float red = STARTING_RED;
  float green = STARTING_GREEN;
  float blue = STARTING_BLUE;
  float red_multiplier = STARTING_RED_MULTIPLIER;
  float green_multiplier = STARTING_GREEN_MULTIPLIER;
  float blue_multiplier = STARTING_BLUE_MULTIPLIER;
  float color_add_factor = 0.99 / ((float)COLUMNS);

  for (size_t c = 0; c < COLUMNS; c++) {
    if (red + red_multiplier * color_add_factor >= 1.0 ||
        red + red_multiplier * color_add_factor <= 0.0) {
      red_multiplier *= -1.0;
    }
    if (green + green_multiplier * color_add_factor >= 1.0 ||
        green + green_multiplier * color_add_factor <= 0.0) {
      green_multiplier *= -1.0;
    }
    if (blue + blue_multiplier * color_add_factor >= 1.0 ||
        blue + blue_multiplier * color_add_factor <= 0.0) {
      blue_multiplier *= -1.0;
    }
    red += red_multiplier * color_add_factor;
    green += green_multiplier * color_add_factor;
    blue += blue_multiplier * color_add_factor;
    rgb_color_t brick_color = color_init(red, green, blue);

    double brick_x = brick_spacing * c + brick_width * c + brick_width / 2;

    for (size_t r = 0; r < ROWS; r++) {
      double brick_y = WINDOW_SIZE.y - brick_spacing * r - BRICK_HEIGHT * r -
                       BRICK_HEIGHT / 2;
      body_t *brick = generate_brick(brick_x, brick_y, brick_width,
                                     BRICK_HEIGHT, brick_color);
      scene_add_body(init_state->scene, brick);
    }
    generate_health_bar(init_state);
  }

  sdl_on_key(player_handler);
  ball_collision_checker(init_state);
  return init_state;
}

void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  paddle_wall_blocker(state, dt);
  reset_condition(state, dt);
  scene_tick(state->scene, dt);
  sdl_render_scene(state->scene);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}