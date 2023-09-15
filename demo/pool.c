#include "ai.h"
#include "angle.h"
#include "body.h"
#include "collision.h"
#include "color.h"
#include "forces.h"
#include "graphics.h"
#include "ids.h"
#include "list.h"
#include "polygon.h"
#include "pool_menu.h"
#include "pool_table.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "shape_utility.h"
#include "vector.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <assert.h>
#include <emscripten.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

const vector_t WINDOW_SIZE = (vector_t){.x = 1000, .y = 500};
const double ELASTICITY = 0.8;

// Ball constants
extern const size_t BALL_RADIUS;
const size_t MIN_BALL_STICK_SEPARATION = 15;
const double CUEBALL_INIT_VELOCITY = 500;
const vector_t BALL_VEL_NEAR_ZERO = {10, 10};

extern const vector_t BUTTON_DIMENSION_SIZE;
extern const double TOGGLE_RADIUS;
extern const double INSTRUCTIONS_CLOSE_RADIUS;
extern const rgb_color_t TOGGLE_ON_COLOR;
extern const rgb_color_t TOGGLE_OFF_COLOR;

// Text positioning
const vector_t MENU_TEXT_POSITION = {350, 40};
const vector_t PLAYER_TEXT_POSITION = {350, 0};
const vector_t WINDOW_CENTER = {350, 220};
const vector_t INSTRUCTIONS_GOAL_POSITION = {50, 150};
const vector_t INSTRUCTIONS_CONTROLS_POSITION = {50, 200};
const vector_t INSTRUCTIONS_CHAOS_POSITION = {50, 250};
const vector_t INSTRUCTIONS_DEATHMATCH_POSITION = {50, 300};
const vector_t INSTRUCTIONS_POWERUP_POSITION = {50, 350};
const vector_t INSTRUCTIONS_CLOSING_POSITION = {300, 400};
const vector_t TWO_PLAYER_TEXT_POSITION = {49, 250};
const vector_t EASY_AI_TEXT_POSITION = {245, 250};
const vector_t HARD_AI_TEXT_POSITION = {438, 250};
const vector_t CHAOS_MODE_TEXT_POSITION = {600, 250};
const vector_t DEATHMATCH_TEXT_POSITION = {730, 250};
const vector_t POWERUP_TEXT_POSITION = {860, 250};

// Text dimensions
const vector_t TITLE_DIM = {300, 60};
const vector_t INSTRUCTIONS_DIM = {900, 30};
const vector_t INSTRUCTIONS_CLOSE_DIM = {400, 30};
const rgb_color_t WHITE = (rgb_color_t){1, 1, 1};
extern const vector_t FIRST_BUTTON_POSITION;
const vector_t BUTTON_TEXT_DIM = {100, 50};
const vector_t TOGGLE_TEXT_DIM = {100, 50};

extern const size_t NUM_SOLID_BALLS;
extern const size_t NUM_STRIPED_BALLS;

// general state IDS
const size_t MENU = 0;            // menu on screen
const size_t SIMULATION = 1;      // balls movin and hittin
const size_t CUEBALL_IN_HAND = 2; // ball in hand of player
const size_t SHOOTING = 3;        // cueball needs to be shot

// gamemodes
const size_t PVP = 0;     // player vs. player
const size_t SP_EASY = 1; // player vs. easy AI
const size_t SP_MED = 2;  // player vs. medium AI

// scoreboard consts
const vector_t P1_NAME_POS = (vector_t){10, 50};
const vector_t P2_NAME_POS = (vector_t){860, 50};
const vector_t P_NAME_SIZE = (vector_t){140, 40};
const vector_t P1_SIDE_POS = (vector_t){30, 90};
const vector_t P2_SIDE_POS = (vector_t){880, 90};
const vector_t P_SIDE_SIZE = (vector_t){100, 30};
const vector_t P1_BALL_POS = (vector_t){20, 350};
const vector_t P2_BALL_POS = (vector_t){870, 350};

// Power-up constants
extern const double POWER_UP_RADIUS;
extern const vector_t POWER_UP_MIN;
extern const vector_t POWER_UP_RANGE;

const double COOLDOWN_TIME = 1;
const double APPLAUSE_VOLUME = 60;
const double PLAYER_WIN_VOLUME = 600;

