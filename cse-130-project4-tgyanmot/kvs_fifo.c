#include "kvs_fifo.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct kvs_fifo_pair {
  char* key;
  char* value;
  int persistence;

} kvs_fifo_pair;

struct kvs_fifo {
  // TODO: add necessary variables
  kvs_base_t* kvs_base;
  int capacity;
  kvs_fifo_pair* cache;
};

kvs_fifo_pair* kvs_fifo_pair_new(kvs_fifo_pair* pair) {
  pair->key = malloc(KVS_KEY_MAX + 1);
  pair->value = malloc(KVS_VALUE_MAX + 1);
  memset(pair->key, 0, KVS_KEY_MAX + 1 * sizeof(char));
  memset(pair->value, 0, KVS_VALUE_MAX + 1 * sizeof(char));
  pair->persistence = 0;
  // TODO: initialize other variables
  return pair;
}

kvs_fifo_t* kvs_fifo_new(kvs_base_t* kvs, int capacity) {
  kvs_fifo_t* kvs_fifo = malloc(sizeof(kvs_fifo_t));
  kvs_fifo->kvs_base = kvs;
  kvs_fifo->capacity = capacity;
  kvs_fifo->cache = malloc(sizeof(kvs_fifo_pair) * capacity);
  for (int i = 0; i < capacity; i++) {
    kvs_fifo->cache[i] = *kvs_fifo_pair_new(&kvs_fifo->cache[i]);
  }
  return kvs_fifo;
}

void kvs_fifo_free(kvs_fifo_t** ptr) {
  // TODO: free dynamically allocated memory
  for (int i = 0; i < (*ptr)->capacity; i++) {
    free((*ptr)->cache[i].key);
    free((*ptr)->cache[i].value);
  }
  free((*ptr)->cache);
  free(*ptr);
  *ptr = NULL;
}

int kvs_fifo_set(kvs_fifo_t* kvs_fifo, const char* key, const char* value) {
  // TODO: implement this function
  char temp_key[KVS_KEY_MAX + 1];
  char temp_value[KVS_VALUE_MAX + 1];
  int temp_persistant;
  if (kvs_fifo->capacity == 0) {
    kvs_base_set(kvs_fifo->kvs_base, key, value);
    return SUCCESS;
  }
  // checking if the key already exists
  for (int i = 0; i < kvs_fifo->capacity; i++) {
    if (strcmp(kvs_fifo->cache[i].key, "\0") == 0) {
      strcpy(kvs_fifo->cache[i].key, key);
      strcpy(kvs_fifo->cache[i].value, value);
      kvs_fifo->cache[i].persistence = 1;
      return SUCCESS;
    }
    if (strcmp(kvs_fifo->cache[i].key, key) == 0) {
      // printf("Case 2\n");
      strcpy(kvs_fifo->cache[i].value, value);
      kvs_fifo->cache[i].persistence = 1;
      return SUCCESS;
    }
  }
  // printf("Case 3\n");
  // if its full --> take the one at index 0, and shift the list down
  if (kvs_fifo->cache[0].persistence == 1) {
    kvs_base_set(kvs_fifo->kvs_base, kvs_fifo->cache[0].key,
                 kvs_fifo->cache[0].value);
    kvs_fifo->cache[0].persistence = 0;
  }
  // kvs_base_set(kvs_fifo->kvs_base,kvs_fifo->cache[0].key,kvs_fifo->cache[0].value);
  for (int i = 0; i < kvs_fifo->capacity - 1; i++) {
    strcpy(temp_key, kvs_fifo->cache[i + 1].key);
    strcpy(temp_value, kvs_fifo->cache[i + 1].value);
    temp_persistant = kvs_fifo->cache[i + 1].persistence;
    strcpy(kvs_fifo->cache[i].key, temp_key);
    strcpy(kvs_fifo->cache[i].value, temp_value);
    kvs_fifo->cache[i].persistence = temp_persistant;
  }
  strcpy(kvs_fifo->cache[kvs_fifo->capacity - 1].key, key);
  strcpy(kvs_fifo->cache[kvs_fifo->capacity - 1].value, value);
  kvs_fifo->cache[kvs_fifo->capacity - 1].persistence = 1;
  return SUCCESS;
}

int kvs_fifo_get(kvs_fifo_t* kvs_fifo, const char* key, char* value) {
  char temp_key[KVS_KEY_MAX + 1];
  char temp_value[KVS_VALUE_MAX + 1];
  int temp_persistant;
  if (kvs_fifo->capacity == 0) {
    kvs_base_get(kvs_fifo->kvs_base, key, value);
    return SUCCESS;
  }
  // The GET operation reads the value associated with the key. If the entry is
  // cached, return the value. checking if the key already exists
  for (int i = 0; i < kvs_fifo->capacity; i++) {
    if (strcmp(kvs_fifo->cache[i].key, "\0") == 0) {
      strcpy(kvs_fifo->cache[i].key, key);
      kvs_base_get(kvs_fifo->kvs_base, key, value);
      strcpy(kvs_fifo->cache[i].value, value);
      return SUCCESS;
    }
    if (strcmp(kvs_fifo->cache[i].key, key) == 0) {
      strcpy(value, kvs_fifo->cache[i].value);
      return SUCCESS;
    }
  }
  // If the entry is not in the cache, use the base KVS to read the value from
  // the disk.
  kvs_base_get(kvs_fifo->kvs_base, key, value);
  // adding to cache
  if (kvs_fifo->cache[0].persistence == 1) {
    // printf("cache key: %s\n", kvs_fifo->cache[0].key);
    kvs_base_set(kvs_fifo->kvs_base, kvs_fifo->cache[0].key,
                 kvs_fifo->cache[0].value);
    kvs_fifo->cache[0].persistence = 0;
  }
  for (int i = 0; i < kvs_fifo->capacity - 1; i++) {
    strcpy(temp_key, kvs_fifo->cache[i + 1].key);
    strcpy(temp_value, kvs_fifo->cache[i + 1].value);
    temp_persistant = kvs_fifo->cache[i + 1].persistence;
    strcpy(kvs_fifo->cache[i].key, temp_key);
    strcpy(kvs_fifo->cache[i].value, temp_value);
    kvs_fifo->cache[i].persistence = temp_persistant;
  }
  strcpy(kvs_fifo->cache[kvs_fifo->capacity - 1].key, key);
  strcpy(kvs_fifo->cache[kvs_fifo->capacity - 1].value, value);
  kvs_fifo->cache[kvs_fifo->capacity - 1].persistence = 0;

  return SUCCESS;
}

int kvs_fifo_flush(kvs_fifo_t* kvs_fifo) {
  // TODO: implement this function
  for (int i = 0; i < kvs_fifo->capacity; i++) {
    if (kvs_fifo->cache[i].persistence == 1) {
      kvs_base_set(kvs_fifo->kvs_base, kvs_fifo->cache[i].key,
                   kvs_fifo->cache[i].value);
      kvs_fifo->cache[i].persistence = 0;
    }
  }
  return SUCCESS;
}
