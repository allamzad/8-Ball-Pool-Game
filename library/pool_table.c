#include "pool_table.h"
#include "forces.h"
#include "graphics.h"
#include "ids.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "shape_utility.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Ball constants
const double BALL_MASS = 1;
const size_t NUM_STRIPED_BALLS = 7;
const size_t NUM_SOLID_BALLS = 7;
const double POCKET_RADIUS = 2;
const size_t MAX_EXTRA_BALLS = 3;

const double DRAG_COEFF = 0.3;
const double CONST_DRAG = 0.2;
// elasticity coeff for ball-ball collision
const double BALL_BALL_CR = 0.9;
// scales ball-ball collision elasticity for chaos mode.
const double CHAOS_BALL_BALL_SCALER = 0.5;
// elasiticty coeff for
const double WALL_BALL_CR = 0.8;
const double STICK_MASS = 1;

// the threshold at which we say a ball stopped
const double BALL_ZERO_THRESH = 0.05;

// Volume constants
const double DEFAULT_VOLUME = 128;
const double HIGH_VELOCITY = 200;

// Power bar constants
const vector_t POWER_BAR_START = (vector_t){.x = 925, .y = 125};
const double POWER_BAR_HEIGHT = 200;
const double POWER_BAR_WIDTH = 50;
const size_t POWER_BAR_OUTSIDE_GAP = 7;
const double POWER_BAR_MASS = 100;
const double POWER_BAR_INIT_CHARGE = 0.1;

// Power-up constants
const double POWER_UP_RADIUS = 10;
const double POWER_UP_MASS = 100;
const vector_t POWER_UP_MIN = {200, 100};
const vector_t POWER_UP_RANGE = {600, 300};

void shuffle(size_t *array, size_t n) {
    for (size_t i = 0; i < n; i++) {
        size_t j = rand() % (n);
        size_t t = array[j];
        array[j] = array[i];
        array[i] = t;
    }
}

size_t *generate_ID(size_t object_id) {
    size_t *object_ID = malloc(sizeof(size_t));
    assert(object_ID != NULL);
    *object_ID = object_id;
    return object_ID;
}

void generate_striped_balls(scene_t *scene, list_t *img_list, list_t *rack_pos) {
    for (size_t i = 0; i < NUM_STRIPED_BALLS; i++) {
        size_t *ball_ID = generate_ID(STRIPED_BALL_ID);
        vector_t position = *(vector_t *)list_get(rack_pos, 2 * i + 1);
        list_t *stripedball_shape = generate_ball(position.x, position.y,
                                                  get_ball_radius());
        sprite_info_t stripedball_sprite = striped_balls_textures(img_list,
                                                                  rack_pos, i);
        body_t *my_ball =
            body_init_with_info(stripedball_shape, stripedball_sprite, BALL_MASS,
                                ball_ID, free);
        body_set_velocity(my_ball, VEC_ZERO);
        scene_add_body(scene, my_ball);
    }
}

void generate_solid_balls(scene_t *scene, list_t *img_list, list_t *rack_pos) {
    for (size_t i = 0; i < NUM_SOLID_BALLS; i++) {
        size_t *ball_ID = generate_ID(SOLID_BALL_ID);
        vector_t position = *(vector_t *)list_get(rack_pos, 2 * i);
        list_t *solidball_shape = generate_ball(position.x, position.y,
                                                get_ball_radius());
        sprite_info_t solidball_sprite = solid_balls_textures(img_list, rack_pos, i);
        body_t *my_ball =
            body_init_with_info(solidball_shape, solidball_sprite, BALL_MASS,
                                ball_ID, free);
        body_set_velocity(my_ball, VEC_ZERO);
        scene_add_body(scene, my_ball);
    }
}

void generate_eightball(scene_t *scene) {
    size_t *ball_ID = generate_ID(EIGHTBALL_ID);
    vector_t eightball_pos = get_eightball_init_pos();
    list_t *eightball_shape = generate_ball(eightball_pos.x, eightball_pos.y,
                                            get_ball_radius());
    sprite_info_t eightball_sprite = eightball_texture();
    body_t *my_ball =
        body_init_with_info(eightball_shape, eightball_sprite, BALL_MASS,
                            ball_ID, free);
    body_set_velocity(my_ball, VEC_ZERO);
    scene_add_body(scene, my_ball);
}

