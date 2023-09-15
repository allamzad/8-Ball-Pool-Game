#ifndef __SDL_WRAPPER_H__
#define __SDL_WRAPPER_H__

#include "color.h"
#include "list.h"
#include "scene.h"
#include "state.h"
#include "vector.h"
#include <stdbool.h>
#include "body.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

// Values passed to a key handler when the given arrow key is pressed
typedef enum
{
  LEFT_ARROW = 1,
  UP_ARROW = 2,
  RIGHT_ARROW = 3,
  DOWN_ARROW = 4
} arrow_key_t;

/**
 * The possible types of key events.
 * Enum types in C are much more primitive than in Java; this is equivalent to:
 * typedef unsigned int KeyEventType;
 * #define KEY_PRESSED 0
 * #define KEY_RELEASED 1
 */
typedef enum
{
  KEY_PRESSED,
  KEY_RELEASED
} key_event_type_t;

/**
 * The possible types of mouse events.
 */
typedef enum
{
  MOUSE_CLICKED,
  MOUSE_RELEASED,
  MOUSE_DRAGGED,
  MOUSE_MOVED
} mouse_event_type_t;

/**
 * The necessary text info
*/
typedef struct text_info {
  SDL_Surface* surfaceMessage;
  SDL_Texture* Message;
  vector_t center;
  vector_t dimensions;
} text_info_t;

/**
 * A keypress handler.
 * When a key is pressed or released, the handler is passed its char value.
 * Most keys are passed as their char value, e.g. 'a', '1', or '\r'.
 * Arrow keys have the special values listed above.
 *
 * @param key a character indicating which key was pressed
 * @param type the type of key event (KEY_PRESSED or KEY_RELEASED)
 * @param held_time if a press event, the time the key has been held in seconds
 */
typedef void (*key_handler_t)(state_t *state, char key, key_event_type_t type,
                              double held_time);

/**
 * A mousepress handler.
 *
 * @param key a character indicating which key was pressed
 * @param type the type of key event (KEY_PRESSED or KEY_RELEASED)
 * @param held_time if a press event, the time the key has been held in seconds
 */
typedef void (*mouse_handler_t)(state_t *state, mouse_event_type_t type, vector_t clicked_point);

/**
 * Initializes the SDL window and renderer.
 * Must be called once before any of the other SDL functions.
 *
 * @param min the x and y coordinates of the bottom left of the scene
 * @param max the x and y coordinates of the top right of the scene
 */
void sdl_init(vector_t min, vector_t max);

/**
 * Processes all SDL events and returns whether the window has been closed.
 * This function must be called in order to handle keypresses.
 *
 * @return true if the window was closed, false otherwise
 */
bool sdl_is_done(state_t *state);

/**
 * Clears the screen. Should be called before drawing polygons in each frame.
 */
void sdl_clear(void);

/**
 * Loops the .ogg file while the SDL window is open.
 *
 * @param music_path the file path containing the .ogg file
 */
void sdl_play_music(char *music_path);

/**
 * Plays the sound effect of the given .wav file.
 *
 * @param effect_path the file path containing the .wav file
 * @param volume the desired volume of the .wav file
 */
void sdl_play_sound_effect(char *effect_path, double volume);

/**
 * Frees all the audio played through music and sound effects.
 *
 */
void sdl_free_audio();

/**
 * Frees surface and texture of the given text_info_t object.
 * 
 * @param text the text_info_t object to be freed
 */
void sdl_free_text(text_info_t *text);

/**
 * Draws a polygon from the given list of vertices and a color.
 *
 * @param points the list of vertices of the polygon
 * @param color the color used to fill in the polygon
 */
void sdl_draw_polygon(list_t *points, rgb_color_t color);

/**
 * Draws a sprite at the position specified.
 * First, add the image to /assets/, must be a png,
 * ideally with a small file size. Then right click
 * and get the relative path and that will be the
 * img_path.
 *
 * @param sprite the sprite to be drawn
 */
void sdl_draw_raw_sprite(sprite_info_t sprite);

/**
 * Draws a body's sprite
 *
 * @param body the body containing the sprite.
 */
void sdl_draw_sprite(body_t *body);

/**
 * Adds text at the position specified.
 * First, add the text font to /assets/, must be a ttf,
 * ideally with a small file size. Then right click
 * and get the relative path and that will be the
 * img_path.
 *
 * @param text the text to be displayed
 * @param center where the text is centered
 * @param dimensions of text box
 * @param color of text
 */
text_info_t* sdl_add_text(char *text, vector_t center, vector_t dimension,
                          rgb_color_t color);

/**
 * Renders the text
*/
void sdl_render_text(text_info_t *text);

/**
 * Returns the dimensions of an image in terms of the window size
 *
 * @param img_path the relative path of the PNG
 * @return a vector with the dimensions of the image
 */
vector_t sdl_get_image_dimensions(char *img_path);
/**
 * Displays the rendered frame on the SDL window.
 * Must be called after drawing the polygons in order to show them.
 */
void sdl_show(void);

/**
 * Draws all bodies in a scene.
 * This internally calls sdl_clear(), sdl_draw_polygon(), and sdl_show(),
 * so those functions should not be called directly.
 *
 * @param scene the scene to draw
 */
void sdl_render_scene(scene_t *scene);

/**
 * Registers a function to be called every time a key is pressed.
 * Overwrites any existing handler.
 *
 * Example:
 * ```
 * void on_key(char key, key_event_type_t type, double held_time) {
 *     if (type == KEY_PRESSED) {
 *         switch (key) {
 *             case 'a':
 *                 printf("A pressed\n");
 *                 break;
 *             case UP_ARROW:
 *                 printf("UP pressed\n");
 *                 break;
 *         }
 *     }
 * }
 * int main(void) {
 *     sdl_on_key(on_key);
 *     while (!sdl_is_done());
 * }
 * ```
 *
 * @param handler the function to call with each key press
 */
void sdl_on_key(key_handler_t handler);

/**
 * Registers a function to be called every time mouse is clicked.
 * Overwrites any existing handler.
 */
void sdl_on_mouse(mouse_handler_t);

/**
 * Gets the amount of time that has passed since the last time
 * this function was called, in seconds.
 *
 * @return the number of seconds that have elapsed
 */
double time_since_last_tick(void);

#endif // #ifndef __SDL_WRAPPER_H__