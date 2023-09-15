#include "graphics.h"
#include "body.h"
#include "collision.h"
#include "color.h"
#include "forces.h"
#include "ids.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "shape_utility.h"
#include "vector.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const vector_t GRAPHICS_WINDOW_SIZE = (vector_t){.x = 1000, .y = 500};

// Ball constants
const size_t BALL_RADIUS = 12;
const size_t BALL_SPACING = 3;
const size_t NUM_BALLS = 14;
const rgb_color_t BALL_COLOR = (rgb_color_t){0, 0, 0};
const vector_t EIGHTBALL_INIT_POS = {.x = 600, .y = 250};
const vector_t CUEBALL_INIT_POS = {.x = 250, .y = 250};

// Table constants
const size_t RECTANGLE_RESOLUTION = 4;
const vector_t TABLE_DIMENSIONS = (vector_t){.x = 700, .y = 400};
const size_t NUM_POCKETS = 6;
const size_t WALL_LIST_CAPACITY = 11;

// Image scaling
const vector_t STRIPED_BALL_IMG_SCALE = {0.8, 0.8};
const vector_t SOLID_BALL_IMG_SCALE = {0.8, 0.8};
const vector_t EIGHT_BALL_IMG_SCALE = {0.8, 0.8};
const vector_t CUE_BALL_IMG_SCALE = {0.8, 0.8};

// is_sprite values (true shows sprite, false shows body)
const bool STRIPED_BALL_IS_SPRITE = true;
const bool SOLID_BALL_IS_SPRITE = true;
const bool EIGHT_BALL_IS_SPRITE = true;
const bool CUE_BALL_IS_SPRITE = true;
const bool POOLTABLE_IS_SPRITE = true; // top wall
const bool WALL_IS_SPRITE = true;      // other walls
const bool POCKET_IS_SPRITE = true;
const bool POOLSTICK_IS_SPRITE = false;
const bool MENU_BACKGROUND_IS_SPRITE = true;
const bool MENU_BUTTON_IS_SPRITE = true;
const bool MENU_TOGGLE_IS_SPRITE = false;
const bool POWER_UP_IS_SPRITE = true;
const bool INSTRUCTIONS_CLOSE_IS_SPRITE = true;
const bool POWER_IS_SPRITE = false;
const bool LINE_IS_SPRITE = false;

// Body colors
const rgb_color_t STRIPED_BALL_COLOR = {0, 1, 0};
const rgb_color_t SOLID_BALL_COLOR = {1, 0, 0};
const rgb_color_t EIGHT_BALL_COLOR = {0, 0, 0};
const rgb_color_t CUE_BALL_COLOR = {1, 1, 1};
const rgb_color_t WALL_COLOR = {0.36, 0.25, 0.2};
const rgb_color_t POCKET_COLOR = {0, 0, 0};
const rgb_color_t POOLSTICK_COLOR = {0.77, 0.64, 0.52};
const rgb_color_t MENU_BACKGROUND_COLOR = {1, 1, 1};
const rgb_color_t MENU_BUTTON_COLOR = {1, 1, 1};
const rgb_color_t TOGGLE_ON_COLOR = {0, 1, 0};
const rgb_color_t TOGGLE_OFF_COLOR = {1, 0, 0};
const rgb_color_t POWER_UP_COLOR = {0.2, 0.2, 0.2};
const rgb_color_t INSTRUCTIONS_CLOSE_COLOR = {0.3, 0.3, 0.3};
const rgb_color_t POWER_BAR_INSIDE_COLOR = (rgb_color_t){1, 1, 1};
const rgb_color_t POWER_BAR_OUTSIDE_COLOR = (rgb_color_t){0, 0, 0};
const rgb_color_t POWER_BAR_CHARGE_COLOR = (rgb_color_t){1, 0, 0};
const rgb_color_t LINE_COLOR = {1, 1, 1};

// Menu constants
const vector_t MENU_WINDOW_SIZE = (vector_t){.x = 1000, .y = 500};
const vector_t MENU_POSITION = (vector_t){.x = 500, .y = 250};
const vector_t BUTTON_DIMENSION_SIZE = (vector_t){.x = 130, .y = 75};
const vector_t FIRST_BUTTON_POSITION = (vector_t){.x = 100, .y = 150};
const vector_t FIRST_TOGGLE_POSITION = (vector_t){.x = 650, .y = 150};
const vector_t INSTRUCTIONS_CLOSE_POSITION = (vector_t){.x = 900, .y = 450};
const double INSTRUCTIONS_CLOSE_RADIUS = 15;
const double TOGGLE_RADIUS = 15;
const double TOGGLE_SPACING = 130;
const double MENU_BUTTON_SPACING = 195;