typedef struct gamestate {
    size_t num_solid;
    size_t num_striped;
    bool eightball_present;
    bool cueball_present;
    bool powerup_active;
} gamestate_t;

typedef struct scoreboard {
    text_info_t *p1, *p2;
    text_info_t **sides;
    list_t *p1_sprites;
    list_t *p2_sprites;
    vector_t *p1_sprite_pos;
    vector_t *p2_sprite_pos;
} scoreboard_t;

typedef struct player_info {
    size_t *player_number; // Either player 1 or 2
    size_t *side;          // Either solid (0) or striped (1). 2 is a placeholder until sides set
} player_info_t;

typedef struct state {
    scene_t *scene;
    size_t general;
    bool instructions_closed;
    bool pool_stick_on_scene;
    size_t current_player;
    size_t current_player_side;
    bool locked;
    bool chaos;
    bool powerup;
    bool deathmatch;
    size_t gamemode;
    bool turn_changed;
    scoreboard_t *sboard;
    text_info_t *player1_text;
    text_info_t *player2_text;
    text_info_t *player1_win_text;
    text_info_t *player2_win_text;
    gamestate_t *gamestate;
    text_info_t *menu_text;
    text_info_t *instructions_title;
    text_info_t *instructions_goal;
    text_info_t *instructions_controls;
    text_info_t *instructions_chaos;
    text_info_t *instructions_deathmatch;
    text_info_t *instructions_powerup;
    text_info_t *instructions_close;
    text_info_t *button1_text;
    text_info_t *button2_text;
    text_info_t *button3_text;
    text_info_t *toggle1_text;
    text_info_t *toggle2_text;
    text_info_t *toggle3_text;
    double time;
    bool instructions;
} state_t;

gamestate_t *track_game_state(state_t *state) {
    gamestate_t *ans = malloc(sizeof(gamestate_t));
    assert(ans != NULL);
    ans->cueball_present = cueball_in_play(state->scene);
    ans->eightball_present = eightball_in_play(state->scene);
    ans->num_solid = solid_count(state->scene);
    ans->num_striped = striped_count(state->scene);
    ans->powerup_active = powerup_triggered(state->scene);
    return ans;
}

size_t invert_side(size_t side) {
    if (side == 2)
        return 2;
    return 1 - side;
}

size_t invert_player(size_t player) {
    return player % 2 + 1;
}

void invert_player_n_side(state_t *state) {
    state->current_player = invert_player(state->current_player);
    state->current_player_side = invert_side(state->current_player_side);
}

size_t *counts_from_gamestate(gamestate_t *gstate) {
    size_t *ans = malloc(sizeof(size_t) * 4);
    assert(ans != NULL);
    ans[0] = gstate->num_solid;
    ans[1] = gstate->num_striped;
    ans[2] = ans[0] + ans[1];
    ans[3] = gstate->powerup_active;
    return ans;
}

void update_turn2(state_t *state) {
    if (state->turn_changed) { // meaning that cueball didn't move
        return;
    }
    gamestate_t *after_hit = track_game_state(state);
    bool cue = after_hit->cueball_present;
    bool powerup_on = after_hit->powerup_active;
    size_t *counts = counts_from_gamestate(after_hit);
    size_t *past_counts = counts_from_gamestate(state->gamestate);

    if (!cue) {
        state->general = CUEBALL_IN_HAND; // time to put cue back
        invert_player_n_side(state);
    } else {
        size_t curr_side = state->current_player_side;
        if (powerup_on && state->powerup) { // powerup takes precedence over whether you pocketed anything
            vector_t random_position = random_spot(state->scene);
            generate_power_up(state->scene, random_position);
            after_hit->powerup_active = false;
        } else if (counts[curr_side] < past_counts[curr_side]) { // friendly potted
            if (curr_side == 2) {                                // sides weren't decided
                if (counts[0] < past_counts[0] && counts[1] < past_counts[1]) {
                    invert_player_n_side(state); // can't decide which color you are...
                } else if (counts[0] < past_counts[0]) {
                    state->current_player_side = 0; // you take solids
                                                    // not switching turns
                } else if (counts[1] < past_counts[1]) {
                    state->current_player_side = 1; // you take stripes
                    // not swithcing side
                }
            } else { // sides were decided
                // nothing happens... current player continues
            }
        } else { // didn't pot anything
            invert_player_n_side(state);
        }
        state->general = SHOOTING; // cueball in play so time to shoot
    }
    free(state->gamestate); // update the gamestate
    state->gamestate = after_hit;
    free(counts);
    free(past_counts);
}

