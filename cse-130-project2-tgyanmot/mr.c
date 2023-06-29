#include "mr.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"
#include "kvlist.h"

typedef struct thread {
  kvlist_t *input;
  kvlist_t *output;
  mapper_t mapper;
} thread;

typedef struct thread2 {
  kvlist_t *input;
  kvlist_t *output;
  reducer_t reducer;
} thread2;

// puesdo idea from TA slides 13

void *thread_mapper(void *arg) {
  thread *mapper_thread = (thread *)arg;
  kvlist_iterator_t *itor4 = kvlist_iterator_new(mapper_thread->input);
  for (;;) {
    kvpair_t *pair = kvlist_iterator_next(itor4);
    if (pair == NULL) {
      break;
    }
    mapper_thread->mapper(pair, mapper_thread->output);
  }
  kvlist_iterator_free(&itor4);
  return NULL;
}

// puesdo idea for reducer helper...
// in the helper
// sort the input list
// first find duplicates
// create new list for it
// call reducer on new list
// reducer ((list1->key),list 1, output)
// clear the list

void *thread_reducer(void *arg) {
  thread2 *reducer_thread = (thread2 *)arg;
  kvlist_sort(reducer_thread->input);
  kvlist_t *dup_list = kvlist_new();
  int counter = 0;
  kvpair_t *dup_pair;
  kvlist_iterator_t *itor5 = kvlist_iterator_new(reducer_thread->input);
  kvpair_t *og_pair = kvlist_iterator_next(itor5);
  if (og_pair != NULL) {
    kvlist_append(dup_list, kvpair_clone(og_pair));
    dup_pair = kvpair_clone(og_pair);
  } else {
    kvlist_iterator_free(&itor5);
    kvlist_free(&dup_list);
    return NULL;
  }

  kvlist_iterator_t *itor6 = kvlist_iterator_new(dup_list);

  for (;;) {
    kvpair_t *og_pair = kvlist_iterator_next(itor5);
    if (og_pair == NULL) {
      for (int i = 0; i < counter; i++) {
        kvlist_append(dup_list, kvpair_clone(dup_pair));
      }
      reducer_thread->reducer(dup_pair->key, dup_list, reducer_thread->output);
      break;
    }
    if (strcmp(dup_pair->key, og_pair->key) == 0) {
      counter++;
    } else {
      for (int i = 0; i < counter; i++) {
        kvlist_append(dup_list, kvpair_clone(dup_pair));
      }
      reducer_thread->reducer(dup_pair->key, dup_list, reducer_thread->output);
      kvlist_free(&dup_list);
      dup_list = kvlist_new();
      kvlist_append(dup_list, kvpair_clone(og_pair));
      kvpair_free(&dup_pair);
      dup_pair = kvpair_clone(og_pair);
      counter = 0;
    }
  }
  kvlist_iterator_free(&itor5);
  kvlist_iterator_free(&itor6);
  kvlist_free(&dup_list);
  kvpair_free(&dup_pair);

  return NULL;
}

void map_reduce(mapper_t mapper, size_t num_mapper, reducer_t reducer,
                size_t num_reducer, kvlist_t *input, kvlist_t *output) {
  // SPLIT --------------------------------------------------------------------

  kvlist_t **smaller_list = malloc(num_mapper * sizeof(kvlist_t *));

  for (unsigned long i = 0; i < num_mapper; i++) {
    smaller_list[i] = kvlist_new();
  }

  // fill the lists
  kvlist_iterator_t *itor1 = kvlist_iterator_new(input);

  unsigned long count = 0;
  kvpair_t *pair = kvlist_iterator_next(itor1);
  while (pair != NULL) {
    kvlist_append(smaller_list[count], kvpair_clone(pair));
    count = (count + 1) % num_mapper;
    pair = kvlist_iterator_next(itor1);
  }

  kvlist_iterator_free(&itor1);

  // MAPPING
  // ---------------------------------------------------------------------

  kvlist_t **mapped_list = (kvlist_t **)malloc(num_mapper * sizeof(kvlist_t *));
  for (size_t i = 0; i < num_mapper; i++) {
    mapped_list[i] = kvlist_new();
  }

  pthread_t threads[num_mapper];

  thread *mapper_struct = malloc(num_mapper * sizeof(thread));

  for (size_t i = 0; i < num_mapper; i++) {
    mapper_struct[i].input = smaller_list[i];
    mapper_struct[i].output = mapped_list[i];
    mapper_struct[i].mapper = mapper;
    pthread_create(&threads[i], NULL, thread_mapper, &mapper_struct[i]);
  }

  for (size_t i = 0; i < num_mapper; i++) {
    pthread_join(threads[i], NULL);
  }

  for (size_t i = 0; i < num_mapper; i++) {
    kvlist_free(&smaller_list[i]);
  }
  free(smaller_list);
  free(mapper_struct);

  // SHUFFLE
  // ---------------------------------------------------------------------

  kvlist_t **shuffled_list =
      (kvlist_t **)malloc(num_reducer * sizeof(kvlist_t *));
  for (size_t i = 0; i < num_reducer; i++) {
    shuffled_list[i] = kvlist_new();
  }
  unsigned long hash_value;
  for (unsigned long i = 0; i < num_mapper; i++) {
    kvlist_iterator_t *itor3 = kvlist_iterator_new(mapped_list[i]);
    for (;;) {
      kvpair_t *pair = kvlist_iterator_next(itor3);
      if (pair == NULL) {
        break;
      }
      // puesdo idea from shun oh
      hash_value = hash(pair->key) % num_reducer;
      kvlist_append(shuffled_list[hash_value], kvpair_clone(pair));
    }
    kvlist_iterator_free(&itor3);
  }

  for (size_t i = 0; i < num_mapper; i++) {
    kvlist_free(&mapped_list[i]);
  }
  free(mapped_list);

  // REDUCER
  // ---------------------------------------------------------------------

  // first new lists

  kvlist_t **reducer_list =
      (kvlist_t **)malloc(num_reducer * sizeof(kvlist_t *));
  for (size_t i = 0; i < num_reducer; i++) {
    reducer_list[i] = kvlist_new();
  }

  // create threads

  pthread_t threads2[num_reducer];

  thread2 *reducer_struct = malloc(num_reducer * sizeof(thread2));

  for (size_t i = 0; i < num_reducer; i++) {
    reducer_struct[i].input = shuffled_list[i];
    reducer_struct[i].output = reducer_list[i];
    reducer_struct[i].reducer = reducer;
    pthread_create(&threads2[i], NULL, thread_reducer, &reducer_struct[i]);
  }

  for (size_t i = 0; i < num_reducer; i++) {
    pthread_join(threads2[i], NULL);
  }

  for (size_t i = 0; i < num_reducer; i++) {
    kvlist_free(&shuffled_list[i]);
  }

  free(shuffled_list);
  free(reducer_struct);

  // EXTEND
  // ---------------------------------------------------------------------

  for (size_t i = 0; i < num_reducer; i++) {
    kvlist_extend(output, reducer_list[i]);
  }

  // FREE
  // ------------------------------------------------------------------------

  for (size_t i = 0; i < num_reducer; i++) {
    kvlist_free(&reducer_list[i]);
  }

  free(reducer_list);
}
