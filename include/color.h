#ifndef __COLOR_H__
#define __COLOR_H__

/**
 * A color to display on the screen.
 * The color is represented by its red, green, and blue components.
 * Each component must be between 0 (black) and 1 (white).
 */

typedef struct rgb_color {
  float r;
  float g;
  float b;
} rgb_color_t;

/**
 * Returns a rgb_color with the specificied amount of red, blue, and green
 *
 * @param r the redness of the rgb_color
 * @param g the greenness of the rgb_color
 * @param b the blueness of the rgb_color
 * @return the display color of a rgb_color specified
 */
rgb_color_t color_init(float r, float g, float b);

/**
 * Gets the amount of red color in a rgb_color.
 *
 * @param color the display color of a rgb_color
 * @return a float indicating the redness of the rgb_color
 */
float color_get_r(rgb_color_t color);

/**
 * Gets the amount of green color in a rgb_color.
 *
 * @param color the display color of a rgb_color
 * @return the greenness of the rgb_color
 */
float color_get_g(rgb_color_t color);

/**
 * Gets the amount of blue color in a rgb_color.
 *
 * @param color the display color of a rgb_color
 * @return the blueness of the rgb_color
 */
float color_get_b(rgb_color_t color);

/**
 * Sets the amount of red color in a rgb_color
 *
 * @param color the display color of a rgb_color
 * @param red desired new red level
 */
void color_set_r(rgb_color_t color, float red);

/**
 * Sets the amount of green color in a rgb_color.
 *
 * @param color the display color of a rgb_color
 * @param green desired new green level
 */
void color_set_g(rgb_color_t color, float green);

/**
 * Sets the amount of blue color in a rgb_color.
 *
 * @param color the display color of a rgb_color
 * @param blue desired new blue level
 */
void color_set_b(rgb_color_t color, float blue);

/**
 * Generates a random color
 *
 * @return random color
 */
rgb_color_t generate_random_color();

#endif // #ifndef __COLOR_H__