// Miscellaneous Dimensions
const vector_t POOLSTICK_DIMENSION = (vector_t){.x = 290, .y = 10};
const vector_t POWER_UP_DIMENSIONS = (vector_t){.x = 20, .y = 20};

//-----------------------Ball Body Generation------------------------------

list_t *generate_rack_pos() {
    list_t *rack_pos = list_init(NUM_BALLS, free);

    // row 1
    list_add(rack_pos, gen_pos_vec(EIGHTBALL_INIT_POS.x - 4 * BALL_RADIUS 
     - 2 * BALL_SPACING, EIGHTBALL_INIT_POS.y));
    // row 2
    list_add(rack_pos, gen_pos_vec(EIGHTBALL_INIT_POS.x - 2 * BALL_RADIUS
     - BALL_SPACING, EIGHTBALL_INIT_POS.y - BALL_RADIUS - BALL_SPACING));
    list_add(rack_pos, gen_pos_vec(EIGHTBALL_INIT_POS.x - 2 * BALL_RADIUS
     - BALL_SPACING, EIGHTBALL_INIT_POS.y + BALL_RADIUS + BALL_SPACING));
    // row 3 (skip the 8-ball)
    list_add(rack_pos, gen_pos_vec(EIGHTBALL_INIT_POS.x,
        EIGHTBALL_INIT_POS.y - 2 * BALL_RADIUS - BALL_SPACING));
    list_add(rack_pos, gen_pos_vec(EIGHTBALL_INIT_POS.x,
        EIGHTBALL_INIT_POS.y + 2 * BALL_RADIUS + BALL_SPACING));
    // row 4
    list_add(rack_pos, gen_pos_vec(EIGHTBALL_INIT_POS.x + 2 * BALL_RADIUS
        + BALL_SPACING, EIGHTBALL_INIT_POS.y - 3 * BALL_RADIUS - 2 * BALL_SPACING));
    list_add(rack_pos, gen_pos_vec(EIGHTBALL_INIT_POS.x + 2 * BALL_RADIUS 
        + BALL_SPACING, EIGHTBALL_INIT_POS.y - BALL_RADIUS - BALL_SPACING));
    list_add(rack_pos, gen_pos_vec(EIGHTBALL_INIT_POS.x + 2 * BALL_RADIUS 
        + BALL_SPACING, EIGHTBALL_INIT_POS.y + BALL_RADIUS + BALL_SPACING));
    list_add(rack_pos, gen_pos_vec(EIGHTBALL_INIT_POS.x + 2 * BALL_RADIUS 
        + BALL_SPACING, EIGHTBALL_INIT_POS.y + 3 * BALL_RADIUS + 2 * BALL_SPACING));
    // row 5
    list_add(rack_pos, gen_pos_vec(EIGHTBALL_INIT_POS.x + 4 * BALL_RADIUS +
        2 * BALL_SPACING, EIGHTBALL_INIT_POS.y - 4 * BALL_RADIUS - 2 * BALL_SPACING));
    list_add(rack_pos, gen_pos_vec(EIGHTBALL_INIT_POS.x + 4 * BALL_RADIUS +
        2 * BALL_SPACING, EIGHTBALL_INIT_POS.y - 2 * BALL_RADIUS - BALL_SPACING));
    list_add(rack_pos, gen_pos_vec(EIGHTBALL_INIT_POS.x + 4 * BALL_RADIUS +
        2 * BALL_SPACING, EIGHTBALL_INIT_POS.y));
    list_add(rack_pos, gen_pos_vec(EIGHTBALL_INIT_POS.x + 4 * BALL_RADIUS +
        2 * BALL_SPACING, EIGHTBALL_INIT_POS.y + 2 * BALL_RADIUS + BALL_SPACING));
    list_add(rack_pos, gen_pos_vec(EIGHTBALL_INIT_POS.x + 4 * BALL_RADIUS +
        2 * BALL_SPACING, EIGHTBALL_INIT_POS.y + 4 * BALL_RADIUS + 2 * BALL_SPACING));
    return rack_pos;
}