void stop_balls(state_t *state) {
    for (size_t i = 0; i < scene_bodies(state->scene); i++) {
        body_t *my_body = scene_get_body(state->scene, i);
        if (is_ball(my_body)) {
            body_set_velocity(my_body, VEC_ZERO);
        }
    }
}

void dummy_mouse_handler(state_t *state, mouse_event_type_t type, vector_t clicked_point) {
    // do nothing :)
}

void cueball_mouse_handler(state_t *state, mouse_event_type_t type, vector_t clicked_point) {
    if (type == MOUSE_RELEASED) {
        if (cueball_in_play(state->scene)) {
            state->general = SHOOTING; // we now shoot
        }
    }
    if (type == MOUSE_DRAGGED) {
        body_t *old_cueball = get_cueball_body(state->scene);
        if (old_cueball != NULL) {
            body_remove(old_cueball);
        }
        list_t *tentative_cue = generate_ball(clicked_point.x, clicked_point.y,
                                              get_ball_radius());
        size_t bodies = scene_bodies(state->scene);
        bool collided = false;
        for (size_t i = 0; i < bodies && !collided; i++) {
            body_t *body = scene_get_body(state->scene, i);
            if (body != old_cueball) {
                collided = (collided || find_collision(body_get_shape(body),
                                                       tentative_cue)
                                            .collided);
            }
        }
        if (!collided) { // so cueball position is valid, didn't collide
            put_cueball(state->scene, tentative_cue, state->chaos, state->powerup);
        } else {
            list_free(tentative_cue);
        }
    }
}

void shooting_mouse_handler(state_t *state, mouse_event_type_t type, vector_t clicked_point) {
    double separation = vec_magnitude(vec_subtract(body_get_centroid(get_cueball_body(state->scene)), clicked_point));
    if (type == MOUSE_RELEASED && state->pool_stick_on_scene) {
        body_t *stick = get_poolstick_body(state->scene);

        if (state->locked == false) { // locked on a direction
            state->locked = true;
            generate_power_bar(state->scene);
        } else {
            body_t *cue = get_cueball_body(state->scene);
            vector_t dir = vec_unit(vec_subtract(body_get_centroid(cue), body_get_centroid(stick)));
            destroy_angle_predictor(state->scene);
            body_add_impulse(stick, vec_multiply(body_get_mass(stick) * CUEBALL_INIT_VELOCITY * get_charge(state->scene), dir));
            state->locked = false;
            remove_power_bar(state->scene);
            state->pool_stick_on_scene = false;
            state->general = SIMULATION; // we start the simulation.
        }
    }
    if (type == MOUSE_DRAGGED) {
        if (separation > MIN_BALL_STICK_SEPARATION && state->locked == false) {
            update_poolstick(state->scene, clicked_point);
            state->pool_stick_on_scene = true;
            generate_angle_predictor(state->scene);
        } else if (state->pool_stick_on_scene && state->locked == false) {
            destroy_poolstick(state->scene);
            destroy_angle_predictor(state->scene);
            state->pool_stick_on_scene = false;
        }
    }
}

