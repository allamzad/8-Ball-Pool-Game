#ifndef __POOL_TABLE_H__
#define __POOL_TABLE_H__

#include "scene.h"
#include "body.h"

void generate_table(scene_t *scene);

/**
 * Generates the ID for an object.
 *
 * @param object_id the integer representing the object's id.
 *
 * @return the object's id.
 */
size_t *generate_ID(size_t object_id);

/**
 * Creates a new pool table with all balls at the start position.
 *
 * @param scene the scene to generate the table in
 * @param chaos whether we are in chaos mode
 * @param powerup whether powerup is being played with
 */
void generate_pool_table(scene_t *scene, bool chaos, bool powerup);

/**
 * Adds the moving cue stick to the scene.
 * It will automatically add the "elastic-destructive" collision
 * with the cue ball.
 *
 * @param scene the scene of the pool table
 * @param stick the body instance of the stick
 */
void add_moving_stick(scene_t *scene, body_t *stick);

/**
 * Checks if the given body is a ball (including the cue ball).
 *
 * @param body the body to be checked
 * @return true if given body is a ball, false otherwise
 */
bool is_ball(body_t *body);

/**
 * Checks if the balls are not moving.
 * NOTE: it will return false if the stick still
 * hasn't hit the cue ball (i.e., stick is still in game)
 *
 * @param scene the scene of the pool table
 * @return true if the balls stopped moving.
 */
bool balls_stopped(scene_t *scene);

/**
 * Generates the poolstick body.
 *
 * TODO: WRITE BETTER DOC HERE!!
 */
void generate_poolstick(scene_t *scene, vector_t my_position, vector_t cueball_position);

/**
 * Gets the cueball body.
 *
 * @param scene the scene of the pool table
 * 
 * @return pointer to cueball or NULL if doesn't exist
 */
body_t *get_cueball_body(scene_t *scene);

/**
 * Gets the poolstick body.
 *
 * @param scene the scene of the pool table
 * 
 * @return pointer to poolstick or NULL if doesn't exist
 */
body_t *get_poolstick_body(scene_t *scene);

/**
 * Finds the number of striped balls still in play.
 *
 * @param scene the scene of the pool table
 * @return the number of striped balls in play
 */
size_t striped_count(scene_t *scene);

/**
 * Finds the number of solid balls still in play.
 *
 * @param scene the scene of the pool table
 * @return the number of solid balls in play
 */
size_t solid_count(scene_t *scene);

/**
 * Counts the number of balls in play with given ID.
 *
 * @param scene the table's scene
 * @param searched_id the id of the balls you want to count
 */
size_t count_balls(scene_t *scene, size_t searched_id);

/**
 * Checks if the eightball is still in play
 *
 * @param scene the scene of the pool table
 * @return true if the eightball is in play, otherwise false
 */
bool eightball_in_play(scene_t *scene);

/**
 * Checks if the cueball is still in play
 *
 * @param scene the scene of the pool table
 * @return true if the cueball is in play, otherwise false
 */
bool cueball_in_play(scene_t *scene);

/**
 * Checks if the powerup has been activated
 *
 * @param scene the scene of the pool table
 * @return true if the powerup is activated (removed from table), otherwise false
 */
bool powerup_triggered(scene_t *scene);

/**
 * This is a collision handler used for "pocketing" the balls.
 * It destroys the ball that hits the pocket body.
 *
 * @param body1 the ball body
 * @param body2 the pocket body
 * @param axis the collision axis
 * @param aux not used, can be anything
 *
 * NOTE: the order of body1 and body2 matters!!!
 */
void ball_in_pocket(body_t *body1, body_t *body2, vector_t axis, void *aux);

/**
 * This is a collision handler used for the powerup.
 * It destroys the powerup.
 *
 * @param body1 the powerup
 * @param body2 the ball body
 * @param axis the collision axis
 * @param aux not used, can be anything
 *
 * NOTE: the order of body1 and body2 matters!!!
 */
void ball_in_pocket(body_t *body1, body_t *body2, vector_t axis, void *aux);

