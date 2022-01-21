#include <sys/inotify.h>
#include <sys/select.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>

#include "watcher.h"

#ifndef NAME_MAX
#define NAME_MAX 2048
#endif

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

void watcher_start(birder_env_t* birder_env) {
  WATCHER_IS_RUNNING = 1;
  char buf[BUF_LEN];
  struct inotify_event *event = NULL;
  uint32_t watch_events = IN_MODIFY | IN_MOVE_SELF;
  int inotify_fd = inotify_init();

  struct timeval timeout;

  fd_set set;
  int errno;

  for (size_t i = 0; i < birder_env->paths->len; i++) {
    inotify_add_watch(inotify_fd, birder_env->paths->strs[i], watch_events);
  }

  while (1) {
    timeout.tv_sec = INT_MAX;
    timeout.tv_usec = 0;
    FD_ZERO(&set);
    FD_SET(inotify_fd, &set);

    if ((errno = select(inotify_fd + 1, &set, NULL, NULL, &timeout)) > 0) {
      int n = read(inotify_fd, buf, BUF_LEN);
      char* p = buf;
      str_vec_t* changed_paths = str_vec_new();

      while (p < buf + n) {
        event = (struct inotify_event*)p;
        uint32_t mask = event->mask;
        if (mask & IN_MODIFY || mask & IN_MOVE_SELF) {
          if (event->len) {
            str_vec_push(changed_paths, event->name);
          }
          else {
            str_vec_push(changed_paths, birder_env->paths->strs[event->wd - 1]);
          }
        }
        p += sizeof(struct inotify_event) + event->len;
      }

      if (changed_paths->len > 0) {
        birder_env->callback(birder_env, changed_paths);
      }

      str_vec_destroy(changed_paths);
    }
    else {
      if (!WATCHER_IS_RUNNING) break;
    }
  }

  close(inotify_fd);
}

void watcher_stop() {
  WATCHER_IS_RUNNING = 0;
}
