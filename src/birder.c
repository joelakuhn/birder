#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <signal.h>
#include <glob.h>
#include <time.h>

#include "birder.h"
#include "watcher.h"
#include "str_vec.h"

void fork_exec(char** command) {
  pid_t proc_id = fork();
  if (proc_id == 0) {
    execvp(command[0], command);

    printf("failed to execute:");
    for (size_t i=0; command[i] != NULL; i++) {
      printf(" %s", command[i]);
    }
    printf("\n");
    exit(1);
  }
}

void execute(birder_env_t* birder_env, str_vec_t* paths) {
  if (birder_env->min_wait > 0) {
    time_t now = time(NULL);
    if (now - birder_env->last_run < birder_env->min_wait) {
      return;
    }
    birder_env->last_run = now;
  }

  printf("executing\n");

  if (birder_env->append) {
    printf("%s\n", paths->strs[0]);
    for (size_t i = 0; i < paths->len; i++) {
      str_vec_t* command = str_vec_dup(birder_env->command);
      str_vec_push(command, paths->strs[i]);
      str_vec_push(command, NULL);
      fork_exec(command->strs);
      str_vec_destroy(command);
    }
  }
  else {
    str_vec_t* command = str_vec_dup(birder_env->command);
    str_vec_push(command, NULL);
    fork_exec(command->strs);
    str_vec_destroy(command);
  }
}

void fork_and_detach()
{
    pid_t pid;

    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    if (setsid() < 0) exit(EXIT_FAILURE);

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    /* Close all open file descriptors */
    for (int x = sysconf(_SC_OPEN_MAX); x>=0; x--) {
      close (x);
    }
}

void usage() {
  puts("Usage: birder [flags] paths... -- command");
  puts("    -a, --append             appends changed file name to command");
  puts("    -g, --glob               treat paths as globs");
  puts("    -d, --daemonize          launch as daemon");
  puts("    -w, --wait N             wait N seconds between executions");
  puts("    -h, --help               print help message");
}

birder_env_t* parse_args(int argc, char** argv) {
  int daemonize = 0;
  int expand_glob = 0;
  int append = 0;
  time_t min_wait = 0;

  int optind = 1;
  while (1) {
    if (optind >= argc) break;

    if (strcmp("-d", argv[optind]) == 0 || strcmp("--daemonize", argv[optind]) == 0) {
      daemonize = 1;
    }
    else if (strcmp("-g", argv[optind]) == 0 || strcmp("--glob", argv[optind]) == 0) {
      expand_glob = 1;
    }
    else if (strcmp("-a", argv[optind]) == 0 || strcmp("--append", argv[optind]) == 0) {
      append = 1;
    }
    else if (strcmp("-w", argv[optind]) == 0 || strcmp("--wait", argv[optind]) == 0) {
      if (optind + 1 < argc) {
        min_wait = atol(argv[optind + 1]);
        optind += 1;
      }
      else {
        usage();
        exit(EXIT_FAILURE);
      }
    }
    else if (strcmp("-h", argv[optind]) == 0 || strcmp("--help", argv[optind]) == 0) {
      usage();
      exit(EXIT_SUCCESS);
    }
    else {
      break;
    }
    optind += 1;
  }

  birder_env_t* birder_env = malloc(sizeof(birder_env_t));
  birder_env->last_run = 0;
  birder_env->min_wait = min_wait;
  birder_env->command = str_vec_new();
  birder_env->paths = str_vec_new();
  birder_env->callback = &execute;
  birder_env->append = append;
  birder_env->daemonize = daemonize;

  size_t i = optind;
  for (; i < argc; i++) {
    if (strcmp(argv[i], "--") == 0) break;
      if (expand_glob) {
      glob_t globbuf; glob(argv[i], GLOB_MARK | GLOB_TILDE, NULL, &globbuf);
      for (size_t glob_i = 0; glob_i < globbuf.gl_pathc; glob_i++) {
        str_vec_push(birder_env->paths, globbuf.gl_pathv[glob_i]);
      }
    }
    else {
      str_vec_push(birder_env->paths, argv[i]);
    }
  }
  i += 1;
  for (; i < argc; i++) {
    str_vec_push(birder_env->command, argv[i]);
  }

  if (birder_env->paths->len == 0 || birder_env->command->len == 0) {
    usage();
    return NULL;
  }

  return birder_env;
}

void birder_env_destroy(birder_env_t* birder_env) {
  str_vec_destroy(birder_env->command);
  str_vec_destroy(birder_env->paths);
  free(birder_env);
}

void handle_term() {
  watcher_stop();
}

void init_traps() {
  signal(SIGTERM, &handle_term);
  signal(SIGINT, &handle_term);
}

int main(int argc, char** argv) {
  birder_env_t* birder_env = parse_args(argc, argv);

  if (birder_env == NULL) {
    return 1;
  }

  if (birder_env->daemonize) {
    fork_and_detach();
  }

  init_traps();
  watcher_start(birder_env);
  birder_env_destroy(birder_env);
  return 0;
}