void ball_collision_sound(body_t *body1, body_t *body2, vector_t axis, void *aux) {
    double vel_mag = vec_magnitude(vec_subtract(body_get_velocity(body1),
                                                body_get_velocity(body2)));
    double volume = DEFAULT_VOLUME * vel_mag / HIGH_VELOCITY;
    sdl_play_sound_effect("assets/ballhitball.wav", volume);
}

void poolstick_hit_cue_sound(body_t *body1, body_t *body2, vector_t axis, void *aux) {
    double vel_mag = vec_magnitude(vec_subtract(body_get_velocity(body1),
                                                body_get_velocity(body2)));
    double volume = DEFAULT_VOLUME * vel_mag / HIGH_VELOCITY;
    sdl_play_sound_effect("assets/poolstickhitcue.wav", volume);
}

double get_cr(bool chaos) {
    if (chaos) {
        return BALL_BALL_CR * CHAOS_BALL_BALL_SCALER;
    }
    return BALL_BALL_CR;
}

void ball_in_power_up(body_t *body1, body_t *body2, vector_t axis, void *aux) {
    body_remove(body1); // we destroy the powerup
    double volume = DEFAULT_VOLUME / 2;
    sdl_play_sound_effect("assets/powerup.wav", volume);
}

void put_cueball(scene_t *scene, list_t *shape, bool chaos, bool powerup) {
    size_t *ball_ID = generate_ID(CUEBALL_ID);
    sprite_info_t cueball_sprite = cueball_texture(polygon_centroid(shape));
    body_t *my_ball =
        body_init_with_info(shape, cueball_sprite, BALL_MASS, ball_ID, free);
    body_set_velocity(my_ball, VEC_ZERO);
    scene_add_body(scene, my_ball);

    size_t bodies = scene_bodies(scene);
    size_t body_indexes[NUM_SOLID_BALLS + NUM_STRIPED_BALLS + MAX_EXTRA_BALLS];
    size_t body_indexes2[NUM_SOLID_BALLS + NUM_STRIPED_BALLS + MAX_EXTRA_BALLS];
    size_t k = 0;
    for (size_t i = 0; i < bodies; i++) {
        body_t *body = scene_get_body(scene, i);
        if (body != my_ball) {
            size_t info = *((size_t *)body_get_info(body));
            if (is_ball(body)) { // ball-ball
                create_physics_collision(scene, get_cr(chaos), my_ball, body);
                create_collision(scene, my_ball, body,
                                 (collision_handler_t)ball_collision_sound, NULL, NULL);
                body_indexes[k] = i;
                body_indexes2[k] = i;
                k++;
            }
            // Wall-ball
            if (info == WALL_ID) {
                create_physics_collision(scene, WALL_BALL_CR, my_ball, body);
                create_collision(scene, my_ball, body,
                                 (collision_handler_t)ball_collision_sound, NULL, NULL);
            }
            // Pocketing collisions
            if (info == POCKET_ID) {
                create_collision(scene, my_ball, body,
                                 (collision_handler_t)ball_in_pocket, NULL, NULL);
            }
            // Powerup "collision": just removes the powerup
            if (info == POWER_UP_ID) {
                create_collision(scene, body, my_ball,
                                 (collision_handler_t)ball_in_power_up, NULL, NULL);
            }
        }
    }

    if (chaos) {
        shuffle(body_indexes2, k);
        for (size_t j = 0; j < k; j++) {
            if (body_indexes[j] != body_indexes2[j]) {
                body_t *bodyB = scene_get_body(scene, body_indexes[j]);
                body_t *bodyC = scene_get_body(scene, body_indexes2[j]);
                create_chaos_physics_collision(scene, get_cr(chaos),
                                               my_ball, bodyB, bodyC);
            }
        }
    }

    create_drag(scene, DRAG_COEFF, my_ball);
    create_constant_drag_force(scene, CONST_DRAG, my_ball);
}

void generate_cueball(scene_t *scene) {
    size_t *ball_ID = generate_ID(CUEBALL_ID);
    vector_t cueball_pos = get_cueball_init_pos();
    list_t *cueball_shape = generate_ball(cueball_pos.x, cueball_pos.y,
                                          get_ball_radius());
    sprite_info_t cueball_sprite = cueball_texture(get_cueball_init_pos());
    body_t *my_ball =
        body_init_with_info(cueball_shape, cueball_sprite, BALL_MASS, ball_ID, free);
    body_set_velocity(my_ball, VEC_ZERO);
    scene_add_body(scene, my_ball);
}

