#define _GNU_SOURCE

#include <err.h>
#include <errno.h>
#include <linux/limits.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <wait.h>

#include "change_root.h"

#define CONTAINER_ID_MAX 16
#define CHILD_STACK_SIZE 4096 * 10

typedef struct container {
  char id[CONTAINER_ID_MAX];
  char image_name[4096];
  char** command;
} container_t;

void container_constuctor(container_t* container, int size) {
  container->command = calloc(size + 1, sizeof(char*));
  for (int i = 0; i < size; i++) {
    container->command[i] = (char*)malloc(4096 * sizeof(char));
  }
}

/**
 * `usage` prints the usage of `client` and exists the program with
 * `EXIT_FAILURE`
 */
void usage(char* cmd) {
  printf("Usage: %s [ID] [IMAGE] [CMD]...\n", cmd);
  exit(EXIT_FAILURE);
}

/**
 * `container_exec` is an entry point of a child process and responsible for
 * creating an overlay filesystem, calling `change_root` and executing the
 * command given as arguments.
 */
int container_exec(void* arg) {
  container_t* container = (container_t*)arg;
  // this line is required on some systems
  if (mount("/", "/", "none", MS_PRIVATE | MS_REC, NULL) < 0) {
    err(1, "mount / private");
  }
  // printf("hi");
  // TODO: Create a overlay filesystem
  // `lowerdir`  should be the image directory: ${cwd}/images/${image}
  // `upperdir`  should be `/tmp/container/{id}/upper`
  // `workdir`   should be `/tmp/container/{id}/work`
  // `merged`    should be `/tmp/container/{id}/merged`
  // ensure all directories exist (create if not exists) and
  // call `mount("overlay", merged, "overlay", MS_RELATIME,
  //    lowerdir={lowerdir},upperdir={upperdir},workdir={workdir})`

  // TODO: Call `change_root` with the `merged` directory
  // change_root(merged)

  // TODO: use `execvp` to run the given command and return its return value
  char testing[PATH_MAX];
  sprintf(testing, "/tmp/container/%s", container->id);
  if (mkdir(testing, 0700) < 0 && errno != EEXIST) {
    err(1, "Failed to create a directory to store container file systems 1");
  }

  char lowerdir[PATH_MAX];
  sprintf(lowerdir, "%s/images/%s", getcwd(NULL, 0), container->image_name);
  char upperdir[PATH_MAX];
  sprintf(upperdir, "/tmp/container/%s/upper", container->id);
  char workdir[PATH_MAX];
  sprintf(workdir, "/tmp/container/%s/work", container->id);
  char merged[PATH_MAX];
  sprintf(merged, "/tmp/container/%s/merged", container->id);

  if (mkdir(lowerdir, 0700) < 0 && errno != EEXIST) {
    err(1, "Failed to create a directory to store container file systems 1");
  }
  if (mkdir(upperdir, 0700) < 0 && errno != EEXIST) {
    err(1, "Failed to create a directory to store container file systems 2");
  }
  if (mkdir(workdir, 0700) < 0 && errno != EEXIST) {
    err(1, "Failed to create a directory to store container file systems 3");
  }
  if (mkdir(merged, 0700) < 0 && errno != EEXIST) {
    err(1, "Failed to create a directory to store container file systems 4");
  }

  char lay[PATH_MAX];
  sprintf(lay, "lowerdir=%s,upperdir=%s,workdir=%s", lowerdir, upperdir,
          workdir);
  if (mount("overlay", merged, "overlay", MS_RELATIME, lay) < 0) {
    err(1, "Failed to mount tmpfs on /tmp/container");
  }
  change_root(merged);
  execvp(container->command[0], container->command);
  return 0;
}

int main(int argc, char** argv) {
  if (argc < 4) {
    usage(argv[0]);
  }

  /* Create tmpfs and mount it to `/tmp/container` so overlayfs can be used
   * inside Docker containers */
  if (mkdir("/tmp/container", 0700) < 0 && errno != EEXIST) {
    err(1, "Failed to create a directory to store container file systems");
  }
  if (errno != EEXIST) {
    if (mount("tmpfs", "/tmp/container", "tmpfs", 0, "") < 0) {
      err(1, "Failed to mount tmpfs on /tmp/container");
    }
  }

  /* cwd contains the absolute path to the current working directory which can
   * be useful constructing path for image */
  char cwd[PATH_MAX];
  getcwd(cwd, PATH_MAX);

  container_t container;
  strncpy(container.id, argv[1], CONTAINER_ID_MAX);

  // TODO: store all necessary information to `container`

  strncpy(container.image_name, argv[2], 4096);

  container_constuctor(&container, argc - 3);
  for (int i = 3; i < argc; i++) {
    strncpy(container.command[i - 3], argv[i], 4096);
    // printf("container.command %s\n",container.command[i-3]);
  }

  /* Use `clone` to create a child process */
  char child_stack[CHILD_STACK_SIZE];  // statically allocate stack for child
  int clone_flags = SIGCHLD | CLONE_NEWNS | CLONE_NEWPID;
  int pid = clone(container_exec, &child_stack, clone_flags, &container);
  if (pid < 0) {
    err(1, "Failed to clone");
  }
  waitpid(pid, NULL, 0);
  for (int i = 3; i < argc; i++) {
    free(container.command[i - 3]);
  }
  free(container.command);
  return EXIT_SUCCESS;
}
