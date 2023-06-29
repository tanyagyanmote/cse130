#include <err.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SIZE 4096

int main(int argc, char *argv[]) {
  int in;
  int nread;
  char buff[4096];
  int error_num = 0;
  // no arguments
  if (argc == 1) {
    nread = read(0, buff, SIZE);
    while (nread > 0) {
      write(1, buff, nread);
      nread = read(0, buff, SIZE);
    }
  }
  for (int i = 1; i < argc; i++) {
    in = open(argv[i], O_RDONLY);
    // if user inputs "-"
    if (strcmp(argv[i], "-") == 0) {
      nread = read(0, buff, SIZE);
      while (nread > 0) {
        write(1, buff, nread);
        nread = read(0, buff, SIZE);
      }
    }
    // if its a invalid file
    else if (in == -1) {
      warn("%s", argv[i]);
      error_num = 1;
    }
    // keep reading the file
    else {
      nread = read(in, buff, SIZE);
      while (nread > 0) {
        write(1, buff, nread);
        nread = read(in, buff, SIZE);
      }
      // directories
      if (nread == -1) {
        warn("%s", argv[i]);
        error_num = 1;
      }
    }
    close(in);
  }
  return error_num;
}