list_t *generate_ball_img_list() {
    list_t *img_list = list_init(NUM_BALLS, free);
    list_add(img_list, (char *)("assets/one.png"));
    list_add(img_list, (char *)("assets/two.png"));
    list_add(img_list, (char *)("assets/three.png"));
    list_add(img_list, (char *)("assets/four.png"));
    list_add(img_list, (char *)("assets/five.png"));
    list_add(img_list, (char *)("assets/six.png"));
    list_add(img_list, (char *)("assets/seven.png"));
    list_add(img_list, (char *)("assets/nine.png"));
    list_add(img_list, (char *)("assets/ten.png"));
    list_add(img_list, (char *)("assets/eleven.png"));
    list_add(img_list, (char *)("assets/twelve.png"));
    list_add(img_list, (char *)("assets/thirteen.png"));
    list_add(img_list, (char *)("assets/fourteen.png"));
    list_add(img_list, (char *)("assets/fifteen.png"));
    return img_list;
}

sprite_info_t striped_balls_textures(list_t *img_list, list_t *rack_pos, size_t index) {
    vector_t position = *(vector_t *)list_get(rack_pos, 2 * index + 1);
    return (sprite_info_t){
        .img_path = (char *)list_get(img_list, index + NUM_BALLS / 2),
        .img_pos = {position.x, position.y},
        .img_scale = STRIPED_BALL_IMG_SCALE,
        .is_sprite = STRIPED_BALL_IS_SPRITE,
        .color = STRIPED_BALL_COLOR,
        .texture = NULL,
        .img_dim = sdl_get_image_dimensions((char *)list_get(img_list,
                                                             index + NUM_BALLS / 2))};
}

sprite_info_t solid_balls_textures(list_t *img_list, list_t *rack_pos, size_t index) {
    vector_t position = *(vector_t *)list_get(rack_pos, 2 * index);
    return (sprite_info_t){
        .img_path = (char *)list_get(img_list, index),
        .img_pos = {position.x, position.y},
        .img_scale = SOLID_BALL_IMG_SCALE,
        .is_sprite = SOLID_BALL_IS_SPRITE,
        .color = SOLID_BALL_COLOR,
        .texture = NULL,
        .img_dim = sdl_get_image_dimensions((char *)list_get(img_list, index))};
}

sprite_info_t eightball_texture() {
    return (sprite_info_t){
        .img_path = "assets/eight.png",
        .img_pos = {EIGHTBALL_INIT_POS.x, EIGHTBALL_INIT_POS.y},
        .img_scale = EIGHT_BALL_IMG_SCALE,
        .is_sprite = EIGHT_BALL_IS_SPRITE,
        .color = EIGHT_BALL_COLOR,
        .texture = NULL,
        .img_dim = sdl_get_image_dimensions("assets/eight.png")};
}

sprite_info_t ball_sprite(vector_t pos, char *img_path) {
    return (sprite_info_t){
        .img_path = img_path,
        .img_pos = {pos.x, pos.y},
        .img_scale = SOLID_BALL_IMG_SCALE,
        .is_sprite = SOLID_BALL_IS_SPRITE,
        .color = SOLID_BALL_COLOR,
        .texture = NULL,
        .img_dim = sdl_get_image_dimensions(img_path)};
}

sprite_info_t cueball_texture(vector_t pos) {
    return (sprite_info_t){
        .img_path = "assets/cue.png",
        .img_pos = {pos.x, pos.y},
        .img_scale = CUE_BALL_IMG_SCALE,
        .is_sprite = CUE_BALL_IS_SPRITE,
        .color = CUE_BALL_COLOR,
        .texture = NULL,
        .img_dim = sdl_get_image_dimensions("assets/cue.png")};
}

//-----------------------Table Body Generation------------------------------

/* Table shapes are hard-coded in to match the table graphics. */
list_t *generate_table_top_shape() {
    list_t *wall_shape_top = list_init(WALL_LIST_CAPACITY, free);
    list_add(wall_shape_top, gen_pos_vec(224, 405));
    list_add(wall_shape_top, gen_pos_vec(478, 405));
    list_add(wall_shape_top, gen_pos_vec(483, 415));
    list_add(wall_shape_top, gen_pos_vec(483, 440));
    list_add(wall_shape_top, gen_pos_vec(518, 440));
    list_add(wall_shape_top, gen_pos_vec(518, 415));
    list_add(wall_shape_top, gen_pos_vec(522, 405));
    list_add(wall_shape_top, gen_pos_vec(776, 405));
    list_add(wall_shape_top, gen_pos_vec(850, 450));
    list_add(wall_shape_top, gen_pos_vec(150, 450));
    list_add(wall_shape_top, gen_pos_vec(224, 405));
    return wall_shape_top;
}

