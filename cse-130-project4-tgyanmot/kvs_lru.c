#include "kvs_lru.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct kvs_lru_pair {
  char* key;
  char* value;
  int access;
  int persistence;
} kvs_lru_pair;

struct kvs_lru {
  // TODO: add necessary variables
  kvs_base_t* kvs_base;
  int capacity;
  kvs_lru_pair* cache;
  int time;
};

kvs_lru_pair* kvs_lru_pair_new(kvs_lru_pair* pair) {
  pair->access = 0;
  pair->key = malloc(KVS_KEY_MAX + 1);
  pair->value = malloc(KVS_VALUE_MAX + 1);
  memset(pair->key, 0, KVS_KEY_MAX + 1 * sizeof(char));
  memset(pair->value, 0, KVS_VALUE_MAX + 1 * sizeof(char));
  pair->persistence = 0;
  // TODO: initialize other variables
  return pair;
}

kvs_lru_t* kvs_lru_new(kvs_base_t* kvs, int capacity) {
  kvs_lru_t* kvs_lru = malloc(sizeof(kvs_lru_t));
  kvs_lru->kvs_base = kvs;
  kvs_lru->capacity = capacity;
  // TODO: initialize other variables
  kvs_lru->cache = malloc(sizeof(kvs_lru_pair) * capacity);
  for (int i = 0; i < capacity; i++) {
    kvs_lru->cache[i] = *kvs_lru_pair_new(&kvs_lru->cache[i]);
  }
  kvs_lru->time = 0;
  return kvs_lru;
}

void kvs_lru_free(kvs_lru_t** ptr) {
  // TODO: free dynamically allocated memory
  for (int i = 0; i < (*ptr)->capacity; i++) {
    free((*ptr)->cache[i].key);
    free((*ptr)->cache[i].value);
  }
  free((*ptr)->cache);
  free(*ptr);
  *ptr = NULL;
}

int kvs_lru_set(kvs_lru_t* kvs_lru, const char* key, const char* value) {
  // TODO: implement this function
  int lowest_access = 0;
  int index = 0;
  if (kvs_lru->capacity == 0) {
    kvs_base_set(kvs_lru->kvs_base, key, value);
    return SUCCESS;
  }
  // checking if the key already exists
  for (int i = 0; i < kvs_lru->capacity; i++) {
    // if it's null just add it
    if (strcmp(kvs_lru->cache[i].key, "\0") == 0) {
      strcpy(kvs_lru->cache[i].key, key);
      strcpy(kvs_lru->cache[i].value, value);
      kvs_lru->cache[i].persistence = 1;
      kvs_lru->time++;
      kvs_lru->cache[i].access = kvs_lru->time;
      return SUCCESS;
    }
    // if not null check if key exists
    if (strcmp(kvs_lru->cache[i].key, key) == 0) {
      strcpy(kvs_lru->cache[i].value, value);
      kvs_lru->cache[i].persistence = 1;
      kvs_lru->time++;
      kvs_lru->cache[i].access = kvs_lru->time;
      return SUCCESS;
    }
  }
  // find the kv pair with the least access value
  lowest_access = kvs_lru->cache[0].access;
  for (int i = 1; i < kvs_lru->capacity; i++) {
    if (kvs_lru->cache[i].access < lowest_access) {
      lowest_access = kvs_lru->cache[i].access;
      index = i;
    }
  }
  if (kvs_lru->cache[index].persistence == 1) {
    kvs_base_set(kvs_lru->kvs_base, kvs_lru->cache[index].key,
                 kvs_lru->cache[index].value);
    kvs_lru->cache[index].persistence = 0;
  }  // how to shift the cache?
  strcpy(kvs_lru->cache[index].key, key);
  strcpy(kvs_lru->cache[index].value, value);
  kvs_lru->time++;
  kvs_lru->cache[index].access = kvs_lru->time;
  kvs_lru->cache[index].persistence = 1;
  return SUCCESS;
}

int kvs_lru_get(kvs_lru_t* kvs_lru, const char* key, char* value) {
  // TODO: implement this function
  int lowest_access = 0;
  int index = 0;
  if (kvs_lru->capacity == 0) {
    kvs_base_get(kvs_lru->kvs_base, key, value);
    return SUCCESS;
  }
  for (int i = 0; i < kvs_lru->capacity; i++) {
    // if it's null just add it
    if (strcmp(kvs_lru->cache[i].key, "\0") == 0) {
      strcpy(kvs_lru->cache[i].key, key);
      kvs_base_get(kvs_lru->kvs_base, key, value);
      strcpy(kvs_lru->cache[i].value, value);
      kvs_lru->time++;
      kvs_lru->cache[i].access = kvs_lru->time;
      return SUCCESS;
    }
    // if not null check if key exists
    if (strcmp(kvs_lru->cache[i].key, key) == 0) {
      strcpy(value, kvs_lru->cache[i].value);
      kvs_lru->time++;
      kvs_lru->cache[i].access = kvs_lru->time;
      return SUCCESS;
    }
  }
  lowest_access = kvs_lru->cache[0].access;
  for (int i = 1; i < kvs_lru->capacity; i++) {
    if (kvs_lru->cache[i].access < lowest_access) {
      lowest_access = kvs_lru->cache[i].access;
      index = i;
    }
  }
  kvs_base_get(kvs_lru->kvs_base, key, value);
  // adding to cache
  if (kvs_lru->cache[index].persistence == 1) {
    kvs_base_set(kvs_lru->kvs_base, kvs_lru->cache[index].key,
                 kvs_lru->cache[index].value);
    kvs_lru->cache[index].persistence = 0;
  }
  // how to shift the cache?
  strcpy(kvs_lru->cache[index].key, key);
  strcpy(kvs_lru->cache[index].value, value);
  kvs_lru->time++;
  kvs_lru->cache[index].access = kvs_lru->time;
  kvs_lru->cache[index].persistence = 0;

  return SUCCESS;
}

int kvs_lru_flush(kvs_lru_t* kvs_lru) {
  // TODO: implement this function
  for (int i = 0; i < kvs_lru->capacity; i++) {
    if (kvs_lru->cache[i].persistence == 1) {
      kvs_base_set(kvs_lru->kvs_base, kvs_lru->cache[i].key,
                   kvs_lru->cache[i].value);
    }
  }
  return SUCCESS;
}
