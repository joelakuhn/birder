#ifndef __constants_h
#define __constants_h

#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"

#include <pthread.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

typedef struct
{
  pthread_t thread;
  pthread_mutex_t lock;
  CFRunLoopRef loop;
} fse_environment_t;

typedef struct
{
  fse_environment_t* fseenv;
  FSEventStreamRef stream;
  boolean_t history_done;
  birder_env_t* birder_env;
} fse_instance_t;

enum fs_event_flags {
  FSE_HISTORY_DONE        = 0X00000010,
  FSE_ITEM_MODIFIED       = 0X00001000,
  FSE_ITEM_CREATED        = 0X00000100,
  FSE_ITEM_REMOVED        = 0X00000200,
  FSE_ITEM_RENAMED        = 0X00000800,
};

#endif
