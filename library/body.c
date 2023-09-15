#include "body.h"
#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "vector.h"
#include "graphics.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

typedef struct body
{
  list_t *shape;
  sprite_info_t sprite;
  vector_t velocity;
  double angle;
  double ang_vel;
  double mass;
  vector_t centroid;
  vector_t force;
  vector_t impulse;
  void *info;
  free_func_t info_freer;
  bool is_removed;
} body_t;

body_t *body_init(list_t *shape, sprite_info_t sprite, double mass)
{
  return body_init_with_info(shape, sprite, mass, NULL, NULL);
}

body_t *body_init_with_info(list_t *shape, sprite_info_t sprite, double mass, void *info,
                            free_func_t info_freer)
{
  body_t *new_body = malloc(sizeof(body_t));
  assert(new_body != NULL);
  assert(mass > 0);

  new_body->shape = shape;
  new_body->sprite = sprite;
  new_body->mass = mass;
  new_body->centroid = polygon_centroid(shape);
  new_body->velocity = VEC_ZERO; // initially at rest
  new_body->angle = new_body->ang_vel = 0;
  new_body->force = VEC_ZERO;
  new_body->impulse = VEC_ZERO;
  new_body->info = info;
  new_body->info_freer = info_freer;
  new_body->is_removed = false;
  return new_body;
}

void body_free(body_t *body)
{
  list_free(body->shape);
  if (body->info_freer != NULL)
  {
    body->info_freer(body->info);
  }
  free(body);
}

list_t *body_get_deepcopied_shape(body_t *body)
{
  size_t num_vertices = list_size(body->shape);
  list_t *ans = list_init(num_vertices, free);
  for (size_t i = 0; i < num_vertices; i++)
  {
    vector_t *vertex = malloc(sizeof(vector_t));
    assert(vertex != NULL);
    *vertex = *(vector_t *)list_get(body->shape, i);
    list_add(ans, vertex);
  }
  return ans;
}

list_t *body_get_shape(body_t *body)
{
  return body->shape;
}

vector_t body_get_centroid(body_t *body) { return body->centroid; }

vector_t body_get_velocity(body_t *body) { return body->velocity; }

double body_get_mass(body_t *body) { return body->mass; }

void *body_get_info(body_t *body) { return body->info; }

sprite_info_t body_get_sprite(body_t *body) { return body->sprite; }

void body_set_centroid(body_t *body, vector_t x)
{
  polygon_translate(body->shape, vec_subtract(x, body_get_centroid(body)));
  body->centroid = x;
}

void body_set_velocity(body_t *body, vector_t v) { body->velocity = v; }

void body_set_ang_velocity(body_t *body, double omega)
{
  body->ang_vel = omega;
}

void body_set_rotation(body_t *body, double angle)
{
  polygon_rotate(body->shape, angle - body->angle, body_get_centroid(body));
  body->angle = angle;
}

void body_set_img_pos(body_t *body, vector_t x)
{
  body->sprite.img_pos = x;
}

void body_set_shape(body_t *body, list_t *shape){
  list_free(body->shape);
  body->shape = shape;
}

void body_set_color(body_t *body, rgb_color_t color){
  body->sprite.color = color;
}

void body_add_force(body_t *body, vector_t force)
{
  body->force = vec_add(body->force, force);
}

void body_add_impulse(body_t *body, vector_t impulse)
{
  body->impulse = vec_add(body->impulse, impulse);
}

void body_tick(body_t *body, double dt)
{
  vector_t avg_velocity = body->velocity;
  vector_t new_velocity = vec_add(body_get_velocity(body),
                                  vec_multiply(1 / body->mass, body->impulse));
  new_velocity =
      vec_add(new_velocity, vec_multiply(dt / body->mass, body->force));
  body_set_velocity(body, new_velocity);
  avg_velocity = vec_multiply(0.5, vec_add(avg_velocity, new_velocity));
  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
  body_set_centroid(
      body, vec_add(body_get_centroid(body), vec_multiply(dt, avg_velocity)));
  sprite_info_t sprite = body_get_sprite(body);
  body_set_img_pos(body, vec_add(sprite.img_pos, vec_multiply(dt, avg_velocity)));
}

void body_update(body_t *body, double dt)
{
  vector_t translation = vec_multiply(dt, body->velocity);
  body_set_centroid(body, vec_add(body_get_centroid(body), translation));
  body_set_rotation(body, body->ang_vel * dt + body->angle);
}

void body_remove(body_t *body) { body->is_removed = true; }

bool body_is_removed(body_t *body) { return body->is_removed; }
