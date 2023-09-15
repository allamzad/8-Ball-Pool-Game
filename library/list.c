#include "list.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const size_t RESIZE_FACTOR = 2;

typedef struct list {
  size_t capacity; // how many elements can we store
  size_t size;     // how filled is the list?
  free_func_t freer;
  void **my_list;
} list_t;

list_t *list_init(size_t initial_size, free_func_t freer) {
  list_t *list = malloc(sizeof(list_t));
  assert(list != NULL);
  list->capacity = initial_size;
  list->size = 0;
  list->freer = freer;
  list->my_list = malloc(sizeof(void *) * initial_size);
  assert(list->my_list != NULL);

  return list;
}

void list_free(list_t *list) {
  // free all objects in the list first
  if (list->freer != NULL) {
    for (size_t i = 0; i < list_size(list); i++) {
      list->freer(list_get(list, i));
    }
  }
  // free the memory allocated for a list.
  free(list->my_list);
  free(list);
}

size_t list_size(list_t *list) { return list->size; }

void *list_get(list_t *list, size_t index) {
  assert(index >= 0 && index < list_size(list));
  return list->my_list[index];
}

void *list_remove(list_t *list, size_t index) {
  assert(list->size > 0);
  void *to_return = list_get(list, index);
  for (size_t i = index; i < list_size(list) - 1; i++) {
    list->my_list[i] = list_get(list, i + 1);
  }
  list->size--;
  return to_return;
}

void list_add(list_t *list, void *value) {
  assert(value != NULL);
  size_t my_size = list_size(list);
  size_t my_capacity = list_capacity(list);
  if (my_size >= my_capacity) {
    list_resize(list, (my_capacity + 1) * RESIZE_FACTOR);
  }

  list->my_list[my_size] = value;
  list->size++;
}

size_t list_capacity(list_t *list) { return list->capacity; }

void list_resize(list_t *list, size_t new_capacity) {
  assert(list->size <= new_capacity);
  void **new_list = malloc(sizeof(void *) * new_capacity);
  assert(new_list != NULL);

  for (size_t i = 0; i < list->size; i++) {
    new_list[i] = list_get(list, i);
  }

  free(list->my_list);
  list->my_list = new_list;
  list->capacity = new_capacity;
}

void *list_dequeue(list_t *list) { return list_remove(list, 0); }

void *list_pop(list_t *list) { return list_remove(list, list_size(list) - 1); }

void list_shuffle(list_t *list) {
  srand(time(NULL));
  for (size_t i = list_size(list)-1; i > 0; i--) {
    size_t j = rand() % (i+1);
    void *aux = list->my_list[i];
    list->my_list[i] = list->my_list[j];
    list->my_list[j] = aux;
  }
}