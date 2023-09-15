#include "vec_list.h"
#include <assert.h>
#include <stdlib.h>

typedef struct vec_list {
  list_t *my_vec_list;
} vec_list_t;

vec_list_t *vec_list_init(size_t initial_size) {
  vec_list_t *veclist = malloc(sizeof(vec_list_t));
  assert(veclist != NULL);
  veclist->my_vec_list = list_init(initial_size, vec_list_free);

  return veclist;
}

void vec_list_free(vec_list_t *list) {
  list_free(list->my_vec_list);
  free(list);
}

size_t vec_list_size(vec_list_t *list) { return list_size(list->my_vec_list); }

vector_t *vec_list_get(vec_list_t *list, size_t index) {
  return list_get(list->my_vec_list, index);
}

void vec_list_add(vec_list_t *list, vector_t *value) {
  list_add(list->my_vec_list, value);
}

vector_t *vec_list_remove(vec_list_t *list, size_t index) {
  return list_remove(list->my_vec_list, index);
}

size_t vec_list_capacity(vec_list_t *list) {
  return list_capacity(list->my_vec_list);
}

void vec_list_resize(vec_list_t *list, size_t new_capacity) {
  list_resize(list->my_vec_list, new_capacity);
}

vec_list_t *vec_list_copy(vec_list_t *list) {
  vec_list_t *copy_vec_list = vec_list_init(vec_list_capacity(list));

  for (size_t i = 0; i < vec_list_capacity(list); i++) {
    vector_t *new = malloc(sizeof(vector_t));
    assert(new != NULL);
    *new = *vec_list_get(list, i);
    vec_list_add(copy_vec_list, new);
  }

  return copy_vec_list;
}
