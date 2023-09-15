#include "sdl_wrapper.h"
#include "color.h"
#include "graphics.h"
#include "scene.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector.h>

const char WINDOW_TITLE[] = "CS 3 Pool";
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 500;
const rgb_color_t WINDOW_COLOR = {3, 37, 126};
const double MS_PER_S = 1e3;
const char *FONT = "assets/SourceSansPro-Regular.ttf";
const double FONT_RESOLUTION = 100;

/**
 * The coordinate at the center of the screen.
 */
vector_t center;
/**
 * The coordinate difference from the center to the top right corner.
 */
vector_t max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 * The keypress handler, or NULL if none has been configured.
 */
key_handler_t key_handler = NULL;
/**
 * The mouse handler, or NULL if none has been configured.
 */
mouse_handler_t mouse_handler = NULL;
/**
 * The wave file of the sound effect being played.
 */
Mix_Chunk *wave = NULL;
/**
 * The ogg file of the background music being played.
 */
Mix_Music *music = NULL;
/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;

uint32_t mouse_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

/** Computes the center of the window in pixel coordinates */
vector_t get_window_center(void) {
    int *width = malloc(sizeof(*width)), *height = malloc(sizeof(*height));
    assert(width != NULL);
    assert(height != NULL);
    SDL_GetWindowSize(window, width, height);
    vector_t dimensions = {.x = *width, .y = *height};
    free(width);
    free(height);
    return vec_multiply(0.5, dimensions);
}

/**
 * Computes the scaling factor between scene coordinates and pixel coordinates.
 * The scene is scaled by the same factor in the x and y dimensions,
 * chosen to maximize the size of the scene while keeping it in the window.
 */
double get_scene_scale(vector_t window_center) {
    // Scale scene so it fits entirely in the window
    double x_scale = window_center.x / max_diff.x,
           y_scale = window_center.y / max_diff.y;
    return x_scale < y_scale ? x_scale : y_scale;
}

/** Maps a scene coordinate to a window coordinate */
vector_t get_window_position(vector_t scene_pos, vector_t window_center) {
    // Scale scene coordinates by the scaling factor
    // and map the center of the scene to the center of the window
    vector_t scene_center_offset = vec_subtract(scene_pos, center);
    double scale = get_scene_scale(window_center);
    vector_t pixel_center_offset = vec_multiply(scale, scene_center_offset);
    vector_t pixel = {.x = round(window_center.x + pixel_center_offset.x),
                      // Flip y axis since positive y is down on the screen
                      .y = round(window_center.y - pixel_center_offset.y)};
    return pixel;
}

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
    switch (key) {
    case SDLK_LEFT:
        return LEFT_ARROW;
    case SDLK_UP:
        return UP_ARROW;
    case SDLK_RIGHT:
        return RIGHT_ARROW;
    case SDLK_DOWN:
        return DOWN_ARROW;
    case SDLK_SPACE:
        return '_';
    default:
        // Only process 7-bit ASCII characters
        return key == (SDL_Keycode)(char)key ? key : '\0';
    }
}

void sdl_init(vector_t min, vector_t max) {
    // Check parameters
    assert(min.x < max.x);
    assert(min.y < max.y);

    center = vec_multiply(0.5, vec_add(min, max));
    max_diff = vec_subtract(max, center);
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                              SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    TTF_Init();
    Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
}

