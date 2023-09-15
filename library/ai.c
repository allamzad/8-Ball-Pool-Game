#include "ai.h"
#include "collision.h"
#include "graphics.h"
#include "ids.h"
#include "pool_table.h"
#include "shape_utility.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const double MAX_HIT_IMPULSE = 1000;
const double HIT_MOMENTUM = 500;
const double ANGLE_THRESH = M_PI / 2.4; // maximum angle a ball can be deflected
const double RADIUS_OFFSET = 1;         // offset for collision errors...
const double COLLISION_EXEMPTION = 10;  // distance to target that isn't checked

const vector_t CUEBALL_UP_MIN = {200, 100};
const vector_t CUEBALL_UP_RANGE = {600, 300};
const double CUEBALL_TARGET_DIST = 5;

void ai_random_move(scene_t *scene) {
    assert(cueball_in_play(scene));
    body_t *cueball = get_cueball_body(scene);
    srand((size_t)time(NULL));
    double phi = ((double)rand() / (double)(RAND_MAX)) * 2 * M_PI;
    vector_t dir = vec_rotate((vector_t){1, 0}, phi);
    double mag = ((double)rand() / (double)(RAND_MAX)) * MAX_HIT_IMPULSE;
    body_add_impulse(cueball, vec_multiply(mag, dir));
}

/**
 * Finds friendly and enermy balls.
 *
 * @param scene the scene of the pool table
 * @param side 0=solids, 1=stripes, 2=undecided
 * @param my_balls the list where to store friendly balls
 * @param enemy_balls the list where to store enemy balls
 *
 * NOTE: in case all your 7 balls have already been pocketed,
 * the function will return the 8-ball as friendly. Otherwise,
 * the 8-ball will be tagged as enemy.
 */
void find_balls(scene_t *scene, size_t side,
                list_t *my_balls, list_t *enemy_balls) {
    size_t num_bodies = scene_bodies(scene);
    size_t enemy_id, friendly_id;
    body_t *eightball = NULL;
    if (side == 0) {
        friendly_id = SOLID_BALL_ID;
        enemy_id = STRIPED_BALL_ID;
    } 
    else if (side == 1) {
        friendly_id = STRIPED_BALL_ID;
        enemy_id = SOLID_BALL_ID;
    }
    for (size_t i = 0; i < num_bodies; i++) {
        body_t *b = scene_get_body(scene, i);
        size_t b_id = body_id(b);
        if (b_id == EIGHTBALL_ID) {
            eightball = b;
        } 
        else if (side == 2) { // undecided
            if (b_id == SOLID_BALL_ID || b_id == STRIPED_BALL_ID) {
                list_add(my_balls, b);
            }
        } 
        else {
            if (b_id == friendly_id) {
                list_add(my_balls, b);
            } 
            else if (b_id == enemy_id) {
                list_add(enemy_balls, b);
            }
        }
    }
    if (list_size(my_balls) == 0 && eightball != NULL) {
        list_add(my_balls, eightball);
    } 
    else if (list_size(my_balls) > 0 && eightball != NULL) {
        list_add(enemy_balls, eightball);
    }
    list_shuffle(my_balls);
}

/**
 * Checks if the direct path from cue to target hits obstacle
 *
 * @param cue the cueball body
 * @param target the target ball body
 * @param obstacle the obstacle ball body
 *
 * @return if path does hit obstacle, false otherwise.
 */
bool path_hits_ball(body_t *cue, body_t *target, body_t *obstacle) {
    const double BALL_RADIUS = get_ball_radius();
    vector_t c = body_get_centroid(cue);
    vector_t t = body_get_centroid(target);
    vector_t r = body_get_centroid(obstacle);
    vector_t ct = vec_subtract(t, c);
    vector_t d = vec_unit(ct);
    vector_t p = vec_unit(vec_rotate(d, M_PI / 2));
    vector_t cr = vec_subtract(r, c);
    double alpha = vec_dot(d, cr);
    double beta = vec_dot(p, cr);
    if (beta < 0) {
        beta = -beta;
    }
    if (0 <= alpha && alpha <= vec_magnitude(ct) && beta <= BALL_RADIUS * 2) {
        // alpha = distance from cue to obstacle along the
        //         direction from cue to target
        // |beta| = distance from obstacle to the line between cue and target
        return true;
    }
    return false;
}

