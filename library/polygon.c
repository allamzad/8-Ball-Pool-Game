#include "polygon.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

double polygon_area(list_t *polygon) {
  size_t poly_size = list_size(polygon);
  double area = 0;

  for (size_t i = 0; i < poly_size; i++) {
    vector_t *vi = list_get(polygon, i % poly_size);
    vector_t *vj = list_get(polygon, (i + 1) % poly_size);
    area += vec_cross(vi[0], vj[0]);
  }

  area /= 2;
  return area;
}

vector_t polygon_centroid(list_t *polygon) {
  size_t poly_size = list_size(polygon);
  double c_x = 0;
  double c_y = 0;
  double six_area = 6 * polygon_area(polygon);

  for (size_t i = 0; i < poly_size; i++) {
    vector_t *vi = list_get(polygon, i % poly_size);
    vector_t *vj = list_get(polygon, (i + 1) % poly_size);
    double xi = vi->x;
    double xj = vj->x;
    double yi = vi->y;
    double yj = vj->y;

    c_x += (xi + xj) * ((xi * yj) - (xj * yi));
    c_y += (yi + yj) * ((xi * yj) - (xj * yi));
  }

  c_x /= six_area;
  c_y /= six_area;

  vector_t centroid = {c_x, c_y};
  return centroid;
}

void polygon_translate(list_t *polygon, vector_t translation) {
  size_t poly_size = list_size(polygon);

  for (size_t i = 0; i < poly_size; i++) {
    vector_t *vi = list_get(polygon, i);
    *vi = vec_add(vi[0], translation);
  }
}

void polygon_rotate(list_t *polygon, double angle, vector_t point) {
  size_t poly_size = list_size(polygon);
  for (size_t i = 0; i < poly_size; i++) {
    vector_t *vi = list_get(polygon, i);

    // axis shift as rotation matrix around origin
    vi->x -= point.x;
    vi->y -= point.y;

    *vi = vec_rotate(vi[0], angle);

    // shift axis back
    vi->x += point.x;
    vi->y += point.y;
  }
}
