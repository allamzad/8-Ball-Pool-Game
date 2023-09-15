#include "shape_utility.h"
#include "list.h"
#include "polygon.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const size_t BALL_RESOLUTION = 30;
const size_t RECT_RESOLUTION = 4;

/* Generates a postion vector. */
vector_t *gen_pos_vec(double x, double y)
{
    vector_t *position = malloc(sizeof(vector_t));
    assert(position != NULL);
    position->x = x;
    position->y = y;
    return position;
}

list_t *generate_ball(double x, double y, double radius)
{
    list_t *my_ball_shape = list_init(BALL_RESOLUTION, free);
    for (size_t point = 0; point < BALL_RESOLUTION; point++)
    {
        double angle = 2 * M_PI * point / BALL_RESOLUTION;
        list_add(my_ball_shape, gen_pos_vec(x + radius * cos(angle),
                                            y + radius * sin(angle)));
    }
    return my_ball_shape;
}

list_t *generate_rect_shape(double rec_x, double rec_y, double rec_width,
                            double rec_height)
{
    list_t *rec_shape = list_init(RECT_RESOLUTION, free);

    list_add(rec_shape, gen_pos_vec(rec_x + rec_width / 2, 
                                    rec_y + rec_height / 2));
    list_add(rec_shape, gen_pos_vec(rec_x + rec_width / 2 * -1, 
                                    rec_y + rec_height / 2));
    list_add(rec_shape, gen_pos_vec(rec_x + rec_width / 2 * -1, 
                                    rec_y + rec_height / 2 * -1));
    list_add(rec_shape, gen_pos_vec(rec_x + rec_width / 2, 
                                    rec_y + rec_height / 2 * -1));
    return rec_shape;
}