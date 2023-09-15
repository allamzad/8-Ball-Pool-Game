#include "body.h"
#include "collision.h"
#include "color.h"
#include "forces.h"
#include "list.h"
#include "pool_menu.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "vector.h"
#include "shape_utility.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ids.h"
#include "pool_table.h"

const size_t NUM_BUTTONS = 3;
const size_t NUM_TOGGLES = 3;

body_t *get_2player_button_body(scene_t *scene)
{
    return get_specified_body(scene, MENU_BUTTON_2P_ID);
}

body_t *get_easyAI_button_body(scene_t *scene)
{
    return get_specified_body(scene, MENU_BUTTON_EASY_AI_ID);
}

body_t *get_hardAI_button_body(scene_t *scene)
{
    return get_specified_body(scene, MENU_BUTTON_HARD_AI_ID);
}

body_t *get_chaos_toggle_body(scene_t *scene)
{
    return get_specified_body(scene, MENU_BUTTON_CHAOS_ID);
}

body_t *get_deathmatch_toggle_body(scene_t *scene)
{
    return get_specified_body(scene, MENU_BUTTON_DEATHMATCH_ID);
}

body_t *get_menu_background_body(scene_t *scene)
{
    return get_specified_body(scene, MENU_BACKGROUND_ID);
}

body_t *get_powerup_toggle_body(scene_t *scene) {
    return get_specified_body(scene, MENU_BUTTON_POWERUP_ID);
}

body_t *get_instructions_close_body(scene_t *scene)
{
    return get_specified_body(scene, CLOSE_INSTRUCTIONS_BUTTON_ID);
}

void menu_generate_background(scene_t *scene)
{
    size_t *menu_background_ID = malloc(sizeof(size_t));
    assert(menu_background_ID != NULL);
    *menu_background_ID = MENU_BACKGROUND_ID;
    list_t *menu_background_shape = generate_menu_background_shape();
    sprite_info_t menu_background_sprite = menu_background_texture();
    body_t *my_menu =
        body_init_with_info(menu_background_shape, menu_background_sprite, INFINITY,
                            menu_background_ID, free);

    scene_add_body(scene, my_menu);
}

void menu_generate_buttons(scene_t *scene, size_t num_buttons)
{
    for (size_t i = 0; i < num_buttons; i++)
    {
        size_t *menu_button_ID = malloc(sizeof(size_t));
        assert(menu_button_ID != NULL);
        if (i == 0)
        {
            *menu_button_ID = MENU_BUTTON_2P_ID;
        }
        else if (i == 1)
        {
            *menu_button_ID = MENU_BUTTON_EASY_AI_ID;
        }
        else if (i == 2)
        {
            *menu_button_ID = MENU_BUTTON_HARD_AI_ID;
        }
        list_t *menu_button_shape = generate_menu_button_shape(i);
        sprite_info_t menu_button_sprite = menu_button_texture(i);
        body_t *my_button = body_init_with_info(menu_button_shape, 
                            menu_button_sprite, INFINITY, menu_button_ID, free);

        scene_add_body(scene, my_button);
    }
}

void menu_generate_toggles(scene_t *scene, size_t num_toggles)
{
    for (size_t i = 0; i < num_toggles; i++)
    {
        size_t *menu_toggle_ID = malloc(sizeof(size_t));
        assert(menu_toggle_ID != NULL);
        if (i == 0)
        {
            *menu_toggle_ID = MENU_BUTTON_CHAOS_ID;
        }
        else if (i == 1)
        {
            *menu_toggle_ID = MENU_BUTTON_DEATHMATCH_ID;
        }
        else if (i == 2) {
            *menu_toggle_ID = MENU_BUTTON_POWERUP_ID;
        }
        list_t *menu_toggle_shape = generate_menu_toggle_shape(i);
        sprite_info_t menu_toggle_sprite = menu_toggle_texture();
        body_t *my_toggle = body_init_with_info(menu_toggle_shape, 
                            menu_toggle_sprite, INFINITY, menu_toggle_ID, free);

        scene_add_body(scene, my_toggle);
    }
}

void generate_instructions_close(scene_t *scene) {
    size_t *instructions_close_ID = malloc(sizeof(size_t));
    assert(instructions_close_ID != NULL);
    *instructions_close_ID = CLOSE_INSTRUCTIONS_BUTTON_ID;
    list_t *instructions_close_shape = generate_instructions_close_shape();
    sprite_info_t instructions_close_sprite = instructions_close_texture();
    body_t *my_button = body_init_with_info(instructions_close_shape, 
            instructions_close_sprite, INFINITY, instructions_close_ID, free);

    scene_add_body(scene, my_button);
}

void generate_menu(scene_t *scene)
{
    menu_generate_background(scene);
    menu_generate_buttons(scene, NUM_BUTTONS);
    menu_generate_toggles(scene, NUM_TOGGLES);
}