void menu_mouse_handler(state_t *state, mouse_event_type_t type, vector_t clicked_point) {
    if (!state->instructions) {
        double rad_2player_button = vec_magnitude(vec_subtract(body_get_centroid(get_2player_button_body(state->scene)), clicked_point));
        double rad_easyAI_button = vec_magnitude(vec_subtract(body_get_centroid(get_easyAI_button_body(state->scene)), clicked_point));
        double rad_hardAI_button = vec_magnitude(vec_subtract(body_get_centroid(get_hardAI_button_body(state->scene)), clicked_point));
        double rad_chaos_toggle = vec_magnitude(vec_subtract(body_get_centroid(get_chaos_toggle_body(state->scene)), clicked_point));
        double rad_deathmatch_toggle = vec_magnitude(vec_subtract(body_get_centroid(get_deathmatch_toggle_body(state->scene)), clicked_point));
        double rad_powerup_toggle = vec_magnitude(vec_subtract(body_get_centroid(get_powerup_toggle_body(state->scene)), clicked_point));

        if (type == MOUSE_CLICKED) {
            double toggle_choice = fmin(fmin(rad_chaos_toggle, rad_deathmatch_toggle), rad_powerup_toggle);
            if (toggle_choice < TOGGLE_RADIUS) {
                if (toggle_choice == rad_chaos_toggle) {
                    body_t *chaos_toggle = get_chaos_toggle_body(state->scene);
                    if (state->chaos) {
                        body_set_color(chaos_toggle, TOGGLE_OFF_COLOR);
                    } else if (!state->chaos) {
                        body_set_color(chaos_toggle, TOGGLE_ON_COLOR);
                    }
                    state->chaos = !state->chaos;
                }
                if (toggle_choice == rad_deathmatch_toggle) {
                    body_t *deathmatch_toggle = get_deathmatch_toggle_body(state->scene);
                    if (state->deathmatch) {
                        body_set_color(deathmatch_toggle, TOGGLE_OFF_COLOR);
                    } else if (!state->deathmatch) {
                        body_set_color(deathmatch_toggle, TOGGLE_ON_COLOR);
                    }
                    state->deathmatch = !state->deathmatch; 
                }
                if (toggle_choice == rad_powerup_toggle) {
                    body_t *powerup_toggle = get_powerup_toggle_body(state->scene);
                    if (state->powerup) {
                        body_set_color(powerup_toggle, TOGGLE_OFF_COLOR);
                    } else if (!state->powerup) {
                        body_set_color(powerup_toggle, TOGGLE_ON_COLOR);
                    }
                    state->powerup = !state->powerup;
                }
            }
        }

        if (type == MOUSE_CLICKED) {
            double game_choice = fmin(fmin(rad_2player_button, rad_easyAI_button), rad_hardAI_button);
            if (game_choice < BUTTON_DIMENSION_SIZE.y) {
                if (game_choice == rad_2player_button) {
                    state->gamemode = PVP;
                }
                if (game_choice == rad_easyAI_button) {
                    state->gamemode = SP_EASY;
                }
                if (game_choice == rad_hardAI_button) {
                    state->gamemode = SP_MED;
                }
                body_remove(get_hardAI_button_body(state->scene));
                body_remove(get_easyAI_button_body(state->scene));
                body_remove(get_2player_button_body(state->scene));
                body_remove(get_chaos_toggle_body(state->scene));
                body_remove(get_deathmatch_toggle_body(state->scene));
                body_remove(get_powerup_toggle_body(state->scene));
                sdl_free_text(state->menu_text);
                sdl_free_text(state->button1_text);
                sdl_free_text(state->button2_text);
                sdl_free_text(state->button3_text);
                sdl_free_text(state->toggle1_text);
                sdl_free_text(state->toggle2_text);
                sdl_free_text(state->toggle3_text);
                generate_instructions_close(state->scene);
                state->instructions = true;
            }
        }
    }
    if (type == MOUSE_CLICKED && state->instructions) {
        double rad_instructions_close = vec_magnitude(vec_subtract(body_get_centroid(get_instructions_close_body(state->scene)), clicked_point));
        double instructions_choice = rad_instructions_close;
        if (instructions_choice < INSTRUCTIONS_CLOSE_RADIUS) {
            body_remove(get_instructions_close_body(state->scene));
            sdl_free_text(state->instructions_title);
            sdl_free_text(state->instructions_goal);
            sdl_free_text(state->instructions_controls);
            sdl_free_text(state->instructions_chaos);
            sdl_free_text(state->instructions_deathmatch);
            sdl_free_text(state->instructions_powerup);
            sdl_free_text(state->instructions_close);
            body_remove(get_menu_background_body(state->scene));
            generate_pool_table(state->scene, state->chaos, state->powerup);
            
            state->gamestate = track_game_state(state);
            state->general = SHOOTING; // next state after table is generated
            state->instructions = false;
            sdl_on_mouse(dummy_mouse_handler);
        }
    }
}