list_t *generate_table_left_shape() {
    list_t *wall_shape_left = list_init(WALL_LIST_CAPACITY, free);
    list_add(wall_shape_left, gen_pos_vec(150, 390));
    list_add(wall_shape_left, gen_pos_vec(181, 390));
    list_add(wall_shape_left, gen_pos_vec(195, 377));
    list_add(wall_shape_left, gen_pos_vec(195, 123));
    list_add(wall_shape_left, gen_pos_vec(183, 110));
    list_add(wall_shape_left, gen_pos_vec(150, 110));
    list_add(wall_shape_left, gen_pos_vec(150, 390));
    return wall_shape_left;
}

list_t *generate_table_right_shape() {
    list_t *wall_shape_right = list_init(WALL_LIST_CAPACITY, free);
    list_add(wall_shape_right, gen_pos_vec(820, 390));
    list_add(wall_shape_right, gen_pos_vec(805, 377));
    list_add(wall_shape_right, gen_pos_vec(805, 124));
    list_add(wall_shape_right, gen_pos_vec(820, 110));
    list_add(wall_shape_right, gen_pos_vec(850, 110));
    list_add(wall_shape_right, gen_pos_vec(850, 390));
    list_add(wall_shape_right, gen_pos_vec(820, 390));
    return wall_shape_right;
}

list_t *generate_table_bottom_shape() {
    list_t *wall_shape_bottom = list_init(WALL_LIST_CAPACITY, free);
    list_add(wall_shape_bottom, gen_pos_vec(224, 95));
    list_add(wall_shape_bottom, gen_pos_vec(180, 50));
    list_add(wall_shape_bottom, gen_pos_vec(810, 50));
    list_add(wall_shape_bottom, gen_pos_vec(783, 95));
    list_add(wall_shape_bottom, gen_pos_vec(522, 95));
    list_add(wall_shape_bottom, gen_pos_vec(517, 85));
    list_add(wall_shape_bottom, gen_pos_vec(517, 60));
    list_add(wall_shape_bottom, gen_pos_vec(483, 60));
    list_add(wall_shape_bottom, gen_pos_vec(483, 85));
    list_add(wall_shape_bottom, gen_pos_vec(478, 95));
    list_add(wall_shape_bottom, gen_pos_vec(224, 95));
    return wall_shape_bottom;
}

list_t *generate_pocket_locations() {
    list_t *pocket_locations = list_init(NUM_POCKETS, free);
    list_add(pocket_locations, gen_pos_vec(196, 404));
    list_add(pocket_locations, gen_pos_vec(500, 406));
    list_add(pocket_locations, gen_pos_vec(806, 405));
    list_add(pocket_locations, gen_pos_vec(196, 97));
    list_add(pocket_locations, gen_pos_vec(500, 94));
    list_add(pocket_locations, gen_pos_vec(804, 97));
    return pocket_locations;
}

sprite_info_t table_top_texture() {
    vector_t img_dim = sdl_get_image_dimensions("assets/pooltable.png");
    return (sprite_info_t){
        .img_path = "assets/pooltable.png",
        .img_pos = {GRAPHICS_WINDOW_SIZE.x / 2, GRAPHICS_WINDOW_SIZE.y / 2},
        .img_scale = {TABLE_DIMENSIONS.x / img_dim.x, TABLE_DIMENSIONS.y / img_dim.y},
        .is_sprite = WALL_IS_SPRITE,
        .color = WALL_COLOR,
        .texture = NULL,
        .img_dim = sdl_get_image_dimensions("assets/pooltable.png")};
}

sprite_info_t wall_texture() {
    return (sprite_info_t){
        .img_path = NULL,
        .img_pos = VEC_ZERO,
        .img_scale = VEC_ZERO,
        .is_sprite = WALL_IS_SPRITE,
        .color = WALL_COLOR,
        .texture = NULL,
        .img_dim = VEC_ZERO};
}