/* Generates the top wall of the pool table and overlays the entire pooltable sprite. */
void generate_table_top(scene_t *scene) {
    size_t *wall_ID = generate_ID(WALL_ID);
    list_t *table_top_shape = generate_table_top_shape();
    sprite_info_t table_top_sprite = table_top_texture();
    body_t *my_table = body_init_with_info(table_top_shape, table_top_sprite,
                                           INFINITY, wall_ID, free);
    scene_add_body(scene, my_table);
}

void generate_table_left(scene_t *scene) {
    size_t *wall_ID = generate_ID(WALL_ID);
    list_t *table_left_shape = generate_table_left_shape();
    sprite_info_t wall_sprite = wall_texture();
    body_t *my_table = body_init_with_info(table_left_shape, wall_sprite,
                                           INFINITY, wall_ID, free);
    scene_add_body(scene, my_table);
}

void generate_table_right(scene_t *scene) {
    size_t *wall_ID = generate_ID(WALL_ID);
    list_t *table_right_shape = generate_table_right_shape();
    sprite_info_t wall_sprite = wall_texture();
    body_t *my_table = body_init_with_info(table_right_shape, wall_sprite,
                                           INFINITY, wall_ID, free);
    scene_add_body(scene, my_table);
}

void generate_table_bottom(scene_t *scene) {
    size_t *wall_ID = generate_ID(WALL_ID);
    list_t *table_bottom_shape = generate_table_bottom_shape();
    sprite_info_t wall_sprite = wall_texture();
    body_t *my_table = body_init_with_info(table_bottom_shape, wall_sprite,
                                           INFINITY, wall_ID, free);
    scene_add_body(scene, my_table);
}

void generate_table_body(scene_t *scene) {
    generate_table_left(scene);
    generate_table_right(scene);
    generate_table_bottom(scene);
    generate_table_top(scene);
}

void generate_pocket(scene_t *scene, double x, double y, double radius) {
    size_t *pocket_ID = generate_ID(POCKET_ID);
    list_t *pocket_shape = generate_ball(x, y, radius);
    sprite_info_t pocket_sprite = pocket_texture();
    body_t *my_pocket =
        body_init_with_info(pocket_shape, pocket_sprite, INFINITY,
                            pocket_ID, free);
    scene_add_body(scene, my_pocket);
}

void generate_all_pockets(scene_t *scene) {
    list_t *pocket_locations = generate_pocket_locations();
    for (size_t i = 0; i < get_num_pockets(); i++) {
        vector_t pocket_location = *(vector_t *)list_get(pocket_locations, i);
        generate_pocket(scene, pocket_location.x, pocket_location.y, POCKET_RADIUS);
    }
    list_free(pocket_locations);
}

/* Generates all the balls */
void generate_ball_rack(scene_t *scene) {
    list_t *rack_pos = generate_rack_pos();
    list_t *img_list = generate_ball_img_list();

    generate_striped_balls(scene, img_list, rack_pos);
    generate_solid_balls(scene, img_list, rack_pos);
    generate_eightball(scene);
    generate_cueball(scene);
    list_free(rack_pos);
}

/* Generates all the table objects */
void generate_table(scene_t *scene) {
    generate_table_body(scene);
    generate_all_pockets(scene);
}

bool balls_stopped(scene_t *scene) {
    if (get_poolstick_body(scene) != NULL) {
        return false;
    }
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *my_body = scene_get_body(scene, i);
        if (is_ball(my_body)) {
            if (fabs(body_get_velocity(my_body).x) > BALL_ZERO_THRESH ||
                fabs(body_get_velocity(my_body).y) > BALL_ZERO_THRESH) {
                return false;
            }
        }
    }
    return true;
}

bool is_ball(body_t *body) {
    void *info = body_get_info(body);
    if (*((size_t *)info) == STRIPED_BALL_ID) {
        return true;
    } else if (*((size_t *)info) == SOLID_BALL_ID) {
        return true;
    } else if (*((size_t *)info) == EIGHTBALL_ID) {
        return true;
    } else if (*((size_t *)info) == CUEBALL_ID) {
        return true;
    }
    return false;
}

void ball_in_pocket(body_t *body1, body_t *body2, vector_t axis, void *aux) {
    body_remove(body1); // we destroy the ball
    double volume = DEFAULT_VOLUME;
    sdl_play_sound_effect("assets/ballinpocket.wav", volume);
}