bool deathmatch_is_over(state_t *state) {
    gamestate_t *after_hit = track_game_state(state);
    size_t counts = after_hit->num_solid + after_hit->num_striped;
    size_t past_counts = state->gamestate->num_solid + state->gamestate->num_striped;
    free(after_hit);
    if (!(counts < past_counts) && state->current_player_side != 2) {
        sdl_play_sound_effect("assets/win_applause.wav", APPLAUSE_VOLUME);
        if (state->current_player == 1) {
            sdl_play_sound_effect("assets/player_two_win.wav", PLAYER_WIN_VOLUME);
            sdl_render_text(state->player2_win_text);

        } else {
            sdl_play_sound_effect("assets/player_one_win.wav", PLAYER_WIN_VOLUME);
            sdl_render_text(state->player1_win_text);
        }
        return true;
        sdl_render_scene(state->scene);
        sdl_show();
        emscripten_cancel_main_loop();
        emscripten_force_exit(0);
    }
    return false;
}

/**
 * Checks if the game has ended.
 *
 * @param state the state of the game
 * @return true if game ended, false otherwise.
 * Will print messages on the screen if the game has ended.
 */
bool is_game_over(state_t *state) {
    gamestate_t *new_state = track_game_state(state);
    if (!new_state->eightball_present && state->general != MENU) {
        size_t *counts = counts_from_gamestate(new_state);
        size_t winner = 0;
        sdl_play_sound_effect("assets/win_applause.wav", APPLAUSE_VOLUME);

        if (!new_state->cueball_present) {                    // cueball also potted
            winner = invert_player(state->current_player);    // the other wins
        } else if (counts[state->current_player_side] == 0) { // potted all friendlies
            winner = state->current_player;
        } else { // failed to pot all friendlies
            winner = invert_player(state->current_player);
        }
        // print the appropriate text
        if (winner == 1) {
            sdl_play_sound_effect("assets/player_one_win.wav", PLAYER_WIN_VOLUME);
            sdl_render_text(state->player1_win_text);
        } else {
            sdl_play_sound_effect("assets/player_two_win.wav", PLAYER_WIN_VOLUME);
            sdl_render_text(state->player2_win_text);
        }
        free(new_state);
        free(counts);
        return true;
        sdl_render_scene(state->scene);
        sdl_show();
        emscripten_cancel_main_loop();
        sdl_free_text(state->player1_win_text);
        sdl_free_text(state->player2_win_text);
        emscripten_force_exit(0);
    }
    return false;
}

void when_to_change_turn(state_t *state) {
    body_t *cueball = get_cueball_body(state->scene);
    if (cueball != NULL) {
        if (fabs(body_get_velocity(cueball).x) > 0 || fabs(body_get_velocity(cueball).y) > 0) {
            state->turn_changed = false;
        }
    }
}

char *deepcopy_str(char *str) {
    char *ans = calloc(strlen(str) + 1, sizeof(char));
    assert(ans != NULL);
    strcat(ans, str);
    return ans;
}