bool sdl_is_done(state_t *state) {
    SDL_Event *event = malloc(sizeof(*event));
    assert(event != NULL);
    while (SDL_PollEvent(event)) {
        switch (event->type) {
        case SDL_QUIT:
            free(event);
            return true;

        case SDL_KEYDOWN:

        case SDL_KEYUP:
            // Skip the keypress if no handler is configured
            // or an unrecognized key was pressed
            if (key_handler == NULL)
                break;
            char key = get_keycode(event->key.keysym.sym);
            if (key == '\0')
                break;

            uint32_t timestamp = event->key.timestamp;
            if (!event->key.repeat) {
                key_start_timestamp = timestamp;
            }
            key_event_type_t type =
                event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
            double held_time = (timestamp - key_start_timestamp) / MS_PER_S;
            key_handler(state, key, type, held_time);
            break;

        case SDL_MOUSEBUTTONUP:
            if (mouse_handler == NULL) {
                break;
            }
            mouse_event_type_t mtype =
                event->type == SDL_MOUSEBUTTONDOWN ? MOUSE_CLICKED : MOUSE_RELEASED;
            vector_t point = (vector_t){(double)event->button.x,
                                        WINDOW_HEIGHT - (double)event->button.y};
            mouse_handler(state, mtype, point);
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (mouse_handler == NULL) {
                break;
            }
            mtype = MOUSE_CLICKED;
            point = (vector_t){(double)event->button.x, WINDOW_HEIGHT 
                    - (double)event->button.y};

            mouse_handler(state, mtype, point);
            break;

        case SDL_MOUSEMOTION:
            if (mouse_handler == NULL) {
                break;
            }
            vector_t motion_point = (vector_t){(double)event->button.x,
                                    WINDOW_HEIGHT - (double)event->button.y};
            mouse_event_type_t motion_mtype =
                event->motion.state == SDL_PRESSED ? MOUSE_DRAGGED : MOUSE_MOVED;
            mouse_handler(state, motion_mtype, motion_point);
            break;
        }
    }
    free(event);
    return false;
}

void sdl_clear(void) {
    SDL_SetRenderDrawColor(renderer, WINDOW_COLOR.r,
                           WINDOW_COLOR.g, WINDOW_COLOR.b, 0);
    SDL_RenderClear(renderer);
}

void sdl_play_music(char *music_path) {
    music = Mix_LoadMUS(music_path);
    Mix_PlayMusic(music, -1);
}

void sdl_play_sound_effect(char *effect_path, double volume) {
    wave = Mix_LoadWAV(effect_path);
    Mix_VolumeChunk(wave, volume);
    Mix_PlayChannel(-1, wave, 0);
}

void sdl_free_audio() {
    Mix_FreeMusic(music);
    Mix_FreeChunk(wave);
}

void sdl_free_text(text_info_t *text) {
    SDL_FreeSurface(text->surfaceMessage);
    SDL_DestroyTexture(text->Message);
    free(text);
}

void sdl_draw_raw_sprite(sprite_info_t sprite) {
    SDL_Texture *img;
    if (sprite.texture == NULL) {
        img = IMG_LoadTexture(renderer, sprite.img_path);
        sprite.texture = img;
    }
    img = sprite.texture;
    SDL_Rect texr;
    texr.w = sprite.img_dim.x * sprite.img_scale.x;
    texr.h = sprite.img_dim.y * sprite.img_scale.y;
    texr.x = sprite.img_pos.x - texr.w / 2;

    if (sprite.img_pos.y < WINDOW_HEIGHT / 2) {
        sprite.img_pos.y = sprite.img_pos.y + 2 * fabs(WINDOW_HEIGHT / 2
                           - sprite.img_pos.y);
    } else {
        sprite.img_pos.y = sprite.img_pos.y - 2 * fabs(WINDOW_HEIGHT / 2
                           - sprite.img_pos.y);
    }
    texr.y = sprite.img_pos.y - texr.h / 2;
    SDL_RenderCopy(renderer, img, NULL, &texr);
    sdl_show();
    SDL_DestroyTexture(img);
}

void sdl_draw_sprite(body_t *body) {
    sprite_info_t sprite = body_get_sprite(body);
    sdl_draw_raw_sprite(sprite);
}