/**
 * Checks if there is a clear centered shot from the cue to the target ball.
 *
 * @param scene the scene of the pool table
 * @param cue the cueball body
 * @param target the target ball body's index in my_balls
 * @param my_balls list of friendly balls (can include target)
 * @param enemy_balls list of enemy balls
 *
 * @return if yes, then the target body is returned; otherwise NULL is returned.
 */
body_t *clear_direct_shot(scene_t *scene, body_t *cue,
                          size_t target, list_t *my_balls, list_t *enemy_balls) {
    for (size_t i = 0; i < list_size(my_balls); i++) {
        if (i != target && path_hits_ball(cue, list_get(my_balls, target),
                                          list_get(my_balls, i))) {
            return NULL;
        }
    }
    for (size_t i = 0; i < list_size(enemy_balls); i++) {
        if (path_hits_ball(cue, list_get(my_balls, target),
                           list_get(enemy_balls, i))) {
            return NULL;
        }
    }
    return list_get(my_balls, target);
}

bool segments_intersect(vector_t s, vector_t sdir, vector_t a, vector_t adir) {
    if (vec_dot(sdir, adir) == 0) { // parallel segments
        return false;
    }
    vector_t adir_perp = vec_unit(vec_rotate(adir, M_PI / 2));
    vector_t sdir_perp = vec_unit(vec_rotate(sdir, M_PI / 2));
    double alpha = vec_dot(vec_subtract(a, s), adir_perp) / vec_dot(sdir, adir_perp);
    double beta = vec_dot(vec_subtract(s, a), sdir_perp) / vec_dot(adir, sdir_perp);
    return (0 <= alpha && alpha <= vec_magnitude(sdir) &&
            0 <= beta && beta <= vec_magnitude(adir));
}

bool segment_intersects_polygon(vector_t s, vector_t dir, list_t *shape) {
    size_t n_points = list_size(shape);
    for (size_t i = 0; i < n_points; i++) {
        vector_t a = *((vector_t *)list_get(shape, i));
        vector_t b = *((vector_t *)list_get(shape, i));
        if (segments_intersect(s, dir, a, vec_subtract(b, a))) {
            return true;
        }
    }
    return false;
}

vector_t *ampersand(vector_t v) {
    vector_t *ans = malloc(sizeof(vector_t));
    *ans = v;
    return ans;
}

vector_t shorten_vector(vector_t v, double dl) {
    double mag = vec_magnitude(v);
    v = vec_unit(v);
    if (mag > dl) {
        return vec_multiply(mag - dl, v);
    }
    return vec_multiply(mag, v);
}

bool clear_path(body_t *ball, vector_t dir, list_t *obs) {
    const double BALL_RADIUS = get_ball_radius();
    vector_t perp = vec_unit(vec_rotate(dir, M_PI / 2));
    vector_t c = body_get_centroid(ball);
    vector_t a = vec_add(c, vec_multiply(BALL_RADIUS, perp));
    vector_t b = vec_add(c, vec_multiply(-BALL_RADIUS, perp));

    list_t *path_rectangle = list_init(4, free); // 4 cuz it's a rectangle
    list_add(path_rectangle, ampersand(b));      // this order gurantees ccw order
    dir = shorten_vector(dir, COLLISION_EXEMPTION);
    list_add(path_rectangle, ampersand(vec_add(b, dir)));
    list_add(path_rectangle, ampersand(vec_add(a, dir)));
    list_add(path_rectangle, ampersand(a));
    for (size_t i = 0; i < list_size(obs); i++) {
        list_t *obstacle = body_get_shape(list_get(obs, i));
        if (find_collision(path_rectangle, obstacle).collided) {
            list_free(path_rectangle);
            return false;
        }
    }
    list_free(path_rectangle);
    return true;
}

