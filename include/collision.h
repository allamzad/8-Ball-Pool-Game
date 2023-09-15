#ifndef __COLLISION_H__
#define __COLLISION_H__

#include "list.h"
#include "vector.h"
#include <stdbool.h>

/**
 * Represents the status of a collision between two shapes.
 * The shapes are either not colliding, or they are colliding along some axis.
 */
typedef struct {
  /** Whether the two shapes are colliding */
  bool collided;
  /**
   * If the shapes are colliding, the axis they are colliding on.
   * This is a unit vector pointing from the first shape towards the second.
   * Normal impulses are applied along this axis.
   * If collided is false, this value is undefined.
   */
  vector_t axis;
} collision_info_t;

/**
 * Computes the status of the collision between two convex polygons.
 * The shapes are given as lists of vertices in counterclockwise order.
 * There is an edge between each pair of consecutive vertices,
 * and one between the first vertex and the last vertex.
 *
 * @param shape1 the first shape
 * @param shape2 the second shape
 * @return whether the shapes are colliding, and if so, the collision axis.
 * The axis should be a unit vector pointing from shape1 towards shape2.
 */
collision_info_t find_collision(list_t *shape1, list_t *shape2);

/**
 * Determines if the projections overlap on the axis's from shape1.
 *
 * @param shape1 the first shape
 * @param shape2 the second shape
 * @param info the updating collision info
 * @param overlap the updating overlap
 */
void find_collision_projs(list_t *shape1, list_t *shape2,
                          collision_info_t *info, double *overlap);

/**
 * Finds the minimum and maximum projection of a shape's vertices on a given
 * line.
 *
 * @param shape the shape
 * @param line the line
 * @return the minimum and maximum projection of a shape's vertices on the line
 */
vector_t find_shape_projs(list_t *shape, vector_t line);

#endif // #ifndef __COLLISION_H__