text_info_t *sdl_add_text(char *text, vector_t center, vector_t dimensions,
                          rgb_color_t color) {
    // Opens a font style and sets resolution
    TTF_Font *Sans = TTF_OpenFont(FONT, FONT_RESOLUTION);
    SDL_Color my_color = {(int32_t)color_get_r(color) * 255,
                          (int32_t)color_get_g(color) * 255,
                          (int32_t)color_get_b(color) * 255};

    SDL_Surface *surfaceMessage =
        TTF_RenderText_Blended_Wrapped(Sans, text, my_color, 0);

    SDL_Texture *Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

    SDL_Rect Message_rect;
    Message_rect.x = center.x;
    Message_rect.y = center.y;
    Message_rect.w = dimensions.x;
    Message_rect.h = dimensions.y;

    text_info_t *result = malloc(sizeof(text_info_t));
    *result = (text_info_t){surfaceMessage, Message, center, dimensions};
    return result;
}

void sdl_render_text(text_info_t *text) {
    SDL_Rect Message_rect;
    Message_rect.x = text->center.x;
    Message_rect.y = text->center.y;
    Message_rect.w = text->dimensions.x;
    Message_rect.h = text->dimensions.y;

    SDL_RenderCopy(renderer, text->Message, NULL, &Message_rect);
    sdl_show();
}

vector_t sdl_get_image_dimensions(char *img_path) {
    int w, h;
    SDL_Texture *img = NULL;
    img = IMG_LoadTexture(renderer, img_path);
    assert(SDL_QueryTexture(img, NULL, NULL, &w, &h) == 0);
    vector_t img_dim = {w, h};
    SDL_DestroyTexture(img);
    return img_dim;
}

void sdl_draw_polygon(list_t *points, rgb_color_t color) {
    // Check parameters
    size_t n = list_size(points);
    assert(n >= 3);
    assert(0 <= color.r && color.r <= 1);
    assert(0 <= color.g && color.g <= 1);
    assert(0 <= color.b && color.b <= 1);

    vector_t window_center = get_window_center();

    // Convert each vertex to a point on screen
    int16_t *x_points = malloc(sizeof(*x_points) * n),
            *y_points = malloc(sizeof(*y_points) * n);
    assert(x_points != NULL);
    assert(y_points != NULL);
    for (size_t i = 0; i < n; i++) {
        vector_t *vertex = list_get(points, i);
        vector_t pixel = get_window_position(*vertex, window_center);
        x_points[i] = pixel.x;
        y_points[i] = pixel.y;
    }

    // Draw polygon with the given color
    filledPolygonRGBA(renderer, x_points, y_points, n, color.r * 255,
                      color.g * 255, color.b * 255, 255);
    free(x_points);
    free(y_points);
    sdl_show();
}

void sdl_show(void) {
    // Draw boundary lines
    vector_t window_center = get_window_center();
    vector_t max = vec_add(center, max_diff),
             min = vec_subtract(center, max_diff);
    vector_t max_pixel = get_window_position(max, window_center),
             min_pixel = get_window_position(min, window_center);
    SDL_Rect *boundary = malloc(sizeof(*boundary));
    boundary->x = min_pixel.x;
    boundary->y = max_pixel.y;
    boundary->w = max_pixel.x - min_pixel.x;

    boundary->h = min_pixel.y - max_pixel.y;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, boundary);
    free(boundary);

    SDL_RenderPresent(renderer);
}

void sdl_render_scene(scene_t *scene) {
    sdl_clear();
    size_t body_count = scene_bodies(scene);
    for (size_t i = 0; i < body_count; i++) {
        body_t *body = scene_get_body(scene, i);
        sprite_info_t body_sprite = body_get_sprite(body);
        bool is_sprite = body_sprite.is_sprite;
        char *img_path = body_sprite.img_path;
        if (is_sprite && img_path != NULL) {
            sdl_draw_sprite(body);
        } else if (!is_sprite) {
            list_t *points = body_get_shape(body);
            rgb_color_t color = body_sprite.color;
            sdl_draw_polygon(points, color);
        }
    }
}

void sdl_on_key(key_handler_t handler) { key_handler = handler; }

void sdl_on_mouse(mouse_handler_t handler) { mouse_handler = handler; }

double time_since_last_tick(void) {
    clock_t now = clock();
    double difference = last_clock
                            ? (double)(now - last_clock) / CLOCKS_PER_SEC
                            : 0.0; // return 0 the first time this is called
    last_clock = now;
    return difference;
}
