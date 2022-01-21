#include "birder.h"

#ifdef __APPLE__
#include "darwin/watcher-fse.c"
#endif

#ifdef __linux__
#include "linux/watcher-inotify.c"
#endif
