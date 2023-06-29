#include "kvs_clock.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// int cache_counter = 0;

typedef struct kvs_clock_pair {
  char* key;
  char* value;
  int ref_bit;
  int persistence;
} kvs_clock_pair;

struct kvs_clock {
  // TODO: add necessary variables
  kvs_base_t* kvs_base;
  int capacity;
  int cursor;
  kvs_clock_pair* cache;
};

kvs_clock_pair* kvs_clock_pair_new(kvs_clock_pair* pair) {
  pair->ref_bit = 1;
  pair->key = malloc(KVS_KEY_MAX + 1);
  pair->value = malloc(KVS_VALUE_MAX + 1);
  memset(pair->key, 0, KVS_KEY_MAX + 1 * sizeof(char));
  memset(pair->value, 0, KVS_VALUE_MAX + 1 * sizeof(char));
  pair->persistence = 0;
  // TODO: initialize other variables
  return pair;
}

kvs_clock_t* kvs_clock_new(kvs_base_t* kvs, int capacity) {
  kvs_clock_t* kvs_clock = malloc(sizeof(kvs_clock_t));
  kvs_clock->kvs_base = kvs;
  kvs_clock->capacity = capacity;
  kvs_clock->cache = malloc(sizeof(kvs_clock_pair) * capacity);
  for (int i = 0; i < capacity; i++) {
    kvs_clock->cache[i] = *kvs_clock_pair_new(&kvs_clock->cache[i]);
  }
  kvs_clock->cursor = 0;
  // TODO: initialize other variables
  return kvs_clock;
}

void kvs_clock_free(kvs_clock_t** ptr) {
  // TODO: free dynamically allocated memory
  for (int i = 0; i < (*ptr)->capacity; i++) {
    free((*ptr)->cache[i].key);
    free((*ptr)->cache[i].value);
  }
  free((*ptr)->cache);
  free(*ptr);
  *ptr = NULL;
}

int kvs_clock_set(kvs_clock_t* kvs_clock, const char* key, const char* value) {
  // TODO: implement this function
  // have to restart to the cursor once it's made a full circle
  if (kvs_clock->capacity == 0) {
    kvs_base_set(kvs_clock->kvs_base, key, value);
    return SUCCESS;
  }
  if (kvs_clock->cursor == kvs_clock->capacity) {
    kvs_clock->cursor = 0;
  }
  // checking if key exists and u just need to replace value
  for (int i = 0; i < kvs_clock->capacity; i++) {
    if (strcmp(kvs_clock->cache[i].key, "\0") == 0) {
      // printf("Case 1");
      strcpy(kvs_clock->cache[i].key, key);
      strcpy(kvs_clock->cache[i].value, value);
      kvs_clock->cache[i].persistence = 1;
      kvs_clock->cursor++;
      return SUCCESS;
    }
    if (strcmp(kvs_clock->cache[i].key, key) == 0) {
      // printf("Case 2");
      strcpy(kvs_clock->cache[i].value, value);
      kvs_clock->cache[i].ref_bit = 1;
      kvs_clock->cache[i].persistence = 1;
      return SUCCESS;
    }
  }
  // if its full --> take the one at index 0, and shift the list down
  //  printf("Case 3");
  for (;;) {
    if (kvs_clock->cache[kvs_clock->cursor].ref_bit == 0) {
      // kvs_base_set(kvs_clock->kvs_base,kvs_clock->cache[kvs_clock->cursor].key,kvs_clock->cache[kvs_clock->cursor].value);
      if (kvs_clock->cache[kvs_clock->cursor].persistence == 1) {
        kvs_base_set(kvs_clock->kvs_base,
                     kvs_clock->cache[kvs_clock->cursor].key,
                     kvs_clock->cache[kvs_clock->cursor].value);
        kvs_clock->cache[kvs_clock->cursor].persistence = 0;
      }
      strcpy(kvs_clock->cache[kvs_clock->cursor].key, key);
      strcpy(kvs_clock->cache[kvs_clock->cursor].value, value);
      kvs_clock->cache[kvs_clock->cursor].persistence = 1;
      // kvs_clock->cursor ++;
      kvs_clock->cache[kvs_clock->cursor].ref_bit = 1;
      return SUCCESS;
    } else {
      kvs_clock->cache[kvs_clock->cursor].ref_bit = 0;
      kvs_clock->cursor++;
      if (kvs_clock->cursor == kvs_clock->capacity) {
        kvs_clock->cursor = 0;
      }
    }
  }
}

int kvs_clock_get(kvs_clock_t* kvs_clock, const char* key, char* value) {
  // TODO: implement this function
  if (kvs_clock->capacity == 0) {
    kvs_base_get(kvs_clock->kvs_base, key, value);
    return SUCCESS;
  }
  if (kvs_clock->cursor == kvs_clock->capacity) {
    kvs_clock->cursor = 0;
  }
  for (int i = 0; i < kvs_clock->capacity; i++) {
    if (strcmp(kvs_clock->cache[i].key, "\0") == 0) {
      // printf("Case 1");
      strcpy(kvs_clock->cache[i].key, key);
      kvs_base_get(kvs_clock->kvs_base, key, value);
      strcpy(kvs_clock->cache[i].value, value);
      kvs_clock->cursor++;
      // printf("cache miss\n");
      return SUCCESS;
    }
    if (strcmp(kvs_clock->cache[i].key, key) == 0) {
      // printf("Case 2");
      strcpy(value, kvs_clock->cache[i].value);
      kvs_clock->cache[i].ref_bit = 1;
      // printf("cache hit\n");
      return SUCCESS;
    }
  }
  kvs_base_get(kvs_clock->kvs_base, key, value);
  // adding to cache
  //  printf("cache miss 2\n");
  //  printf("Case 3");
  for (;;) {
    if (kvs_clock->cache[kvs_clock->cursor].ref_bit == 0) {
      if (kvs_clock->cache[kvs_clock->cursor].persistence == 1) {
        kvs_base_set(kvs_clock->kvs_base,
                     kvs_clock->cache[kvs_clock->cursor].key,
                     kvs_clock->cache[kvs_clock->cursor].value);
        kvs_clock->cache[kvs_clock->cursor].persistence = 0;
      }
      strcpy(kvs_clock->cache[kvs_clock->cursor].key, key);
      strcpy(kvs_clock->cache[kvs_clock->cursor].value, value);
      kvs_clock->cursor++;
      return SUCCESS;
    } else {
      kvs_clock->cache[kvs_clock->cursor].ref_bit = 0;
      kvs_clock->cursor++;
      if (kvs_clock->cursor == kvs_clock->capacity) {
        kvs_clock->cursor = 0;
      }
    }
  }

  return SUCCESS;
}

int kvs_clock_flush(kvs_clock_t* kvs_clock) {
  // TODO: implement this function
  for (int i = 0; i < kvs_clock->capacity; i++) {
    if (kvs_clock->cache[i].persistence == 1) {
      kvs_base_set(kvs_clock->kvs_base, kvs_clock->cache[i].key,
                   kvs_clock->cache[i].value);
    }
  }
  return SUCCESS;
}