bool is_cueball(body_t *body) {
    void *info = body_get_info(body);
    return (*((size_t *)info) == CUEBALL_ID);
}

bool is_powerup(body_t *body) {
    void *info = body_get_info(body);
    return (*((size_t *)info) == POWER_UP_ID);
}

void add_collisions(scene_t *scene, bool chaos, bool powerup) {
    size_t body_count = scene_bodies(scene);

    // Adding the drag forces
    for (size_t i = 0; i < body_count; i++) {
        body_t *body = scene_get_body(scene, i);
        if (is_ball(body)) {
            create_drag(scene, DRAG_COEFF, body);
            create_constant_drag_force(scene, CONST_DRAG, body);
        }
    }
    double ball_ball = get_cr(chaos);

    // Adding collisions
    size_t extra_balls = 2; // cueball and eightball
    if (powerup) {
        extra_balls++;
    }
    size_t body_indexes[NUM_SOLID_BALLS + NUM_STRIPED_BALLS + extra_balls];
    size_t body_indexes2[NUM_SOLID_BALLS + NUM_STRIPED_BALLS + extra_balls];
    size_t k = 0;
    for (size_t i = 1; i < body_count; i++) {
        body_t *body1 = scene_get_body(scene, i);
        if (is_ball(body1) || is_powerup(body1)) {
            body_indexes[k] = i;
            body_indexes2[k] = i;
            k++;
        }
        for (size_t j = 0; j < i; j++) {
            body_t *body2 = scene_get_body(scene, j);
            size_t info1 = *((size_t *)body_get_info(body1));
            size_t info2 = *((size_t *)body_get_info(body2));
            if (is_ball(body1) && is_ball(body2)) { // ball-ball
                create_physics_collision(scene, ball_ball, body1, body2);
                create_collision(scene, body1, body2,
                                 (collision_handler_t)ball_collision_sound, NULL, NULL);
            }
            // Wall-ball
            if ((is_ball(body1) && info2 == WALL_ID) ||
                (info1 == WALL_ID && is_ball(body2))) {
                create_physics_collision(scene, WALL_BALL_CR, body1, body2);
                create_collision(scene, body1, body2,
                                 (collision_handler_t)ball_collision_sound, NULL, NULL);
            }
            // Pocketing collisions
            if (info1 == POCKET_ID && is_ball(body2)) {
                create_collision(scene, body2, body1,
                                 (collision_handler_t)ball_in_pocket, NULL, NULL);
            }
            if (info2 == POCKET_ID && is_ball(body1)) {
                create_collision(scene, body1, body2,
                                 (collision_handler_t)ball_in_pocket, NULL, NULL);
            }
            // Powerup "collision": just removes the powerup
            if (info1 == POWER_UP_ID && is_cueball(body2)) {
                create_collision(scene, body1, body2,
                                 (collision_handler_t)ball_in_power_up, NULL, NULL);
            }
            if (info2 == POWER_UP_ID && is_cueball(body1)) {
                create_collision(scene, body2, body1,
                                 (collision_handler_t)ball_in_power_up, NULL, NULL);
            }
        }
    }

    // Chaos mode setup between balls
    if (chaos) {
        shuffle(body_indexes2, k);
        for (size_t i = 0; i < k; i++) {
            for (size_t j = 0; j < k; j++) {
                if (i != j) {
                    body_t *bodyA = scene_get_body(scene, body_indexes[i]);
                    body_t *bodyB = scene_get_body(scene, body_indexes[j]);
                    body_t *bodyC;
                    if (i < j) {
                        bodyC = scene_get_body(scene, body_indexes2[i]);
                    } else {
                        bodyC = scene_get_body(scene, body_indexes2[j]);
                    }
                    if (body_indexes[i] != body_indexes2[i]) {
                        create_chaos_physics_collision(scene, ball_ball,
                                                       bodyA, bodyB, bodyC);
                    }
                }
            }
        }
    }
}

vector_t random_spot(scene_t *scene) {
    srand(time(NULL));
    bool keep_searching = true;
    while (keep_searching) {
        double rand_x = rand() % (int32_t)POWER_UP_RANGE.x + POWER_UP_MIN.x;
        double rand_y = rand() % (int32_t)POWER_UP_RANGE.y + POWER_UP_MIN.y;
        vector_t candidate = {rand_x, rand_y};
        keep_searching = false;
        for (int i = 0; i < scene_bodies(scene); i++) {
            body_t *obstacle = scene_get_body(scene, i);
            double dist = vec_magnitude(vec_subtract(body_get_centroid(obstacle),
                                                     candidate));
            if (*(size_t *)body_get_info(obstacle) != POWER_UP_ID) {
                if (dist < 2 * POWER_UP_RADIUS) {
                    keep_searching = true;
                }
            }
        }
        if (!keep_searching) {
            return candidate;
        }
    }
    return VEC_ZERO;
}

