#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include "color.h"
#include "list.h"
#include "vector.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

extern const size_t RECTANGLE_RESOLUTION;
extern const size_t BALL_RADIUS;
extern const vector_t POOLSTICK_DIMENSION;

/**
 * A general struct used to pass sprite texture info.
 */
typedef struct sprite_info {
    char *img_path;
    vector_t img_pos;
    vector_t img_scale;
    bool is_sprite;
    rgb_color_t color;
    SDL_Texture *texture;
    vector_t img_dim;
} sprite_info_t;

/**
 * Generates the positions of the balls in the rack at the start.
 *
 * @return the list containing the positions.
 */
list_t *generate_rack_pos();

/**
 * Generates a list containing all the img paths of the balls.
 *
 * @return the list containing the img paths.
 */
list_t *generate_ball_img_list();

/**
 * Generates the info of the striped balls' texture.
 *
 * @param img_list a list containing all the img paths of the balls.
 * @param rack_pos
 *
 * @return the shape of the stick body.
 */
sprite_info_t striped_balls_textures(list_t *img_list, list_t *rack_pos, size_t index);

/**
 * Returns the info of the solid balls' texture.
 *
 * @return the eightball's color/sprite info.
 */
sprite_info_t solid_balls_textures(list_t *img_list, list_t *rack_pos, size_t index);

/**
 * Creates a ball sprite.
 *
 * @param pos the positions where to spawn sprite
 * @param img_path the path to the ball png
 *
 * @return the sprite centered correctly
 */
sprite_info_t ball_sprite(vector_t pos, char *img_path);

/**
 * Returns the info of the eightball's texture.
 *
 * @return the eightball's color/sprite info.
 */
sprite_info_t eightball_texture();

/**
 * Returns the info of the cueball's texture.
 *
 * @param pos the position where to spawn the cueball
 *
 * @return the cueball's color/sprite info.
 */
sprite_info_t cueball_texture(vector_t pos);

/**
 * Returns the info of the pool table's texture.
 *
 * @return the pool table's color/sprite info.
 */
sprite_info_t table_top_texture();

/**
 * Returns the info of the wall's texture.
 *
 * @return the wall's color/sprite info.
 */
sprite_info_t wall_texture();

/**
 * Returns the info of the pockets's texture.
 *
 * @return the pocket's color/sprite info.
 */
sprite_info_t pocket_texture();

/**
 * Returns the info of the poolstick's texture.
 *
 * @return the poolstick's color/sprite info.
 */
sprite_info_t poolstick_texture();

/**
 * Returns the info of the menu background's texture.
 *
 * @return the menu background's color/sprite info.
 */
sprite_info_t menu_background_texture();

/**
 * Returns the info of the menu button's texture.
 *
 * @param index the index of the button (button 1, button 2, etc)
 *
 * @return the menu button's color/sprite info.
 */
sprite_info_t menu_button_texture(size_t index);

/**
 * Returns the info of the menu toggle button's texture.
 *
 * @param index the index of the toggle button (chaos, deathmatch, etc)
 *
 * @return the toggle button's color/sprite info.
 */
sprite_info_t menu_toggle_texture();

/**
 * Returns the info of the instructions close button's texture.
 *
 * @return the instruction close button's color/sprite info.
 */
sprite_info_t instructions_close_texture();

/**
 * Returns the info of the powerup's texture.
 *
 * @param position the position of the powerup's centroid
 *
 * @return the powerup's color/sprite info.
 */
sprite_info_t power_up_texture(vector_t position);

/**
 * Returns the info of the powerbar's inside texture.
 *
 * @return the powerbar inside's color/sprite info.
 */
sprite_info_t powerbar_inside_texture();

/**
 * Returns the info of the powerbar's outside texture.
 *
 * @return the powerbar outside's color/sprite info.
 */
sprite_info_t powerbar_outside_texture();

/**
 * Returns the info of the powerbar's charge texture.
 *
 * @return the powerbar's charge color/sprite info.
 */
sprite_info_t powerbar_charge_texture();

/**
 * Returns the info of the poolstick line of trajectory texture.
 *
 * @return the poolstick line of trajectory color/sprite info.
 */
sprite_info_t line_texture();

/**
 * Generates the polygon shape for the menu background.
 *
 * @return the list containing the menu background's polygon.
 */
list_t *generate_menu_background_shape();

/**
 * Generates the polygon shape for the menu button.
 *
 * @param index the index of the button (button 1, button 2, etc)
 *
 * @return the list containing the menu button's polygon
 */
list_t *generate_menu_button_shape(size_t index);

/**
 * Generates the polygon shape for the toggle buttons.
 *
 * @param index the index of the toggle button (chaos toggle, deathmatch toggle, etc)
 *
 * @return the list containing the toggle button's polygon
 */
list_t *generate_menu_toggle_shape(size_t index);

/**
 * Generates the shape for the button to the close the instructions.
 *
 * @return the list containing the instructions close button shape
 */
list_t *generate_instructions_close_shape();

/**
 * Generates the polygon shape for the top wall's body.
 *
 * @return the list containing the polygon's shape.
 */
list_t *generate_table_top_shape();

/**
 * Generates the polygon shape for the left wall's body.
 *
 * @return the list containing the polygon's shape.
 */
list_t *generate_table_left_shape();

/**
 * Generates the polygon shape for the right wall's body.
 *
 * @return the list containing the polygon's shape.
 */
list_t *generate_table_right_shape();

/**
 * Generates the polygon shape for the bottom wall's body.
 *
 * @return the list containing the polygon's shape.
 */
list_t *generate_table_bottom_shape();

/**
 * Generates the positions of the six pockets on the pool table.
 *
 * @return the list containing the pocket locations.
 */
list_t *generate_pocket_locations();

/**
 * Generates the poolstick shape.
 *
 * @param tip_position the position of the stick's tip
 * @param cueball_position the position of the cuball
 *
 * @return the shape of the stick body
 */
list_t *poolstick_shape(vector_t my_position,
                        vector_t cueball_position);

/**
 * Returns the radius of the balls.
 *
 * Used in AI.
 */
double get_ball_radius();

/**
 * Returns the number of pockets.
 *
 * Used in pool_table.c
 */
double get_num_pockets();

/**
 * Returns the eightball's initial position.
 *
 * Used in pool_table.c
 */
vector_t get_eightball_init_pos();

/**
 * Returns the cueball's initial position.
 *
 * Used in pool_table.c
 */
vector_t get_cueball_init_pos();

#endif // #ifndef __GRAPHICS_H__
