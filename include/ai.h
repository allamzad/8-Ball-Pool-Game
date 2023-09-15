#ifndef __AI_H__
#define __AI_H__

#include "scene.h"
#include "body.h"
#include "pool_table.h"

/**
 * Exectues a random hit by the AI.
 * It will not display any poolstick, it will just add
 * an impulse to the cue ball.
 * 
 * @param scene the scene of the pool table
 * NOTE: function checks if cue ball is in play.
*/
void ai_random_move(scene_t *scene);

/**
 * Executes a hit by the easy AI.
 * It will not display any poolstick, it will just add
 * an impulse to the cue ball.
 * 
 * @param scene the scene of the pool table
 * @param side 0=solids, 1=stripes, 2=undecided
 * 
 * NOTE: function checks if cue ball is in play.
*/
void ai_easy_make_move(scene_t *scene, size_t side);

/**
 * Executes a hit by the medium AI.
 * It will not display any poolstick, it will just add
 * an impulse to the cue ball.
 * 
 * @param scene the scene of the pool table
 * @param side 0=solids, 1=stripes, 2=undecided
 * 
 * NOTE: function checks if cue ball is in play.
*/
void ai_medium_make_move(scene_t *scene, size_t side);

/**
 * Easy AI puts cueball back in game
 * 
 * @param scene the scene of the pool table
 * @param side 0=solids, 1=stripes, 2=undecided
 * 
 * @return shape of the cueball to be put back into game.
*/
list_t* ai_easy_put_cue(scene_t *scene, size_t side);

/**
 * Medium AI puts cueball back in game
 * 
 * @param scene the scene of the pool table
 * @param side 0=solids, 1=stripes, 2=undecided
 * 
 * @return shape of the cueball to be put back into game.
*/
list_t* ai_medium_put_cue(scene_t *scene, size_t side);

#endif // #ifndef __AI_H__