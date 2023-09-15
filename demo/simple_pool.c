#include "forces.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "vector.h"
#include "pool_table.h"
#include "pool_menu.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <assert.h>
#include <emscripten.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include "ai.h"

const vector_t WINDOW_SIZE = (vector_t){.x = 1000, .y = 500};
const double ELASTICITY = 0.8;

// Noncolliding objects do not have an ID
extern const size_t MENU_BACKGROUND_ID;
extern const size_t MENU_BUTTON_2P_ID;
extern const size_t MENU_BUTTON_EASY_AI_ID;
extern const size_t MENU_BUTTON_HARD_AI_ID;

// Ball constants
extern const size_t BALL_RADIUS;
const size_t MIN_BALL_STICK_SEPARATION = 15;
const double CUEBALL_INIT_VELOCITY = 500;
const vector_t BALL_VEL_NEAR_ZERO = {1, 1};

extern const vector_t BUTTON_DIMENSION_SIZE;

typedef struct gamestate
{
  size_t num_solid;
  size_t num_striped;
  bool eightball_present;
  bool cueball_present;
} gamestate_t;

typedef struct state
{
  scene_t *scene;
  bool *pool_stick_on_scene;
  bool *menu_on_scene;
  size_t *gamemode;       // 0 is 2 player, 1 is easy AI, 2 is hard AI
  size_t *current_player; // either 0 (solid) or 1 (striped)
  gamestate_t *gamestate;
  size_t stop_cooldown;
} state_t;

gamestate_t track_game_state(state_t *state)
{
  return (gamestate_t){solid_count(state->scene), striped_count(state->scene),
                       cueball_in_play(state->scene), eightball_in_play(state->scene)};
}

void stop_balls(state_t *state)
{
  for (size_t i = 0; i < scene_bodies(state->scene); i++)
  {
    body_t *my_body = scene_get_body(state->scene, i);
    if (is_ball(my_body))
    {
      body_set_velocity(my_body, VEC_ZERO);
    }
  }
}

void update_turn(state_t *state, gamestate_t new_gamestate)
{
  gamestate_t curr_gamestate = *state->gamestate;
  size_t curr_player = *state->current_player;
  bool solid_change = false;
  bool striped_change = false;
  // Turn must change if cueball pocketed
  if (!new_gamestate.cueball_present)
  {
    if (curr_player == 0)
    {
      *state->current_player = 1;
    }
    else
    {
      *state->current_player = 0;
    }
  }
  else
  {
    if (curr_gamestate.num_solid != new_gamestate.num_solid)
    {
      solid_change = true;
    }
    if (curr_gamestate.num_striped != new_gamestate.num_striped)
    {
      striped_change = true;
    }
    // Turn must change if solid player doesn't pocket any solid balls
    if ((!solid_change) && (curr_player == 0))
    {
      *state->current_player = 1;
    }
    // Turn must change if striped player doesn't pocket any striped balls
    if ((!striped_change) && (curr_player == 1))
    {
      *state->current_player = 0;
    }
  }
}

void my_mouse_handler(state_t *state, mouse_event_type_t type, vector_t clicked_point)
{
  double separation = vec_magnitude(vec_subtract(body_get_centroid(get_cueball_body(state->scene)), clicked_point));
  if (type == MOUSE_CLICKED)
  {
    printf("hello there! mouse info:\n");
    printf("x: %f\n", clicked_point.x);
    printf("y: %f\n", clicked_point.y);
    if (*state->pool_stick_on_scene == true && state->menu_on_scene == false)
    {
      body_remove(get_poolstick_body(state->scene));
      *state->pool_stick_on_scene = false;
    }
    if (separation > MIN_BALL_STICK_SEPARATION && state->menu_on_scene == false)
    {
      generate_poolstick(state->scene, clicked_point, body_get_centroid(get_cueball_body(state->scene)));
      *state->pool_stick_on_scene = true;
    }
    printf("mouse clikced!\n");
  }
  if (type == MOUSE_RELEASED && *state->pool_stick_on_scene == true)
  {
    body_t *cue = get_cueball_body(state->scene);
    vector_t dir = vec_unit(vec_subtract(body_get_centroid(cue), clicked_point));
    dir.y = -dir.y; // because screen y axis is inverted from scene y axis...
    // THIS SHOULD BE CHANGED TO SMTH CLEANER!
    body_add_impulse(cue, vec_multiply(body_get_mass(cue) * CUEBALL_INIT_VELOCITY, dir));
    body_remove(get_poolstick_body(state->scene));
    *state->pool_stick_on_scene = false;
    printf("Direction: %lf %lf\n", dir.x, -dir.y);
    printf("mouse released!\n");
  }
  if (type == MOUSE_DRAGGED)
  {
    if (*state->pool_stick_on_scene == true)
    {
      body_remove(get_poolstick_body(state->scene));
      *state->pool_stick_on_scene = false;
    }
    if (separation > MIN_BALL_STICK_SEPARATION)
    {
      generate_poolstick(state->scene, clicked_point, body_get_centroid(get_cueball_body(state->scene)));
      *state->pool_stick_on_scene = true;
    }
    printf("drag!\n");
  }
  if (type == MOUSE_MOVED)
  {
    printf("mouse moved!\n");
  }
  printf("%u\n", type);
}

state_t *emscripten_init()
{
  // set up seed for rand

  sdl_init(VEC_ZERO, WINDOW_SIZE);
  state_t *init_state = malloc(sizeof(state_t));
  assert(init_state != NULL);

  // sdl_on_mouse(my_mouse_handler);
  init_state->stop_cooldown = 0;
  init_state->scene = scene_init();
  init_state->pool_stick_on_scene = malloc(sizeof(bool));
  *init_state->pool_stick_on_scene = false;
  init_state->menu_on_scene = malloc(sizeof(bool));
  *init_state->menu_on_scene = false;
  init_state->gamemode = malloc(sizeof(size_t));
  init_state->gamestate = malloc(sizeof(gamestate_t));
  init_state->current_player = malloc(sizeof(size_t));
  *init_state->current_player = 0;
  generate_pool_table(init_state->scene, false);
  *init_state->gamestate = track_game_state(init_state);
  // init_state->player_one_turn = true;
  // init_state->player_one_is_solid = true;
  // generate_pool_graphics(init_state->scene);
  vector_t aux = body_get_centroid(get_cueball_body(init_state->scene));
  printf("CUE BALL AT: %lf %lf\n", aux.x, aux.y);
  return init_state;
}

void emscripten_main(state_t *state)
{
  sdl_clear();
  // is_game_over(state);
  double dt = time_since_last_tick();
  if (state->stop_cooldown == 0 && balls_stopped(state->scene))
  {
    //ai_easy_make_move(state->scene, true); //debugging
    ai_medium_make_move(state->scene, true);
    *state->gamestate = track_game_state(state);
    printf("HIT!\n");
    state->stop_cooldown = 20;
  }
  scene_tick(state->scene, dt);
  if (state->stop_cooldown == 0)
    state->stop_cooldown = 11;
  state->stop_cooldown--;
  // if (balls_stopped(state->scene))
  // {
  //   stop_balls(state);
  // }
  sdl_render_scene(state->scene);
}

void emscripten_free(state_t *state)
{
  scene_free(state->scene);
  free(state);
}