void scoreboard_update(state_t *state) {
    char *sides[] = {"solids", "stripes", "N/A"};
    size_t other_player = invert_player(state->current_player);
    size_t other_p_side = invert_side(state->current_player_side);
    char *player_messages[3];
    player_messages[other_player] = sides[other_p_side];
    player_messages[state->current_player] = sides[state->current_player_side];
    sdl_free_text(state->sboard->sides[1]);
    sdl_free_text(state->sboard->sides[2]);
    state->sboard->sides[1] = sdl_add_text(player_messages[1],
                                           P1_SIDE_POS, P_SIDE_SIZE, WHITE);
    state->sboard->sides[2] = sdl_add_text(player_messages[2],
                                           P2_SIDE_POS, P_SIDE_SIZE, WHITE);

    if (other_p_side == 2) {
        return; // no need to calculate the pocketed balls
    }
    list_t *remaining[2];
    remaining[0] = list_init(NUM_SOLID_BALLS, free);
    remaining[1] = list_init(NUM_SOLID_BALLS, free);
    for (size_t i = 0; i < scene_bodies(state->scene); i++) {
        body_t *my_body = scene_get_body(state->scene, i);
        if (body_id(my_body) == SOLID_BALL_ID) {
            list_add(remaining[0], body_get_sprite(my_body).img_path);
        }
        if (body_id(my_body) == STRIPED_BALL_ID) {
            list_add(remaining[1], body_get_sprite(my_body).img_path);
        }
    }
    list_t *all_sprites = generate_ball_img_list();
    list_t *missing[2];
    missing[0] = list_init(NUM_SOLID_BALLS, free);
    missing[1] = list_init(NUM_SOLID_BALLS, free);
    for (size_t i = 0; i < 2 * NUM_SOLID_BALLS; i++) {
        bool found = false;
        char *current = (char *)list_get(all_sprites, i);
        for (size_t j = 0; !found && j < list_size(remaining[i / NUM_SOLID_BALLS]); j++) {
            if (strcmp((char *)list_get(remaining[i / NUM_SOLID_BALLS], j), current) == 0) {
                found = true;
            }
        }
        if (!found) {
            list_add(missing[i / NUM_SOLID_BALLS], deepcopy_str(current));
        }
    }
    list_free(state->sboard->p1_sprites);
    list_free(state->sboard->p2_sprites);
    list_free(remaining[0]);
    list_free(remaining[1]);
    list_t *player_missing[3];
    player_missing[state->current_player] = missing[state->current_player_side];
    player_missing[other_player] = missing[other_p_side];
    state->sboard->p1_sprites = player_missing[1];
    state->sboard->p2_sprites = player_missing[2];
    list_free(all_sprites);
}

vector_t *init_sprite_pos(vector_t start) {
    vector_t X_SPACING = (vector_t){2.5 * get_ball_radius(), 0};
    vector_t Y_SPACING = (vector_t){0, -2.5 * get_ball_radius()};
    vector_t *pos = malloc(sizeof(vector_t) * NUM_SOLID_BALLS);
    assert(pos != NULL);
    pos[0] = start;
    pos[1] = vec_add(start, vec_multiply(1, X_SPACING));
    pos[2] = vec_add(start, vec_multiply(2, X_SPACING));
    pos[3] = vec_add(start, vec_multiply(3, X_SPACING));
    pos[4] = vec_add(start, vec_add(vec_multiply(0, X_SPACING), Y_SPACING));
    pos[5] = vec_add(start, vec_add(vec_multiply(1, X_SPACING), Y_SPACING));
    pos[6] = vec_add(start, vec_add(vec_multiply(2, X_SPACING), Y_SPACING));
    return pos;
}

void scoreboard_init(state_t *state) {
    state->sboard = malloc(sizeof(scoreboard_t));
    assert(state->sboard != NULL);
    state->sboard->p1 = sdl_add_text("Player 1", P1_NAME_POS,
                                     P_NAME_SIZE, WHITE);
    state->sboard->p2 = sdl_add_text("Player 2", P2_NAME_POS,
                                     P_NAME_SIZE, WHITE);
    state->sboard->sides = malloc(sizeof(text_info_t *) * 3);
    state->sboard->sides[1] = sdl_add_text("N/A", P1_SIDE_POS,
                                           P_SIDE_SIZE, WHITE);
    state->sboard->sides[2] = sdl_add_text("N/A", P2_SIDE_POS,
                                           P_SIDE_SIZE, WHITE);
    state->sboard->p1_sprite_pos = init_sprite_pos(P1_BALL_POS);
    state->sboard->p2_sprite_pos = init_sprite_pos(P2_BALL_POS);
    state->sboard->p1_sprites = list_init(NUM_SOLID_BALLS, free);
    state->sboard->p2_sprites = list_init(NUM_SOLID_BALLS, free);
}

void scoreboard_free(scoreboard_t *sboard) {
    sdl_free_text(sboard->p1);
    sdl_free_text(sboard->p2);
    free(sboard->sides[1]);
    free(sboard->sides[2]);
    free(sboard->sides);
    free(sboard);
    list_free(sboard->p1_sprites);
    list_free(sboard->p2_sprites);
    free(sboard->p1_sprite_pos);
    free(sboard->p2_sprite_pos);
}

