#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"

#include <pthread.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

#include "watcher-fse.h"

void fse_environment_destroy(void *voidenv, void *hint) {
  fse_environment_t *fseenv = voidenv;
  CFRunLoopStop(fseenv->loop);
  pthread_join(fseenv->thread, NULL);
  pthread_mutex_destroy(&fseenv->lock);
  fseenv->thread = NULL;
  fseenv->loop = NULL;
  free(fseenv);
}

void *fse_run_loop(void *voidenv) {
  fse_environment_t *fseenv = voidenv;
  fseenv->loop = CFRunLoopGetCurrent();
  CFRunLoopSourceContext context = {0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
  CFRunLoopSourceRef source = CFRunLoopSourceCreate(NULL, 0, &context);
  CFRunLoopAddSource(fseenv->loop, source, kCFRunLoopDefaultMode);
  pthread_mutex_unlock(&fseenv->lock);
  CFRunLoopRun();
  pthread_mutex_lock(&fseenv->lock);
  fseenv->loop = NULL;
  pthread_mutex_unlock(&fseenv->lock);
  return NULL;
}

fse_environment_t* fse_environment_create() {
  fse_environment_t *fseenv = malloc(sizeof(fse_environment_t));
  fseenv->loop = NULL;

  pthread_mutex_init(&fseenv->lock, NULL);
  pthread_mutex_lock(&fseenv->lock);

  fseenv->thread = NULL;
  pthread_create(&fseenv->thread, NULL, fse_run_loop, (void *)fseenv);

  pthread_mutex_lock(&fseenv->lock);
  pthread_mutex_unlock(&fseenv->lock);

  return fseenv;
}

void fse_instance_destroy(void *voidinst, void *hint) {
  fse_instance_t *instance = voidinst;

  if (instance->stream) {
    FSEventStreamStop(instance->stream);
    FSEventStreamUnscheduleFromRunLoop(instance->stream, instance->fseenv->loop, kCFRunLoopDefaultMode);
    FSEventStreamInvalidate(instance->stream);
    FSEventStreamRelease(instance->stream);
    instance->stream = NULL;
  }

  if (instance != hint) {
    free(instance);
  }
}

void fse_handle_events(
    ConstFSEventStreamRef stream,
    void *data,
    size_t numEvents,
    void *eventPaths,
    const FSEventStreamEventFlags eventFlags[],
    const FSEventStreamEventId eventIds[])
{
  fse_instance_t *instance = data;
  char** paths = eventPaths;
  str_vec_t* changed_paths = str_vec_new();

  for (size_t i = 0; i < numEvents; i++) {
    unsigned long long flags = eventFlags[i];

    bool is_history = flags & FSE_HISTORY_DONE;
    bool is_modified = flags & FSE_ITEM_MODIFIED;
    bool is_created = flags & FSE_ITEM_CREATED;
    bool is_removed = flags & FSE_ITEM_REMOVED;
    bool is_renamed = flags & FSE_ITEM_RENAMED;

    if (is_history) {
      instance->history_done = true;
      continue;
    }
    else if (!instance->history_done) {
      continue;
    }

    if (is_modified || is_created || is_removed || is_renamed) {
      str_vec_push(changed_paths, paths[i]);
    }
  }

  if (changed_paths->len > 0) {
    instance->birder_env->callback(instance->birder_env, changed_paths);
  }

  str_vec_destroy(changed_paths);
}


void fse_start(birder_env_t* birder_env) {
  int64_t since = 0;

  fse_instance_t *instance = malloc(sizeof(fse_instance_t));
  instance->history_done = false;
  instance->fseenv = fse_environment_create();
  instance->stream = NULL;
  instance->birder_env = birder_env;

  FSEventStreamContext streamcontext = {0, instance, NULL, NULL, NULL};

  CFStringRef* paths = malloc(sizeof(CFStringRef) * birder_env->paths->len);
  for (size_t i = 0; i < birder_env->paths->len; i++) {
    paths[i] = CFStringCreateWithCString(NULL, birder_env->paths->strs[i], kCFStringEncodingUTF8);
  }

  CFArrayRef paths_array = CFArrayCreate(NULL, (const void **)paths, birder_env->paths->len, NULL);

  uint64_t flags = kFSEventStreamCreateFlagNone | kFSEventStreamCreateFlagWatchRoot | kFSEventStreamCreateFlagFileEvents;

  instance->stream = FSEventStreamCreate(
    NULL, /* allocator */
    &fse_handle_events, /* callback */
    &streamcontext,
    paths_array,
    since,
    (CFAbsoluteTime)0.1, /* latency */
    flags
  );
  FSEventStreamScheduleWithRunLoop(instance->stream, instance->fseenv->loop, kCFRunLoopDefaultMode);
  FSEventStreamStart(instance->stream);
}

void fse_stop() {
  fse_instance_t *instance = NULL;
  fse_instance_destroy(instance, instance);
}

void watcher_start(birder_env_t* birder_env) {
  fse_start(birder_env);
  for (;;) sleep(1);
}
