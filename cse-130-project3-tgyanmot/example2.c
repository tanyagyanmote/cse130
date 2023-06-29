// // #include <pthread.h>
// // #include <stdio.h>
// // #include <unistd.h>

// // #include "dining.h"
// // #include "utils.h"

// // int main(void) {
// //   dining_t* d = dining_init(3);

// //   student_t student1 = make_student(1, d);
// //   student_t student2 = make_student(2, d);
// //   cleaning_t cleaning = make_cleaning(1, d);

// //   // student 1 comes in, can enter
// //   student_enter(&student1);

// //   // cleaning cannot enter because of student 1; this blocks
// //   pthread_create(&cleaning.thread, NULL, cleaning_enter, &cleaning);
// //   msleep(100);

// //   // student 1 leaves
// //   student_leave(&student1);

// //   // cleaning should begin now
// //   pthread_join(cleaning.thread, NULL);

// //   // student 2 comes in but cannot enter because the cleaning is in
// progress
// //   pthread_create(&student2.thread, NULL, student_enter, &student2);

// //   // 0.1 seconds later
// //   msleep(100);

// //   // cleaning completes
// //   cleaning_leave(&cleaning);

// //   // now, student 2 should be able to enter
// //   pthread_join(student2.thread, NULL);
// //   student_leave(&student2);

// //   dining_destroy(&d);
// // }

// #include <pthread.h>
// #include <stdio.h>
// #include <unistd.h>

// #include "dining.h"
// #include "utils.h"

// // extra credit scenario
// // student-enter should be blocked after cleaner ques up

// int main(void) {
//   dining_t* d = dining_init(2);

//   student_t student1 = make_student(1, d);
//   student_t student2 = make_student(2, d);
//   student_t student3 = make_student(3, d);
//   cleaning_t cleaning = make_cleaning(1, d);
//   // cleaning_t cleaning_2 = make_cleaning(2, d);

//   student_enter(&student1);
//   msleep(100);

//   // student 2 joins queue
//   pthread_create(&student2.thread, NULL, student_enter, &student2);
//   msleep(100);

//   // student 3 joines queue
//   pthread_create(&student3.thread, NULL, student_enter, &student3);
//   msleep(100);

//   // cleaner joins the queue
//   pthread_create(&cleaning.thread, NULL, cleaning_enter, &cleaning);
//   msleep(100);

//   // everyone leaves
//   student_leave(&student1);
//   msleep(100);

//   student_leave(&student2);
//   msleep(100);

//   // cleaner joins
//   pthread_join(cleaning.thread, NULL);
//   msleep(100);

//   // cleaner leaves
//   cleaning_leave(&cleaning);
//   msleep(100);

//   student_leave(&student3);
//   msleep(100);

//   dining_destroy(&d);
// }

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "dining.h"
#include "utils.h"

int main(void) {
  dining_t* d = dining_init(3);

  student_t student1 = make_student(1, d);
  student_t student2 = make_student(2, d);
  student_t student3 = make_student(3, d);
  student_t student4 = make_student(4, d);
  cleaning_t cleaning1 = make_cleaning(1, d);
  cleaning_t cleaning2 = make_cleaning(2, d);
  cleaning_t cleaning3 = make_cleaning(3, d);

  pthread_create(&cleaning1.thread, NULL, cleaning_enter, &cleaning1);

  msleep(100);
  pthread_create(&student1.thread, NULL, student_enter, &student1);

  msleep(100);
  pthread_create(&student2.thread, NULL, student_enter, &student2);

  msleep(100);

  pthread_create(&student3.thread, NULL, student_enter, &student3);

  msleep(100);

  pthread_create(&student4.thread, NULL, student_enter, &student4);

  msleep(100);

  pthread_join(cleaning1.thread, NULL);
  cleaning_leave(&cleaning1);

  msleep(100);

  pthread_join(student2.thread, NULL);
  student_leave(&student2);

  msleep(100);

  pthread_create(&cleaning2.thread, NULL, cleaning_enter, &cleaning2);

  msleep(100);

  pthread_create(&cleaning3.thread, NULL, cleaning_enter, &cleaning3);

  msleep(100);

  pthread_create(&student2.thread, NULL, student_enter, &student2);

  msleep(100);

  pthread_join(student4.thread, NULL);
  student_leave(&student4);

  msleep(100);
  pthread_join(student3.thread, NULL);
  student_leave(&student3);

  msleep(100);
  pthread_join(student1.thread, NULL);
  student_leave(&student1);

  msleep(100);

  pthread_join(cleaning2.thread, NULL);
  cleaning_leave(&cleaning2);

  msleep(100);

  pthread_join(cleaning3.thread, NULL);
  cleaning_leave(&cleaning3);

  msleep(100);

  pthread_join(student2.thread, NULL);
  student_leave(&student2);

  dining_destroy(&d);
}
