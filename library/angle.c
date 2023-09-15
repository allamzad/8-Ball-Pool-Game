#include "angle.h"
#include "body.h"
#include "collision.h"
#include "forces.h"
#include "graphics.h"
#include "ids.h"
#include "list.h"
#include "polygon.h"
#include "pool_table.h"
#include "scene.h"
#include "shape_utility.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const double LINE_WIDTH = 2;
const double LINE_MASS = 100;
const double GAP = 1;

list_t *line_shape(vector_t start_position, vector_t end_position, double width) {
    vector_t vector_of_attack = vec_unit(vec_subtract(end_position, start_position));
    vector_t orthogonal_vector_of_attack = (vector_t){vector_of_attack.y,
                                                      -vector_of_attack.x};

    list_t *ans = list_init(RECTANGLE_RESOLUTION, free);

    vector_t *line_point_vec = malloc(sizeof(vector_t));
    assert(line_point_vec != NULL);
    *line_point_vec = vec_add(vec_multiply(width / 2, orthogonal_vector_of_attack),
                              end_position);
    list_add(ans, line_point_vec);

    line_point_vec = malloc(sizeof(vector_t));
    assert(line_point_vec != NULL);
    *line_point_vec = vec_add(vec_multiply(width / 2, orthogonal_vector_of_attack),
                              start_position);
    list_add(ans, line_point_vec);

    line_point_vec = malloc(sizeof(vector_t));
    assert(line_point_vec != NULL);
    *line_point_vec = vec_subtract(start_position, vec_multiply(width / 2,
                                    orthogonal_vector_of_attack));
    list_add(ans, line_point_vec);

    line_point_vec = malloc(sizeof(vector_t));
    assert(line_point_vec != NULL);
    *line_point_vec = vec_subtract(end_position, vec_multiply(width / 2,
                                    orthogonal_vector_of_attack));
    list_add(ans, line_point_vec);

    return ans;
}

void destroy_angle_predictor(scene_t *scene) {
    body_remove(get_specified_body(scene, ANGLE_LINE_ID));
}

void generate_angle_predictor(scene_t *scene) {
    body_t *my_angle_line;

    body_t *poolstick = get_poolstick_body(scene);
    body_t *cueball = get_cueball_body(scene);
    vector_t vector_of_attack = vec_unit(vec_subtract(body_get_centroid(cueball),
                                                      body_get_centroid(poolstick)));

    vector_t start = vec_add(vec_multiply(BALL_RADIUS, vector_of_attack),
                             body_get_centroid(cueball));
    vector_t end = vec_add(vec_multiply(GAP, vector_of_attack), start);
    list_t *shape = line_shape(start, end, BALL_RADIUS * 2);
    bool exist = false;

    if (get_specified_body(scene, ANGLE_LINE_ID) != NULL) {
        my_angle_line = get_specified_body(scene, ANGLE_LINE_ID);
        body_set_shape(my_angle_line, shape);
        exist = true;
    } else {
        size_t *angle_ID = generate_ID(ANGLE_LINE_ID);
        sprite_info_t sprite = line_texture();
        my_angle_line = body_init_with_info(shape, sprite, LINE_MASS, angle_ID, free);
    }

    bool collision = false;

    while (!collision) {
        for (size_t i = 0; i < (size_t)scene_bodies(scene); i++) {
            body_t *body = scene_get_body(scene, i);
            if (*(size_t *)body_get_info(body) != CUEBALL_ID &&
                *(size_t *)body_get_info(body) != ANGLE_LINE_ID) {
                collision = find_collision(body_get_shape(my_angle_line),
                                           body_get_shape(body))
                                .collided;
                if (collision == true) {
                    break;
                }
            }
        }
        if (!collision) {
            end = vec_add(vec_multiply(GAP, vector_of_attack), end);
            shape = line_shape(start, end, BALL_RADIUS * 2);
            body_set_shape(my_angle_line, shape);
        }
    }

    shape = line_shape(start, end, LINE_WIDTH);
    body_set_shape(my_angle_line, shape);
    if (!exist) {
        scene_add_body(scene, my_angle_line);
    }
}
