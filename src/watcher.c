#include "birder.h"

int WATCHER_IS_RUNNING = 0;

#ifdef __APPLE__
#include "darwin/watcher-fse.c"
#endif

#ifdef __linux__
#include "linux/watcher-inotify.c"
#endif
