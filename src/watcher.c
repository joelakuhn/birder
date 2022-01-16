#include "birder.h"

#ifdef __APPLE__
#include "watcher-fse.c"
#endif

#ifdef __linux__
#include "watcher-inotify.c"
#endif
