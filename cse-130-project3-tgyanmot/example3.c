#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "dining.h"
#include "utils.h"

// extra credit scenario
// student-enter should be blocked after cleaner ques up

int main(void) {
  dining_t* d = dining_init(2);

  student_t student1 = make_student(1, d);
  student_t student2 = make_student(2, d);
  student_t student3 = make_student(3, d);
  cleaning_t cleaning = make_cleaning(1, d);
  // cleaning_t cleaning_2 = make_cleaning(2, d);

  student_enter(&student1);
  msleep(100);

  // student 2 joins queue
  pthread_create(&student2.thread, NULL, student_enter, &student2);
  msleep(100);

  // student 3 joines queue
  pthread_create(&student3.thread, NULL, student_enter, &student3);
  msleep(100);

  // cleaner joins the queue
  pthread_create(&cleaning.thread, NULL, cleaning_enter, &cleaning);
  msleep(100);

  // everyone leaves
  student_leave(&student1);
  msleep(100);

  student_leave(&student2);
  msleep(100);

  // cleaner joins
  pthread_join(cleaning.thread, NULL);
  msleep(100);

  // cleaner leaves
  cleaning_leave(&cleaning);
  msleep(100);

  student_leave(&student3);
  msleep(100);

  dining_destroy(&d);
}