void generate_pool_table(scene_t *scene, bool chaos, bool powerup) {
    generate_table(scene);
    generate_ball_rack(scene);
    if (powerup) {
        vector_t random_position = random_spot(scene);
        generate_power_up(scene, random_position);
    }
    add_collisions(scene, chaos, powerup);
}

void generate_poolstick(scene_t *scene, vector_t my_position,
                        vector_t cueball_position) {
    size_t *poolstick_ID = generate_ID(POOLSTICK_ID);

    list_t *shape = poolstick_shape(my_position, cueball_position);
    sprite_info_t sprite = poolstick_texture();

    body_t *my_poolstick =
        body_init_with_info(shape, sprite, STICK_MASS, poolstick_ID, free);

    setup_ball_stick_collisions(my_poolstick, scene, 0.8);
    scene_add_body(scene, my_poolstick);
}

void setup_ball_stick_collisions(body_t *stick, scene_t *scene, double elasticity) {
    body_t *my_body = get_cueball_body(scene);
    create_destructive_physics_collision(scene, elasticity, my_body, stick);
    create_collision(scene, my_body, stick,
                     (collision_handler_t)poolstick_hit_cue_sound, NULL, NULL);
}

size_t count_balls(scene_t *scene, size_t searched_id) {
    size_t ans = 0;
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *my_body = scene_get_body(scene, i);
        if (*(size_t *)body_get_info(my_body) == searched_id) {
            ans++;
        }
    }
    return ans;
}

size_t striped_count(scene_t *scene) {
    return count_balls(scene, STRIPED_BALL_ID);
}

size_t solid_count(scene_t *scene) {
    return count_balls(scene, SOLID_BALL_ID);
}

bool eightball_in_play(scene_t *scene) {
    return count_balls(scene, EIGHTBALL_ID) > 0;
}

bool cueball_in_play(scene_t *scene) {
    return count_balls(scene, CUEBALL_ID) > 0;
}

bool powerup_triggered(scene_t *scene) {
    return count_balls(scene, POWER_UP_ID) == 0;
}

void generate_power_bar(scene_t *scene) {
    list_t *inside_shape = generate_rect_shape(POWER_BAR_START.x, POWER_BAR_START.y,
                                               POWER_BAR_WIDTH, POWER_BAR_HEIGHT);
    list_t *outside_shape = generate_rect_shape(POWER_BAR_START.x, POWER_BAR_START.y,
        POWER_BAR_WIDTH + POWER_BAR_OUTSIDE_GAP * 2,
        POWER_BAR_HEIGHT + POWER_BAR_OUTSIDE_GAP * 2);
    list_t *charge_shape = generate_rect_shape(POWER_BAR_START.x, POWER_BAR_START.y 
        - POWER_BAR_HEIGHT / 2 + POWER_BAR_HEIGHT * POWER_BAR_INIT_CHARGE / 2,
        POWER_BAR_WIDTH, POWER_BAR_HEIGHT * POWER_BAR_INIT_CHARGE);

    size_t *inside_shape_ID = malloc(sizeof(size_t));
    assert(inside_shape_ID != NULL);
    *inside_shape_ID = POWER_BAR_INSIDE_ID;

    size_t *outside_shape_ID = malloc(sizeof(size_t));
    assert(outside_shape_ID != NULL);
    *outside_shape_ID = POWER_BAR_OUTSIDE_ID;

    size_t *charge_shape_ID = malloc(sizeof(size_t));
    assert(charge_shape_ID != NULL);
    *charge_shape_ID = POWER_BAR_CHARGE_ID;

    sprite_info_t inside_sprite = powerbar_inside_texture();
    sprite_info_t outside_sprite = powerbar_outside_texture();
    sprite_info_t charge_sprite = powerbar_charge_texture();

    body_t *inside_power_bar =
        body_init_with_info(inside_shape, inside_sprite, POWER_BAR_MASS,
                            inside_shape_ID, free);

    body_t *outside_power_bar =
        body_init_with_info(outside_shape, outside_sprite, POWER_BAR_MASS,
                            outside_shape_ID, free);

    body_t *charge_power_bar =
        body_init_with_info(charge_shape, charge_sprite, POWER_BAR_MASS,
                            charge_shape_ID, free);

    scene_add_body(scene, outside_power_bar);
    scene_add_body(scene, inside_power_bar);
    scene_add_body(scene, charge_power_bar);
}

