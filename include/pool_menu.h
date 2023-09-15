#ifndef __POOL_MENU_H__
#define __POOL_MENU_H__

#include "scene.h"

/**
 * Returns the two player button from the menu.
 *
 * @param scene the scene the menu is generated in
 * @return the two player button's body
 */
body_t *get_2player_button_body(scene_t *scene);

/**
 * Returns the easy AI button from the menu.
 *
 * @param scene the scene the menu is generated in
 * @return the easy AI button's body
 */
body_t *get_easyAI_button_body(scene_t *scene);

/**
 * Returns the hard AI button from the menu.
 *
 * @param scene the scene the menu is generated in
 * @return the hard AI button's body
 */
body_t *get_hardAI_button_body(scene_t *scene);

/**
 * Returns the chaos toggle button from the menu.
 *
 * @param scene the scene the menu is generated in
 * @return the chaos toggle button's body
 */
body_t *get_chaos_toggle_body(scene_t *scene);

/**
 * Returns the deathmatch toggle button from the menu.
 *
 * @param scene the scene the menu is generated in
 * @return the deathmatch toggle button's body
 */
body_t *get_deathmatch_toggle_body(scene_t *scene);

/**
 * Returns the menu background's body.
 *
 * @param scene the scene the menu is generated in
 * @return the menu background's body
 */
body_t *get_menu_background_body(scene_t *scene);

/**
 * Returns the instruction close button body.
 *
 * @param scene the scene the menu is generated in
 * @return the instruction close button body
 */
body_t *get_instructions_close_body(scene_t *scene);

/**
 * Generates the button used to close the instructions screen.
 *
 * @param scene the scene to generate the button in
 */
void generate_instructions_close(scene_t *scene);

/**
 * Returns the powerup toggle from menu.
 *
 * @param scene the scene the menu is generated in
 * @return the powerup toggle button's body
 */
body_t *get_powerup_toggle_body(scene_t *scene);

/**
 * Master function that generates the entire menu at the
 * start of the game. A background with 3 buttons (2 player, easy AI, hard AI)
 * are generated.
 *
 * @param scene the scene to generate the menu in
 */
void generate_menu(scene_t *scene);


#endif // #ifndef __POOL_MENU_H__