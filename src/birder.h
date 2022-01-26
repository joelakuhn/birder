#ifndef birder_h
#define birder_h

#include <time.h>
#include "str_vec.h"

struct _birder_env;

typedef void (*watcher_callback_t)(struct _birder_env *, str_vec_t* paths);

struct _birder_env {
  watcher_callback_t callback;
  str_vec_t* command;
  str_vec_t* paths;
  int append;
  int daemonize;
  time_t last_run;
  time_t min_wait;
};

typedef struct _birder_env birder_env_t;

#endif
