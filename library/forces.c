#include "forces.h"
#include "body.h"
#include "collision.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "pool_table.h"
#include "ids.h"

const double GRAVITY_THRESHOLD = 5;
const double ZERO = 0;
const double VEL_THRESH = 0.1;

typedef struct two_body_params {
  body_t *body1;
  body_t *body2;
  double constant;
} two_body_params_t;

void newtonian_gravity(two_body_params_t *aux) {
  vector_t r21 = vec_subtract(body_get_centroid(aux->body2),
                              body_get_centroid(aux->body1));
  double r21_mag = vec_magnitude(r21);
  if (r21_mag >= GRAVITY_THRESHOLD) {
    double m1 = body_get_mass(aux->body1);
    double m2 = body_get_mass(aux->body2);
    vector_t f21 =
        vec_multiply((-aux->constant) * m1 * m2 / pow(r21_mag, 3), r21);
    body_add_force(aux->body2, f21);
    body_add_force(aux->body1, vec_multiply(-1, f21));
  }
}

void create_newtonian_gravity(scene_t *scene, double G, body_t *body1,
                              body_t *body2) {
  two_body_params_t *aux = malloc(sizeof(two_body_params_t));
  assert(aux != NULL);
  *aux = (two_body_params_t){body1, body2, G};
  // scene_add_force_creator(scene, (force_creator_t)newtonian_gravity, aux,
  // free);
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, (force_creator_t)newtonian_gravity,
                                 aux, bodies, free);
}

//------------------------------------------------------------------------------

void elastic_force(two_body_params_t *aux) {
  vector_t r21 = vec_subtract(body_get_centroid(aux->body2),
                              body_get_centroid(aux->body1));
  body_add_force(aux->body2, vec_multiply(-(aux->constant), r21));
  body_add_force(aux->body1, vec_multiply(aux->constant, r21));
}

void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2) {
  two_body_params_t *aux = malloc(sizeof(two_body_params_t));
  assert(aux != NULL);
  *aux = (two_body_params_t){body1, body2, k};
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, (force_creator_t)elastic_force,
                                 aux, bodies, free);
}

//------------------------------------------------------------------------------

typedef struct drag_params {
  body_t *body;
  double gamma;
} drag_params_t;

void drag(drag_params_t *aux) {
  vector_t vel = body_get_velocity(aux->body);
  body_add_force(aux->body, vec_multiply(-(aux->gamma), vel));
}

void create_drag(scene_t *scene, double gamma, body_t *body) {
  drag_params_t *aux = malloc(sizeof(drag_params_t));
  assert(aux != NULL);
  *aux = (drag_params_t){body, gamma};
  list_t *bodies = list_init(1, NULL);
  list_add(bodies, body);
  scene_add_bodies_force_creator(scene, (force_creator_t)drag, aux, bodies, free);
}

void constant_drag(drag_params_t *aux) {
  vector_t vel = body_get_velocity(aux->body);
  if (vec_magnitude(vel) >= VEL_THRESH) {
    body_add_force(aux->body, vec_multiply(-aux->gamma, vec_unit(vel)));
  }
}

void create_constant_drag_force(scene_t *scene, double force, body_t *body) {
  drag_params_t *aux = malloc(sizeof(drag_params_t));
  assert(aux != NULL);
  *aux = (drag_params_t){body, force};
  // scene_add_force_creator(scene, (force_creator_t)drag, aux, free);
  list_t *bodies = list_init(1, NULL);
  list_add(bodies, body);
  scene_add_bodies_force_creator(scene, (force_creator_t)constant_drag,
                                   aux, bodies, free);
}

//-----------------------------------------------------------------------------

void destroy_upon_collision(two_body_params_t *aux) {
  list_t *shape1 = body_get_shape(aux->body1);
  list_t *shape2 = body_get_shape(aux->body2);
  if (find_collision(shape1, shape2).collided) {
    body_remove(aux->body1);
    body_remove(aux->body2);
    free(shape1);
    free(shape2);
  }
}

void create_destructive_collision(scene_t *scene, body_t *body1,
                                  body_t *body2) {
  two_body_params_t *aux = malloc(sizeof(two_body_params_t));
  assert(aux != NULL);
  *aux = (two_body_params_t){body1, body2, ZERO};
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, (force_creator_t)destroy_upon_collision,
                                 aux, bodies, free);
}

//------------------------------------------------------------------------------

//minimum number of ticks before next collision detection
const size_t COLLISION_COOLDOWN = 6; 

typedef struct collision_params {
  body_t *body1;
  body_t *body2;
  collision_handler_t handler;
  void *aux;
  free_func_t freer;
  size_t previously_collided;
} collision_params_t;

typedef struct chaos_collision_params {
  body_t *body1;
  body_t *body2;
  body_t *body3;
  collision_handler_t handler;
  void *aux;
  free_func_t freer;
  size_t previously_collided;
} chaos_collision_params_t;

