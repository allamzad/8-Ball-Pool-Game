#include "vector.h"
#include <math.h>
#include <stdlib.h>

const vector_t VEC_ZERO = {0, 0};

vector_t vec_add(vector_t v1, vector_t v2) {
  v1.x = v1.x + v2.x;
  v1.y = v1.y + v2.y;
  return v1;
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
  return vec_add(v1, vec_negate(v2));
}

vector_t vec_negate(vector_t v) {
  v.x = v.x * -1;
  v.y = v.y * -1;
  return v;
}

vector_t vec_multiply(double scalar, vector_t v) {
  v.x = v.x * scalar;
  v.y = v.y * scalar;
  return v;
}

double vec_dot(vector_t v1, vector_t v2) { return v1.x * v2.x + v1.y * v2.y; }

double vec_cross(vector_t v1, vector_t v2) { return v1.x * v2.y - v1.y * v2.x; }

vector_t vec_rotate(vector_t v, double angle) {
  double rotated_x = v.x * cos(angle) - v.y * sin(angle);
  double rotated_y = v.x * sin(angle) + v.y * cos(angle);
  v.x = rotated_x;
  v.y = rotated_y;
  return v;
}

double vec_magnitude(vector_t v) { return sqrt(vec_dot(v, v)); }

vector_t vec_unit(vector_t v) { return vec_multiply(1 / vec_magnitude(v), v); }

double vec_proj(vector_t v, vector_t line) {
  return vec_dot(v, vec_unit(line));
}