/**
 * Generates all 3 layers of power bar: the charge, the black outline, and the white
 * interior.
 *
 * @param scene the scene of the pool table
 */
void generate_power_bar(scene_t *scene);

/**
 * Gets the power bar inside body (white element)
 *
 * @param scene the scene of the pool table
 * @return pointer to power bar inside body
 */
body_t *get_power_bar_inside_body(scene_t *scene);

/**
 * Gets the power bar outside body (black element)
 *
 * @param scene the scene of the pool table
 * @return pointer to power bar outside body
 */
body_t *get_power_bar_outside_body(scene_t *scene);

/**
 * Gets the power bar charge body (red element)
 *
 * @param scene the scene of the pool table
 * @return pointer to power bar charge body
 */
body_t *get_power_bar_charge_body(scene_t *scene);

/**
 * Adds a charge value to power bar charge (a value from 0 to 1) of power bar.
 * Modifies charge body accordingly (scales the red element to match the charge fraction).
 *
 * @param scene the scene of the pool table
 * @param charge_add the amount of charge being added to current charge
 */
void charge_power_bar(scene_t *scene, double charge_add);

/**
 * Removes all 3 bodies of power bar.
 *
 * @param scene the scene of the pool table
 */
void remove_power_bar(scene_t *scene);

/**
 * Gets current charge value of power bar (value between 0 and 1).
 *
 * @param scene the scene of the pool table
 * @return the current charge of the power bar
 */
double get_charge(scene_t *scene);

/**
 * Destroys the poolstick body
 *
 * @param scene the scene of the pool table
 */
void destroy_poolstick(scene_t *scene);

/**
 * Gets a first body in scene bodies (if it exists) that matches a given ID.
 *
 * @param scene the scene of the pool table
 * @param id specified ID
 * 
 * @return pointer to found body or NULL if doesn't exist
 */
body_t *get_specified_body(scene_t *scene, size_t id);

/**
 * Generates pool stick if one does not exist, else moves poolstick to be at
 * given position and pointing towards cueball.
 *
 * @param scene the scene of the pool table.
 * @param my_position the position we want to place the stick
 */
void update_poolstick(scene_t *scene, vector_t my_position);

/**
 * Adds collisions and drag forces to the scene according to whether we are in chaos
 * mode or not.
 *
 * @param scene the scene of the table
 * @param chaos whether we are in chaos mode
 * @param powerup whether we are playing with powerup
 */
void add_collisions(scene_t *scene, bool chaos, bool powerup);

/**
 * Sets up a destructive collision between the cueball and stick in the scene
 * (destructive such that stick is destroyed after collision but not the cueball).
 *
 * @param stick body of the poolstick
 * @param scene the scene of the table
 * @param elasticity the desired elasticity of the collision
 */
void setup_ball_stick_collisions(body_t *stick, scene_t *scene, double elasticity);

/**
 * Generates the power up body.
 *
 * @param scene the scene of the pool table.
 * @param position the position we want to place the powerup.
 */
void generate_power_up(scene_t *scene, vector_t position);

/**
 * Function used to put cueball back in play.
 * 
 * @param scene the scene of the pool table
 * @param shape of the cueball
 * @param chaos true/false if we are in chaos mode.
 * @param powerup if we are playing with powerup
 * 
 * NOTE: it is assumed that shape is VALID! (no collisions, right radius)
 * It will also add the collision forcers for this new cueball
*/
void put_cueball(scene_t *scene, list_t *shape, bool chaos, bool powerup);

/**
 * Returns the id of the given body.
 * 
 * @param body the body
 * @return ID of body
*/
size_t body_id(body_t *body);

/**
 * Shuffles a size_t array of size n
 * 
 * @param array the array to be modified
 * @param n length of the array
*/
void shuffle(size_t *array, size_t n);

/**
 * Returns a new location for the powerup.
 * 
 * @param scene the scene
 * @return location of new powerup
*/
vector_t random_spot(scene_t *scene);

#endif // #ifndef __POOL_TABLE_H__