state_t *emscripten_init() {
    // set up seed for rand
    size_t ltime = time(NULL);
    size_t time = (unsigned)ltime / 2;
    srand(time);

    sdl_on_mouse(menu_mouse_handler);

    sdl_init(VEC_ZERO, WINDOW_SIZE);
    state_t *init_state = malloc(sizeof(state_t));
    assert(init_state != NULL);

    init_state->scene = scene_init();
    init_state->general = MENU; // we start in MENU general state
    init_state->pool_stick_on_scene = false;
    init_state->locked = false;
    init_state->chaos = false;
    init_state->powerup = false;
    init_state->deathmatch = false;
    generate_menu(init_state->scene);
    scoreboard_init(init_state);
    init_state->gamestate = malloc(sizeof(gamestate_t));
    assert(init_state->gamestate != NULL);

    init_state->current_player = 1;
    init_state->current_player_side = 2;

    init_state->time = 0;

    init_state->instructions = false;

    init_state->turn_changed = true;
    init_state->player1_text = sdl_add_text("Player 1 Turn!", PLAYER_TEXT_POSITION, TITLE_DIM, WHITE);
    init_state->player2_text = sdl_add_text("Player 2 Turn!", PLAYER_TEXT_POSITION, TITLE_DIM, WHITE);

    init_state->player1_win_text = sdl_add_text("Player 1 Wins!", WINDOW_CENTER, TITLE_DIM, WHITE);
    init_state->player2_win_text = sdl_add_text("Player 2 Wins!", WINDOW_CENTER, TITLE_DIM, WHITE);

    init_state->menu_text = sdl_add_text("Welcome to CS 3 Pool!", MENU_TEXT_POSITION, TITLE_DIM, WHITE);
    init_state->instructions_title = sdl_add_text("Instructions", MENU_TEXT_POSITION, TITLE_DIM, WHITE);
    init_state->instructions_goal = sdl_add_text("Goal: Hit all of one ball type (solid/striped) into the pockets on the pool table, and then hit the eightball into the pocket.", INSTRUCTIONS_GOAL_POSITION, INSTRUCTIONS_DIM, WHITE);
    init_state->instructions_controls = sdl_add_text("How to play: Drag your mouse to aim the poolstick at the cueball then click. If the cueball is pocketed, drag and click it back on the pool table.", INSTRUCTIONS_CONTROLS_POSITION, INSTRUCTIONS_DIM, WHITE);
    init_state->instructions_chaos = sdl_add_text("Gamemodes: Chaos mode: A chaotic rendition of the pool where when you hit a ball, another one moves!", INSTRUCTIONS_CHAOS_POSITION, INSTRUCTIONS_DIM, WHITE);
    init_state->instructions_deathmatch = sdl_add_text("Deathmatch: A rendition of pool where if a striped/solid ball isn't pocketed on your turn after the sides are designated, you lose!", INSTRUCTIONS_DEATHMATCH_POSITION, INSTRUCTIONS_DIM, WHITE);
    init_state->instructions_powerup = sdl_add_text("Powerup: If a player hits the powerup with the cueball, then you get an extra turn even if you don't pocket a ball!", INSTRUCTIONS_POWERUP_POSITION, INSTRUCTIONS_DIM, WHITE);
    init_state->instructions_close = sdl_add_text("Click the X to the top right to begin. Enjoy!", INSTRUCTIONS_CLOSING_POSITION, INSTRUCTIONS_CLOSE_DIM, WHITE);

    init_state->button1_text = sdl_add_text("2 Player", TWO_PLAYER_TEXT_POSITION, BUTTON_TEXT_DIM, WHITE);
    init_state->button2_text = sdl_add_text("Easy AI", EASY_AI_TEXT_POSITION, BUTTON_TEXT_DIM, WHITE);
    init_state->button3_text = sdl_add_text("Hard AI", HARD_AI_TEXT_POSITION, BUTTON_TEXT_DIM, WHITE);
    init_state->toggle1_text = sdl_add_text("Chaos Mode", CHAOS_MODE_TEXT_POSITION, BUTTON_TEXT_DIM, WHITE);
    init_state->toggle2_text = sdl_add_text("Deathmatch", DEATHMATCH_TEXT_POSITION, BUTTON_TEXT_DIM, WHITE);
    init_state->toggle3_text = sdl_add_text("Powerup", POWERUP_TEXT_POSITION, BUTTON_TEXT_DIM, WHITE);

    sdl_play_music("assets/bgmusic.ogg");
    return init_state;
}

