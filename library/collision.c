#include "collision.h"
#include "list.h"
#include "polygon.h"
#include "vec_list.h"
#include "vector.h"
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

collision_info_t find_collision(list_t *shape1, list_t *shape2) {
  collision_info_t *info = malloc(sizeof(collision_info_t));
  assert(info != NULL);
  info->collided = true;
  double *overlap = malloc(sizeof(double));
  assert(overlap != NULL);
  *overlap = INT_MAX;
  find_collision_projs(shape1, shape2, info, overlap);
  find_collision_projs(shape2, shape1, info, overlap);
  collision_info_t result = *info;
  free(overlap);
  free(info);
  if (result.collided) {
    if (vec_dot(result.axis, vec_subtract(polygon_centroid(shape2),
                                          polygon_centroid(shape1))) < 0) {
      result.axis = vec_negate(result.axis);
    }
  }
  return result;
}

double dmax(double a, double b) { return a > b ? a : b; }

double dmin(double a, double b) { return a < b ? a : b; }

void find_collision_projs(list_t *shape1, list_t *shape2,
                          collision_info_t *info, double *overlap) {
  size_t n_edges = list_size(shape1);
  for (int i = 0; i < n_edges; i++) {
    vector_t axis =
        vec_subtract(*(vector_t *)list_get(shape1, (i + 1) % n_edges),
                     *(vector_t *)list_get(shape1, i));
    axis = vec_unit(vec_rotate(axis, -M_PI / 2));
    vector_t shape1_projs = find_shape_projs(shape1, axis);
    vector_t shape2_projs = find_shape_projs(shape2, axis);
    double s1_min = shape1_projs.x;
    double s1_max = shape1_projs.y;
    double s2_min = shape2_projs.x;
    double s2_max = shape2_projs.y;
    double temp_overlap = dmin(s1_max, s2_max) - dmax(s1_min, s2_min);
    if (temp_overlap < 0) {
      info->collided = false;
      return;
    } else {
      if (temp_overlap < *overlap) {
        *overlap = temp_overlap;
        info->collided = true;
        info->axis = axis;
      }
    }
  }
}

vector_t find_shape_projs(list_t *shape, vector_t line) {
  double min = vec_proj(*(vector_t *)list_get(shape, 0), line);
  ;
  double max = min;
  for (int i = 1; i < list_size(shape); i++) {
    double proj = vec_proj(*(vector_t *)list_get(shape, i), line);
    if (proj < min) {
      min = proj;
    }
    if (proj > max) {
      max = proj;
    }
  }
  vector_t values = (vector_t){.x = min, .y = max};
  return values;
}
