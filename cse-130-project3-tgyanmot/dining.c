#include "dining.h"

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct dining {
  // TODO: Add your variables here
  int student_count;
  int capacity;
  sem_t semEmpty;
  sem_t semFull;
  pthread_mutex_t mutexBuffer;
  pthread_cond_t c;
  pthread_cond_t c2;
  pthread_cond_t c3;
  bool checker;
  int check_counter;

} dining_t;

dining_t *dining_init(int capacity) {
  // TODO: Initialize necessary variables
  dining_t *dining = malloc(sizeof(dining_t));
  dining->capacity = capacity;
  dining->student_count = 0;
  // from shun
  pthread_mutex_init(&dining->mutexBuffer, NULL);
  sem_init(&dining->semEmpty, 0, capacity);
  sem_init(&dining->semFull, 0, 0);
  pthread_cond_init(&dining->c, NULL);
  pthread_cond_init(&dining->c2, NULL);
  pthread_cond_init(&dining->c3, NULL);
  dining->checker = false;
  dining->check_counter = 0;
  return dining;
}
void dining_destroy(dining_t **dining) {
  // TODO: Free dynamically allocated memory
  sem_destroy(&(*dining)->semEmpty);
  sem_destroy(&(*dining)->semFull);
  pthread_mutex_destroy(&(*dining)->mutexBuffer);
  pthread_cond_destroy(&(*dining)->c);
  pthread_cond_destroy(&(*dining)->c2);
  pthread_cond_destroy(&(*dining)->c3);
  free(*dining);
  *dining = NULL;
}

void dining_student_enter(dining_t *dining) {
  // TODO: Your code goes here
  // student enter 3 edge cases
  sem_wait(&dining->semEmpty);
  if (dining->checker == true) {
    pthread_cond_wait(&dining->c2, &dining->mutexBuffer);
    pthread_mutex_unlock(&dining->mutexBuffer);
  }
  pthread_mutex_lock(&dining->mutexBuffer);
  // counting how many students enter
  dining->student_count++;
  pthread_mutex_unlock(&dining->mutexBuffer);
  sem_post(&dining->semFull);
}

void dining_student_leave(dining_t *dining) {
  // TODO: Your code goes here
  sem_wait(&dining->semFull);
  pthread_mutex_lock(&dining->mutexBuffer);
  // student leaves
  dining->student_count--;
  pthread_mutex_unlock(&dining->mutexBuffer);
  sem_post(&dining->semEmpty);
  if (dining->student_count == 0) {
    pthread_cond_signal(&dining->c);
  }
}

void dining_cleaning_enter(dining_t *dining) {
  // TODO: Your code goes here
  // 2 edge cases
  pthread_mutex_lock(&dining->mutexBuffer);
  dining->checker = true;
  dining->check_counter++;
  if (dining->check_counter > 1) {
    pthread_cond_wait(&dining->c3, &dining->mutexBuffer);
  }
  if (dining->student_count == 0) {
    pthread_mutex_unlock(&dining->mutexBuffer);
    return;
  } else {
    pthread_cond_wait(&dining->c, &dining->mutexBuffer);
  }
  pthread_mutex_unlock(&dining->mutexBuffer);
}

void dining_cleaning_leave(dining_t *dining) {
  // TODO: Your code goes here
  pthread_mutex_lock(&dining->mutexBuffer);
  dining->check_counter--;
  if (dining->check_counter == 0) {
    dining->checker = false;
    pthread_cond_broadcast(&dining->c2);
  } else {
    pthread_cond_signal(&dining->c3);
  }
  pthread_mutex_unlock(&dining->mutexBuffer);
}