double get_charge(scene_t *scene) {
    body_t *charge_body = get_power_bar_charge_body(scene);
    list_t *charge_shape = body_get_shape(charge_body);
    double charge = fabs(((*(vector_t *)list_get(charge_shape, 1)).y -
                          (*(vector_t *)list_get(charge_shape, 2)).y) /
                         POWER_BAR_HEIGHT);
    return charge;
}

void charge_power_bar(scene_t *scene, double charge_add) {
    double current_charge = get_charge(scene);
    body_t *power_bar = get_power_bar_charge_body(scene);

    double charge = current_charge + charge_add;
    if (charge >= 1.0) {
        charge = 1.0;
    }

    list_t *charge_shape = generate_rect_shape(POWER_BAR_START.x, POWER_BAR_START.y
     - POWER_BAR_HEIGHT / 2 + POWER_BAR_HEIGHT * charge / 2, POWER_BAR_WIDTH,
     POWER_BAR_HEIGHT * charge);
    body_set_shape(power_bar, charge_shape);
}

body_t *get_specified_body(scene_t *scene, size_t id) {
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *my_body = scene_get_body(scene, i);
        if (*(size_t *)body_get_info(my_body) == id) {
            return my_body;
        }
    }
    return NULL;
}

size_t body_id(body_t *body) {
    return *((size_t *)body_get_info(body));
}

body_t *get_cueball_body(scene_t *scene) {
    return get_specified_body(scene, CUEBALL_ID);
}

body_t *get_poolstick_body(scene_t *scene) {
    return get_specified_body(scene, POOLSTICK_ID);
}

body_t *get_power_bar_inside_body(scene_t *scene) {
    return get_specified_body(scene, POWER_BAR_INSIDE_ID);
}

body_t *get_power_bar_outside_body(scene_t *scene) {
    return get_specified_body(scene, POWER_BAR_OUTSIDE_ID);
}

body_t *get_power_bar_charge_body(scene_t *scene) {
    return get_specified_body(scene, POWER_BAR_CHARGE_ID);
}

void remove_power_bar(scene_t *scene) {
    body_remove(get_power_bar_inside_body(scene));
    body_remove(get_power_bar_outside_body(scene));
    body_remove(get_power_bar_charge_body(scene));
}

void destroy_poolstick(scene_t *scene) {
    body_remove(get_poolstick_body(scene));
}

void update_poolstick(scene_t *scene, vector_t my_position) {

    body_t *cueball = get_cueball_body(scene);
    body_t *poolstick = get_poolstick_body(scene);
    vector_t cue_point = body_get_centroid(cueball);

    if (poolstick == NULL) {
        generate_poolstick(scene, my_position, cue_point);
    } else {
        list_t *shape = body_get_deepcopied_shape(poolstick);
        vector_t angle_of_attack = vec_unit(vec_subtract(my_position, cue_point));
        vector_t new_centroid = vec_add(vec_multiply(vec_magnitude(
                POOLSTICK_DIMENSION) / 2, angle_of_attack), my_position);
        vector_t b = vec_unit(vec_subtract(*(vector_t *)list_get(shape, 0),
                                           *(vector_t *)list_get(shape, 1)));
        vector_t a = vec_multiply(-1, angle_of_attack);
        double angle = atan2(b.x * a.y - b.y * a.x, b.x * a.x + b.y * a.y);

        polygon_rotate(shape, angle, body_get_centroid(poolstick));
        body_set_shape(poolstick, shape);
        body_set_centroid(poolstick, new_centroid);
    }
}

void generate_power_up(scene_t *scene, vector_t position) {
    size_t *power_up_ID = generate_ID(POWER_UP_ID);

    list_t *shape = generate_ball(position.x, position.y, POWER_UP_RADIUS);
    sprite_info_t sprite = power_up_texture(position);

    body_t *my_power_up =
        body_init_with_info(shape, sprite, POWER_UP_MASS, power_up_ID, free);

    scene_add_body(scene, my_power_up);
}
