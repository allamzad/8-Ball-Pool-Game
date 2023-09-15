#ifndef __ANGLE_H__
#define __ANGLE_H__

#include "scene.h"
#include "vector.h"

/**
 * Generates a line shape between 2 points with specified width
 *
 * @param start_position starting position of desired line
 * @param start_position ending position of desired line
 * @param width desired width of line
 */
list_t *line_shape(vector_t start_position, vector_t end_position, double width);

/**
 * Generates the predicted trajectory line of cueball following hit by stick.
 *
 * @param scene the scene of the table
 */
void generate_angle_predictor(scene_t *scene);

/**
 * Destroys the predicted trajectory line body.
 *
 * @param scene the scene of the table
 */
void destroy_angle_predictor(scene_t *scene);

#endif // #ifndef __ANGLE_H__
