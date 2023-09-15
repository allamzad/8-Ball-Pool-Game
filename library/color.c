#include "color.h"
#include "assert.h"
#include <stdio.h>
#include <stdlib.h>

rgb_color_t color_init(float r, float g, float b) {
  assert(r >= 0.0 && r <= 1.0);
  assert(g >= 0.0 && g <= 1.0);
  assert(b >= 0.0 && b <= 1.0);
  rgb_color_t color = {r, g, b};
  return color;
}

float color_get_r(rgb_color_t color) { return color.r; }

float color_get_g(rgb_color_t color) { return color.g; }

float color_get_b(rgb_color_t color) { return color.b; }

void color_set_r(rgb_color_t color, float red) {
  assert(red >= 0.0 && red <= 1.0);
  color.r = red;
};

void color_set_g(rgb_color_t color, float green) {
  assert(green >= 0.0 && green <= 1.0);
  color.g = green;
};

void color_set_b(rgb_color_t color, float blue) {
  assert(blue >= 0.0 && blue <= 1.0);
  color.b = blue;
};

rgb_color_t generate_random_color() {
  rgb_color_t my_color;
  my_color.r = (float)rand() / (float)RAND_MAX;
  my_color.b = (float)rand() / (float)RAND_MAX;
  my_color.g = (float)rand() / (float)RAND_MAX;

  return my_color;
}