/**
 * Chekcs if there is a clear shot into selected pocket.
 *
 * @param cue the cueball body
 * @param target the target ball body
 * @param pocket the target pocket baody
 * @param obs the list of obstacle bodies (should not include cue and target)
 *
 * @return if true, then the hit direction is given, otherwise returns NULL
 *
 * NOTE: the output will have to be deallocated if not NULL!
 */
vector_t *clear_pocket_shot(body_t *cue, body_t *target, body_t *pocket, list_t *obs) {
    const double BALL_RADIUS = get_ball_radius() - RADIUS_OFFSET;
    vector_t c = body_get_centroid(cue);
    vector_t t = body_get_centroid(target);
    vector_t p = body_get_centroid(pocket);
    vector_t tp = vec_subtract(p, t);
    vector_t ct = vec_subtract(t, c);
    if (vec_dot(vec_unit(ct), vec_unit(tp)) <= cos(ANGLE_THRESH)) {
        return NULL; // the ball can't be 'turned' around...
    }
    vector_t contact_pos = vec_add(t, vec_multiply(-2 * BALL_RADIUS, vec_unit(tp)));
    vector_t dir = vec_subtract(contact_pos, c); // dir we'd want to hit the cueball
    if (clear_path(cue, dir, obs) && clear_path(target, tp, obs)) {
        vector_t *ans = malloc(sizeof(vector_t));
        assert(ans != NULL);
        *ans = vec_unit(dir);
        return ans;
    }
    return NULL;
}

/**
 * Checks if there is a clear shot to pocket a ball in any pocket.
 *
 * @param scene the scene of the pool table
 * @param cue the cueball body
 * @param target the target ball body
 *
 * @return if true, then a hit direction is given, othewise returns VEC_ZERO
 * NOTE: the output will have to be deallocated if not NULL!
 */
vector_t *clear_pocketing_shot(scene_t *scene, body_t *cue, body_t *target) {
    list_t *pockets = list_init(6, NULL);
    list_t *obstacles = list_init(20, NULL);
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *b = scene_get_body(scene, i);
        if (b == cue || b == target) {
            continue;
        }
        size_t b_id = body_id(b);
        if (b_id == WALL_ID || is_ball(b)) {
            list_add(obstacles, b);
        } 
        else if (b_id == POCKET_ID) {
            list_add(pockets, b);
        }
    }
    vector_t *dir = NULL;
    for (size_t p = 0; p < list_size(pockets) && dir == NULL; p++) {
        dir = clear_pocket_shot(cue, target, list_get(pockets, p), obstacles);
    }
    return dir;
}

/**
 * Execute a shot.
 *
 * @param cue the cueball body
 * @param target position vector of the body/place to hit
 * @param mag the magnitude of momentum to be applied to cueball.
 */
void execute_shot(body_t *cue, vector_t target, double mag) {
    vector_t dir = vec_unit(vec_subtract(target, body_get_centroid(cue)));
    body_add_impulse(cue, vec_multiply(mag, dir));
}

void ai_easy_make_move(scene_t *scene, size_t side) {
    assert(cueball_in_play(scene));

    body_t *cueball = get_cueball_body(scene);
    list_t *my_balls = list_init(7, NULL);    // 7 = # of striped/solid balls
    list_t *enemy_balls = list_init(8, NULL); // 8 cuz eightball is enemy
    find_balls(scene, side, my_balls, enemy_balls);
    body_t *target = NULL;
    for (size_t i = 0; i < list_size(my_balls) && target == NULL; i++) {
        target = clear_direct_shot(scene, cueball, i, my_balls, enemy_balls);
    }
    if (target == NULL) {      // no clear shot...
        ai_random_move(scene); // just do something...
    } 
    else {
        execute_shot(cueball, body_get_centroid(target), HIT_MOMENTUM);
    }
}