void emscripten_main(state_t *state) {
    sdl_clear();
    double dt = time_since_last_tick();
    if (state->general == SIMULATION) {
        sdl_on_mouse(dummy_mouse_handler);
        when_to_change_turn(state);
        if (balls_stopped(state->scene)) {
            stop_balls(state);
            if (state->deathmatch) {
                if (deathmatch_is_over(state)) {
                    emscripten_force_exit(0); // we done
                }
            }
            if (is_game_over(state)) {
                emscripten_force_exit(0); // we done
            }
            update_turn2(state); // will also update state.general
            scoreboard_update(state);
        }
    }
    if (state->general == SHOOTING) {
        if (state->gamemode == SP_EASY && state->current_player == 2) {
            ai_easy_make_move(state->scene, state->current_player_side);
            state->general = SIMULATION;
        } else if (state->gamemode == SP_MED && state->current_player == 2) {
            ai_medium_make_move(state->scene, state->current_player_side);
            state->general = SIMULATION;
        } else {
            if (state->time < COOLDOWN_TIME) {
                state->time += dt;
            } else {
                sdl_on_mouse(shooting_mouse_handler);
            }
            if (state->locked == true) { // locked on a direction
                charge_power_bar(state->scene, dt * 0.5);
            }
        }
    }
    if (state->general == CUEBALL_IN_HAND) {
        if (state->gamemode == SP_EASY && state->current_player == 2) {
            list_t *cue = ai_easy_put_cue(state->scene, state->current_player_side);
            put_cueball(state->scene, cue, state->chaos, state->powerup);
            state->general = SHOOTING; // time to shoot
        } else if (state->gamemode == SP_MED && state->current_player == 2) {
            list_t *cue = ai_medium_put_cue(state->scene, state->current_player_side);
            put_cueball(state->scene, cue, state->chaos, state->powerup);
            state->general = SHOOTING;
        } else {
            sdl_on_mouse(cueball_mouse_handler);
        }
    }
    scene_tick(state->scene, dt);
    sdl_render_scene(state->scene);

    // text-handling area
    if (state->instructions == false) {
        if (state->general == MENU) { // MENU text
            sdl_render_text(state->menu_text);
            sdl_render_text(state->button1_text);
            sdl_render_text(state->button2_text);
            sdl_render_text(state->button3_text);
            sdl_render_text(state->toggle1_text);
            sdl_render_text(state->toggle2_text);
            sdl_render_text(state->toggle3_text);
        } else if (state->sboard != NULL) { // we need to print scoreboard
            sdl_render_text(state->sboard->p1);
            sdl_render_text(state->sboard->p2);
            sdl_render_text(state->sboard->sides[1]);
            sdl_render_text(state->sboard->sides[2]);
            for (size_t i = 0; i < list_size(state->sboard->p1_sprites); i++) {
                sdl_draw_raw_sprite(ball_sprite(state->sboard->p1_sprite_pos[i],
                                                (char *)list_get(state->sboard->p1_sprites, i)));
            }
            for (size_t i = 0; i < list_size(state->sboard->p2_sprites); i++) {
                sdl_draw_raw_sprite(ball_sprite(state->sboard->p2_sprite_pos[i],
                                                (char *)list_get(state->sboard->p2_sprites, i)));
            }
        }
    }
    if (state->general == SHOOTING) { // player names for shooting phase
        if (state->current_player == 1) {
            sdl_render_text(state->player1_text);
        } else if (state->current_player == 2) {
            sdl_render_text(state->player2_text);
        }
    }
    if (state->general == MENU && get_hardAI_button_body(state->scene) == NULL) {
        sdl_render_text(state->instructions_title);
        sdl_render_text(state->instructions_goal);
        sdl_render_text(state->instructions_controls);
        sdl_render_text(state->instructions_chaos);
        sdl_render_text(state->instructions_deathmatch);
        sdl_render_text(state->instructions_powerup);
        sdl_render_text(state->instructions_close);
    }
    sdl_show();
}

void emscripten_free(state_t *state) {
    scene_free(state->scene);
    scoreboard_free(state->sboard);
    free(state->gamestate);
    sdl_free_text(state->player1_text);
    sdl_free_text(state->player2_text);
    sdl_free_audio();

    free(state);
}
