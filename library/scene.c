#include "scene.h"
#include "body.h"
#include "list.h"
#include "polygon.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

const size_t INIT_BODY_COUNT = 10;
const size_t INIT_FORCE_COUNT = 20;

typedef struct scene {
  list_t *bodies;
  list_t *forcer_specs;
} scene_t;

typedef struct forcer_spec { // wrapper for force creator info
  force_creator_t forcer;
  void *aux;
  free_func_t aux_freer;
  list_t *bodies;
} forcer_spec_t;

void forcer_spec_freer(forcer_spec_t *forcer_spec) {
  if (forcer_spec->aux_freer != NULL) {
    forcer_spec->aux_freer(forcer_spec->aux);
  }
  list_free(forcer_spec->bodies);
  free(forcer_spec);
}

scene_t *scene_init(void) {
  scene_t *new_scene = malloc(sizeof(scene_t));
  assert(new_scene != NULL);
  new_scene->bodies = list_init(INIT_BODY_COUNT, (free_func_t)body_free);
  new_scene->forcer_specs =
      list_init(INIT_FORCE_COUNT, (free_func_t)forcer_spec_freer);
  return new_scene;
}

void scene_free(scene_t *scene) {
  list_free(scene->bodies);
  list_free(scene->forcer_specs);
  free(scene);
}

size_t scene_bodies(scene_t *scene) { return list_size(scene->bodies); }

body_t *scene_get_body(scene_t *scene, size_t index) {
  return list_get(scene->bodies, index);
}

void scene_add_body(scene_t *scene, body_t *body) {
  list_add(scene->bodies, body);
}

void scene_remove_body(scene_t *scene, size_t index) {
  body_remove(list_get(scene->bodies, index));
}

void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux,
                             free_func_t freer) {
  forcer_spec_t *new_forcer = malloc(sizeof(forcer_spec_t));
  assert(new_forcer != NULL);
  new_forcer->forcer = forcer;
  new_forcer->aux = aux;
  new_forcer->aux_freer = freer;
  new_forcer->bodies = list_init(0, (free_func_t)body_free);
  list_add(scene->forcer_specs, new_forcer);
}

void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies,
                                    free_func_t freer) {
  forcer_spec_t *new_forcer = malloc(sizeof(forcer_spec_t));
  assert(new_forcer != NULL);
  new_forcer->forcer = forcer;
  new_forcer->aux = aux;
  new_forcer->aux_freer = freer;
  new_forcer->bodies = bodies;
  list_add(scene->forcer_specs, new_forcer);
}

void eliminate_redundant_forcers(scene_t *scene) {
  for (int32_t i = list_size(scene->forcer_specs) - 1; i >= 0; i--) {
    forcer_spec_t *forcer_spec = list_get(scene->forcer_specs, i);
    for (size_t j = 0; j < list_size(forcer_spec->bodies); j++) {
      if (body_is_removed(list_get(forcer_spec->bodies, j))) {
        forcer_spec_freer(list_remove(scene->forcer_specs, i));
        break;
      }
    }
  }
}

void scene_tick(scene_t *scene, double dt) {
  eliminate_redundant_forcers(scene);
  for (size_t i = 0; i < list_size(scene->forcer_specs); i++) {
    forcer_spec_t *forcer_spec = list_get(scene->forcer_specs, i);
    forcer_spec->forcer(forcer_spec->aux);
  }
  list_t *removed_bodies = list_init(0, (free_func_t)body_free);
  for (int32_t i = scene_bodies(scene) - 1; i >= 0; i--) {
    body_t *body = list_get(scene->bodies, i);
    if (body_is_removed(body)) {
      list_add(removed_bodies, list_remove(scene->bodies, i));
    } 
    else {
      body_tick(body, dt);
    }
  }
  eliminate_redundant_forcers(scene);
  list_free(removed_bodies);
}