sprite_info_t pocket_texture() {
    return (sprite_info_t){
        .img_path = NULL,
        .img_pos = VEC_ZERO,
        .img_scale = VEC_ZERO,
        .is_sprite = POCKET_IS_SPRITE,
        .color = POCKET_COLOR,
        .texture = NULL,
        .img_dim = VEC_ZERO};
}

//-----------------------Poolstick Generation------------------------------

list_t *poolstick_shape(vector_t tip_position, vector_t cueball_position) {
    vector_t vector_of_attack = vec_unit(vec_subtract(cueball_position, tip_position));
    vector_t orthogonal_vector_of_attack = (vector_t){vector_of_attack.y,
                                                      -vector_of_attack.x};

    list_t *ans = list_init(RECTANGLE_RESOLUTION, free);

    vector_t *stick_point_vec = malloc(sizeof(vector_t));
    assert(stick_point_vec != NULL);
    *stick_point_vec = vec_add(vec_multiply((double)POOLSTICK_DIMENSION.y / 2,
                                            orthogonal_vector_of_attack),
                               tip_position);
    vector_t previous_stick_point_vec = *stick_point_vec;
    list_add(ans, stick_point_vec);

    stick_point_vec = malloc(sizeof(vector_t));
    assert(stick_point_vec != NULL);
    *stick_point_vec = vec_subtract(previous_stick_point_vec, vec_multiply(
                            (double)POOLSTICK_DIMENSION.x, vector_of_attack));
    previous_stick_point_vec = *stick_point_vec;
    list_add(ans, stick_point_vec);

    stick_point_vec = malloc(sizeof(vector_t));
    assert(stick_point_vec != NULL);
    *stick_point_vec = vec_subtract(previous_stick_point_vec, vec_multiply(
                            (double)POOLSTICK_DIMENSION.y, orthogonal_vector_of_attack));
    previous_stick_point_vec = *stick_point_vec;
    list_add(ans, stick_point_vec);

    stick_point_vec = malloc(sizeof(vector_t));
    assert(stick_point_vec != NULL);
    *stick_point_vec = vec_add(previous_stick_point_vec, vec_multiply(
                            (double)POOLSTICK_DIMENSION.x, vector_of_attack));
    list_add(ans, stick_point_vec);

    return ans;
}

sprite_info_t poolstick_texture() {
    return (sprite_info_t){
        .img_path = NULL,
        .img_pos = VEC_ZERO,
        .img_scale = VEC_ZERO,
        .is_sprite = POOLSTICK_IS_SPRITE,
        .color = POOLSTICK_COLOR,
        .texture = NULL,
        .img_dim = VEC_ZERO};
}

sprite_info_t powerbar_inside_texture() {
    return (sprite_info_t){
        .img_path = NULL,
        .img_pos = VEC_ZERO,
        .img_scale = VEC_ZERO,
        .is_sprite = POWER_IS_SPRITE,
        .color = POWER_BAR_INSIDE_COLOR,
        .texture = NULL,
        .img_dim = VEC_ZERO};
}

sprite_info_t powerbar_outside_texture() {
    return (sprite_info_t){
        .img_path = NULL,
        .img_pos = VEC_ZERO,
        .img_scale = VEC_ZERO,
        .is_sprite = POWER_IS_SPRITE,
        .color = POWER_BAR_OUTSIDE_COLOR,
        .texture = NULL,
        .img_dim = VEC_ZERO};
}

sprite_info_t powerbar_charge_texture() {
    return (sprite_info_t){
        .img_path = NULL,
        .img_pos = VEC_ZERO,
        .img_scale = VEC_ZERO,
        .is_sprite = POWER_IS_SPRITE,
        .color = POWER_BAR_CHARGE_COLOR,
        .texture = NULL,
        .img_dim = VEC_ZERO};
}

sprite_info_t line_texture()
{
    return (sprite_info_t){
        .img_path = NULL,
        .img_pos = VEC_ZERO,
        .img_scale = VEC_ZERO,
        .is_sprite = LINE_IS_SPRITE,
        .color = LINE_COLOR,
        .texture = NULL,
        .img_dim = VEC_ZERO};
}

//-----------------------Menu Generation------------------------------

list_t *generate_menu_background_shape() {
    return generate_rect_shape(MENU_POSITION.x, MENU_POSITION.y,
                               MENU_WINDOW_SIZE.x, MENU_WINDOW_SIZE.y);
}