void collision_forcer(collision_params_t *params) {
  if (params->previously_collided > 0) {
    params->previously_collided = (params->previously_collided) - 1;
    return;
  }
  list_t *shape1 = body_get_shape(params->body1);
  list_t *shape2 = body_get_shape(params->body2);
  collision_info_t collision_info = find_collision(shape1, shape2);
  if (collision_info.collided) {
    params->previously_collided = COLLISION_COOLDOWN;
    params->handler(params->body1, params->body2, collision_info.axis,
                    params->aux);
  }
}

void create_collision(scene_t *scene, body_t *body1, body_t *body2,
                      collision_handler_t handler, void *aux,
                      free_func_t freer) {
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  collision_params_t *params = malloc(sizeof(collision_params_t));
  assert(params != NULL);
  *params = (collision_params_t){body1, body2, handler, aux, freer, 0};
  scene_add_bodies_force_creator(scene, (force_creator_t)collision_forcer,
                                 params, bodies, free);
}

void elastic_collision(body_t *body1, body_t *body2, vector_t axis,
                       void *elasticity) {
  double ma = body_get_mass(body1);
  double mb = body_get_mass(body2);
  double ua = vec_dot(body_get_velocity(body1), axis);
  double ub = vec_dot(body_get_velocity(body2), axis);
  double c_r = *((double *)elasticity);
  double mu = ma * mb / (ma + mb);
  if (ma == INFINITY) {
    mu = mb;
  } else if(mb == INFINITY) {
    mu = ma;
  }
  vector_t dp1 = vec_multiply((1.0 + c_r) * mu * (ub - ua), axis);
  if (ub - ua > 0) { //prevents "hanging"
    return;
  }
  body_add_impulse(body1, dp1);
  body_add_impulse(body2, vec_negate(dp1));
}

// special ball collision handler. Just makes axis = vec between centroids
// for better precision.
void ball_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                       void *elasticity) {
  vector_t axis_true = vec_unit(vec_subtract(body_get_centroid(body2), 
                                             body_get_centroid(body1)));
  elastic_collision(body1, body2, axis_true, elasticity);
}

// special poolstick collision handler.
void poolstick_collision_handler(body_t *cue, body_t *stick, void *elasticity) {
  double ma = body_get_mass(stick);
  double mb = body_get_mass(cue);
  double c_r = *((double *)elasticity);
  double mu = ma * mb / (ma + mb);
  vector_t v = body_get_velocity(stick);
  vector_t dp = vec_multiply((1.0 + c_r) * mu, v);
  body_add_impulse(cue, dp);
}

void destructive_elastic_collision(body_t *body1, body_t *body2, vector_t axis,
                                   void *elasticity) {
  if (*((size_t*)body_get_info(body2)) == POOLSTICK_ID) {
    poolstick_collision_handler(body1, body2, elasticity);
  }
  else {
    elastic_collision(body1, body2, axis, elasticity);
  }
  body_remove(body2);
}

void create_physics_collision(scene_t *scene, double elasticity, body_t *body1,
                              body_t *body2) {
  double *aux = malloc(sizeof(double));
  *aux = elasticity;
  if (is_ball(body1) && is_ball(body2)) {
    create_collision(scene, body1, body2, ball_collision_handler, aux, free);
  }
  else {
    create_collision(scene, body1, body2, elastic_collision, aux, free);
  }
}

void create_destructive_physics_collision(scene_t *scene, double elasticity,
                                          body_t *body1, body_t *body2) {
  double *aux = malloc(sizeof(double));
  *aux = elasticity;
  create_collision(scene, body1, body2, destructive_elastic_collision, aux,
                   free);
}

void chaos_collision_forcer(chaos_collision_params_t *params) {
  if (params->previously_collided > 0) {
    params->previously_collided = (params->previously_collided) - 1;
    return;
  }
    list_t *shape1 = body_get_shape(params->body1);
    list_t *shape2 = body_get_shape(params->body2);
    collision_info_t collision_info = find_collision(shape1, shape2);
    if (collision_info.collided) {
      params->previously_collided = COLLISION_COOLDOWN;
      params->handler(params->body1, params->body3, collision_info.axis,
                        params->aux);
    }
  }

void create_chaos_collision(scene_t *scene, body_t *body1, body_t *body2, body_t *body3,
                      collision_handler_t handler, void *aux,
                      free_func_t freer) {
  list_t *bodies = list_init(3, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  list_add(bodies, body3);
  chaos_collision_params_t *params = malloc(sizeof(chaos_collision_params_t));
  assert(params != NULL);
  *params = (chaos_collision_params_t){body1, body2, body3, handler, aux, freer, 0};
  scene_add_bodies_force_creator(scene, (force_creator_t)chaos_collision_forcer,
                                 params, bodies, free);
}

void create_chaos_physics_collision(scene_t *scene, double elasticity, body_t *body1,
                              body_t *body2, body_t *body3) {
  double *aux = malloc(sizeof(double));
  *aux = elasticity;
  create_chaos_collision(scene, body1, body2, body3, elastic_collision, aux, free);
}