void ai_medium_make_move(scene_t *scene, size_t side) {
    assert(cueball_in_play(scene));
    body_t *cueball = get_cueball_body(scene);
    list_t *my_balls = list_init(7, NULL);    // 7 = # of striped/solid balls
    list_t *enemy_balls = list_init(8, NULL); // 8 cuz eightball is enemy
    find_balls(scene, side, my_balls, enemy_balls);
    vector_t *dir = NULL;
    for (size_t i = 0; i < list_size(my_balls) && dir == NULL; i++) {
        dir = clear_pocketing_shot(scene, cueball, list_get(my_balls, i));
    }
    if (dir == NULL) { // no clear pocketing shot...
        ai_easy_make_move(scene, side);
    } 
    else {
        body_add_impulse(cueball, vec_multiply(HIT_MOMENTUM, *dir));
        free(dir);
    }
}

bool cueball_ok(scene_t *scene, list_t *tentative_cue) {
    bool collided = false;
    size_t bodies = scene_bodies(scene);
    for (size_t i = 0; i < bodies && !collided; i++) {
        body_t *body = scene_get_body(scene, i);
        collided = (collided || find_collision(body_get_shape(body),
                    tentative_cue).collided);
    }
    return !collided;
}

list_t *ai_easy_put_cue(scene_t *scene, size_t side) {
    srand(time(NULL));
    while (true) {
        double rand_x = rand() % (int32_t)CUEBALL_UP_RANGE.x + CUEBALL_UP_MIN.x;
        double rand_y = rand() % (int32_t)CUEBALL_UP_RANGE.y + CUEBALL_UP_MIN.y;
        list_t *tentative_cue = generate_ball(rand_x, rand_y,
                                              get_ball_radius());
        // so cueball position is valid, didn't collide
        if (cueball_ok(scene, tentative_cue)) {
            return tentative_cue;
        } 
        else {
            list_free(tentative_cue); // retry
        }
    }
}

list_t *ai_medium_put_cue(scene_t *scene, size_t side) {
    list_t *my_balls = list_init(7, NULL);    // 7 = # of striped/solid balls
    list_t *enemy_balls = list_init(8, NULL); // 8 cuz eightball is enemy
    find_balls(scene, side, my_balls, enemy_balls);
    list_t *pockets = list_init(6, NULL);
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        if (body_id(body) == POCKET_ID) {
            list_add(pockets, body);
        }
    }

    for (size_t j = 0; j < list_size(my_balls); j++) {
        list_t *obstacles = list_init(20, NULL);
        body_t *target = list_get(my_balls, j);
        for (size_t i = 0; i < scene_bodies(scene); i++) {
            body_t *b = scene_get_body(scene, i);
            if (b == target) {
                continue;
            }
            size_t b_id = body_id(b);
            if (b_id == WALL_ID || is_ball(b)) {
                list_add(obstacles, b);
            }
        }

        for (size_t i = 0; i < list_size(pockets); i++) {
            body_t *pocket = list_get(pockets, i);
            vector_t dir = vec_subtract(body_get_centroid(pocket),
                                        body_get_centroid(target));
            if (!clear_path(target, dir, obstacles)) {
                continue;
            }
            vector_t dir_hat = vec_unit(dir);
            vector_t cue_centroid = vec_subtract(body_get_centroid(target),
                     vec_multiply(2 * get_ball_radius() + CUEBALL_TARGET_DIST, dir_hat));
            list_t *tentative_cue = generate_ball(cue_centroid.x, cue_centroid.y,
                                                  get_ball_radius());
            if (cueball_ok(scene, tentative_cue)) {
                list_free(obstacles);
                list_free(my_balls);
                list_free(enemy_balls);
                list_free(pockets);
                return tentative_cue;
            } 
            else {
                list_free(tentative_cue); // retry
            }
        }
        list_free(obstacles);
    }
    list_free(my_balls);
    list_free(enemy_balls);
    list_free(pockets);
    return ai_easy_put_cue(scene, side); // summon the dumb dumb
}