list_t *generate_menu_button_shape(size_t index) {
    return generate_rect_shape(FIRST_BUTTON_POSITION.x + (MENU_BUTTON_SPACING * index),
                               FIRST_BUTTON_POSITION.y, BUTTON_DIMENSION_SIZE.x,
                               BUTTON_DIMENSION_SIZE.y);
}

list_t *generate_menu_toggle_shape(size_t index) {
    return generate_ball(FIRST_TOGGLE_POSITION.x + (TOGGLE_SPACING * index),
                         FIRST_TOGGLE_POSITION.y, TOGGLE_RADIUS);
}

list_t *generate_instructions_close_shape() {
    return generate_ball(INSTRUCTIONS_CLOSE_POSITION.x, INSTRUCTIONS_CLOSE_POSITION.y,
                         INSTRUCTIONS_CLOSE_RADIUS);
}

sprite_info_t menu_background_texture() {
    vector_t img_dim = sdl_get_image_dimensions("assets/menubackground.png");
    return (sprite_info_t){
        .img_path = "assets/menubackground.png",
        .img_pos = {MENU_POSITION.x, MENU_POSITION.y},
        .img_scale = {MENU_WINDOW_SIZE.x / img_dim.x, MENU_WINDOW_SIZE.y / img_dim.y},
        .is_sprite = MENU_BACKGROUND_IS_SPRITE,
        .color = MENU_BACKGROUND_COLOR,
        .texture = NULL,
        .img_dim = img_dim};
}

sprite_info_t menu_button_texture(size_t index) {
    vector_t img_dim = sdl_get_image_dimensions("assets/menubutton.png");
    return (sprite_info_t){
        .img_path = "assets/menubutton.png",
        .img_pos = {FIRST_BUTTON_POSITION.x + (MENU_BUTTON_SPACING * index),
                    FIRST_BUTTON_POSITION.y},
        .img_scale = {BUTTON_DIMENSION_SIZE.x / img_dim.x, 
                    BUTTON_DIMENSION_SIZE.y / img_dim.y},
        .is_sprite = MENU_BUTTON_IS_SPRITE,
        .color = MENU_BUTTON_COLOR,
        .texture = NULL,
        .img_dim = img_dim};
}

sprite_info_t menu_toggle_texture() {
    return (sprite_info_t){
        .img_path = NULL,
        .img_pos = VEC_ZERO,
        .img_scale = VEC_ZERO,
        .is_sprite = MENU_TOGGLE_IS_SPRITE,
        .color = TOGGLE_OFF_COLOR,
        .texture = NULL,
        .img_dim = VEC_ZERO};
}

sprite_info_t instructions_close_texture() {
    vector_t img_dim = sdl_get_image_dimensions("assets/close.png");
    return (sprite_info_t){
        .img_path = "assets/close.png",
        .img_pos = INSTRUCTIONS_CLOSE_POSITION,
        .img_scale = {2 * INSTRUCTIONS_CLOSE_RADIUS / img_dim.x,
                      2 * INSTRUCTIONS_CLOSE_RADIUS / img_dim.y},
        .is_sprite = INSTRUCTIONS_CLOSE_IS_SPRITE,
        .color = INSTRUCTIONS_CLOSE_COLOR,
        .texture = NULL,
        .img_dim = img_dim};
}

//-----------------------Power Up Generation------------------------------

sprite_info_t power_up_texture(vector_t position) {
    vector_t img_dim = sdl_get_image_dimensions("assets/powerup.png");
    return (sprite_info_t){
        .img_path = "assets/powerup.png",
        .img_pos = position,
        .img_scale = {POWER_UP_DIMENSIONS.x / img_dim.x,
                      POWER_UP_DIMENSIONS.y / img_dim.y},
        .is_sprite = POWER_UP_IS_SPRITE,
        .color = POWER_UP_COLOR,
        .texture = NULL,
        .img_dim = img_dim};
}

//-----------------------Constant Getters------------------------------

double get_ball_radius() {
    return (double)BALL_RADIUS;
}

double get_num_pockets() {
    return (double)NUM_POCKETS;
}

vector_t get_eightball_init_pos() {
    return (vector_t)EIGHTBALL_INIT_POS;
}

vector_t get_cueball_init_pos() {
    return (vector_t)CUEBALL_INIT_POS;
}