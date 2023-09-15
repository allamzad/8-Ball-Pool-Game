#ifndef __SHAPE_UTILITY_H__
#define __SHAPE_UTILITY_H__

#include "list.h"
#include "polygon.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Generates a vector pointer at the given coordinates
 * @param x the x coordinate of the vector
 * @param y the y coordinate of the vector
 * @return the vector with coordinates (x, y)
 */
vector_t *gen_pos_vec(double x, double y);

/**
 * Generates a ball of the provided radius at the given coordinates.
 * @param x the x coordinate of the ball's center
 * @param y the y coordinate of the ball's center
 * @param radius the radius of the ball
 * @return the ball with the radius provided centered at (x, y)
 */
list_t *generate_ball(double x, double y, double radius);

/**
 * Generates a rectangle with the given width and height at the given coordinates.
 * @param rec_x the x coordinate of the rectangle
 * @param rec_y the y coordinate of the rectangle
 * @param rec_width the width of the rectangle
 * @param rec_height the height of the rectangle
 * @return the rectangle with dimensions rec_width X rec_height centered at (rec_x, rec_y)
 */
list_t *generate_rect_shape(double rec_x, double rec_y, double rec_width,
                            double rec_height);

#endif // #ifndef __SHAPE_UTILITY_H__